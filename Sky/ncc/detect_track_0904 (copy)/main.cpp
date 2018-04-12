#include <iostream>
#include <unistd.h>
#include <stdio.h>

#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"

#include "track.hpp"

#define LINT_ARGS 1
extern "C" {
#include "libyolo.h"
#include "image.h"
}

using namespace std;
using namespace cv;

void ipl_into_image(IplImage *src, image im)
{
    unsigned char *data = (unsigned char *)src->imageData;
    int h = src->height;
    int w = src->width;
    int c = src->nChannels;
    int step = src->widthStep;
    int i, j, k;

    for (i = 0; i < h; ++i)
    {
        for (k = 0; k < c; ++k)
        {
            for (j = 0; j < w; ++j)
            {
                im.data[k * w * h + i * w + j] = data[i * step + j * c + k] / 255.;
            }
        }
    }
}

image ipl_to_image(IplImage *src)
{
    int h = src->height;
    int w = src->width;
    int c = src->nChannels;
    image out = make_image(w, h, c);
    ipl_into_image(src, out);
    return out;
}

box detect_image(yolo_handle yolo, Mat frame, double thresh, double hier_thresh)
{
    // convert mat to image
    IplImage img_ipl = IplImage(frame);
    image img = ipl_to_image(&img_ipl);
    box detect_box = yolo_detect(yolo, img, thresh, hier_thresh);
    return detect_box;
}

box detect_frame_file(yolo_handle yolo, Mat frame, char *frame_file, double thresh, double hier_thresh)
{
    imwrite(frame_file, frame);
    box detect_box = yolo_test(yolo, frame_file, thresh, hier_thresh);
    return detect_box;
}

int ncc_track(yolo_handle yolo, char *videofile,
              const double &thresh, const double &hier_thresh)
{
    VideoCapture cap;
    Mat Camera_CImg, Camera_GImg;
    int framenum = 0;
    int track_flag = 0;
    char str[25];
    char *frame_file = "./frame.jpg";
    box detect_box;

    cap.open(videofile);
    if (!cap.isOpened())
    {
        cout << "Can't open video!" << endl;
        yolo_cleanup(yolo);
        return -1;
    }

    while (true)
    {
        cap >> Camera_CImg;

        if (Camera_CImg.empty())
        {
            cout << "Video Ends: 4 0 0" << endl;
            break;
        }

        framenum++;
        cout << framenum << endl;
        cvtColor(Camera_CImg, Camera_GImg, CV_RGB2GRAY);

        if (track_flag == 0)
        {
            cout << "Enter detection: 3 0 0" << endl;
            detect_box = detect_frame_file(yolo, Camera_CImg, frame_file, thresh, hier_thresh);
            cout << "x:" << detect_box.x << " y:" << detect_box.y << " w:" << detect_box.w << " h:" << detect_box.h << endl;

            if (detect_box.h == 0 || detect_box.w == 0)
            {
                cout << "Detection failed: 2 0 0" << endl;
            }
            else
            {
                track_flag = track(Camera_GImg.data, track_flag, detect_box.x, detect_box.y,
                                   detect_box.w, detect_box.h);
            }
        }
        else
        {
            track_flag = track(Camera_GImg.data, track_flag, detect_box.x, detect_box.y,
                               detect_box.w, detect_box.h);
        }

        sprintf(str, "%d", framenum);
        putText(Camera_GImg, str, Point(100, 100), FONT_HERSHEY_PLAIN,
                2.0, Scalar(255, 255, 255), 1, 8, 0);
        imshow("input", Camera_GImg);

        printf("framenum = %d, i = %d\n", framenum, track_flag);
        if (cvWaitKey(60) == 'q')
            break;
    }

    return 0;
}


int main(int argc, char *argv[])
{
    char *darknet_path = "./darknet";
    char *datacfg = "tiny-yolo_fineanchors/voc.data";
    char *cfgfile = "tiny-yolo_fineanchors/tiny-yolo.cfg";
    char *weightfile = "tiny-yolo_fineanchors/tiny-yolo.backup";
    double hier_thresh = 0.8;

    double thresh = 0.25;
    char *videofile = "./video/result08240801.avi";

    for (int i = 1; i < argc; i++)
    {
        string arg_value = string(argv[i]);

        if (arg_value == "-t" || arg_value == "--thresh")
        {
            thresh = atof(argv[i + 1]);
            i += 1;
        }
        else if (strcmp(argv[i], "-v") == 0 || arg_value == "--video")
        {
            videofile = argv[i + 1];
            i += 1;
        }
        else
        {
            cout << "Invalid option " << argv[i] << endl;
            return -1;
        }
    }

    yolo_handle yolo = yolo_init(darknet_path, datacfg, cfgfile, weightfile);
    int ret = ncc_track(yolo, videofile, thresh, hier_thresh);

    yolo_cleanup(yolo);
    return ret;
}
