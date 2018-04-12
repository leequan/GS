#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "detection.h"
#include "./kcf/tracker.h"
//#include "tagEdgeExtraction.cpp"
#include <dirent.h>

using namespace std;
using namespace cv;

int kcf_track(Detection &detection, char *videofile,
              const double &thresh, const double &hier_thresh)
{
    // Tracker results
    Rect result;
    int update_num = 0;

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

        if (framenum == 250)
        {
            int t = 1;
        }
        // new detection
        if (track_flag == 0)
        {
            cout << "Enter detection: 3 0 0" << endl;
            detect_box = detection.detect_image(Camera_CImg, frame_file, thresh, hier_thresh);
            cout << "x:" << detect_box.x << " y:" << detect_box.y << " w:" << detect_box.w << " h:" << detect_box.h << endl;

            if (detect_box.h == 0 || detect_box.w == 0)
            {
                cout << "Detection failed: 2 0 0" << endl;
            }
            else
            {
                result.x = (int)detect_box.x - (int)detect_box.w / 2 + 1;
                result.y = (int)detect_box.y - (int)detect_box.h / 2 + 1;
                result.width = (int)detect_box.w;
                result.height = (int)detect_box.h;

                result = tagEdgeExtraction(Camera_CImg, result);

                track_flag = kcfTrack(Camera_CImg, result, track_flag);
            }
        }

        // just track
        else
        {
            track_flag = kcfTrack(Camera_CImg, result, track_flag);
        }

        sprintf(str, "%d", framenum);
        putText(Camera_CImg, str, Point(100, 100), FONT_HERSHEY_PLAIN,
                2.0, Scalar(255, 255, 255), 1, 8, 0);
        imshow("input", Camera_CImg);
        printf("framenum = %d, i = %d\n", framenum, track_flag);
        if (cvWaitKey(20) == 'q')
            break;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    char *darknet_path = "SkyDetection1/darknet";
    char *datacfg = "model/tiny-yolo_fineanchors/voc.data";
    char *cfgfile = "model/tiny-yolo_fineanchors/tiny-yolo.cfg";
    char *weightfile = "model/tiny-yolo_fineanchors/tiny-yolo_final.weights";
    double hier_thresh = 0.5;

    double thresh = 0.25;
    char *videofile = "/home/userver/Qt5.5.1/detect_track_kcf_1023/video/02.avi";

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

    Detection detection(darknet_path, datacfg, cfgfile, weightfile);
    int ret = kcf_track(detection, videofile, thresh, hier_thresh);

    return ret;
}
