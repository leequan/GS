#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "detection.h"
#include "./kcf/tracker.h"
// #include "tagEdgeExtraction.cpp"

#include <dirent.h>

using namespace std;
using namespace cv;

int kcf_track(Detection &detection, char *videofile,
              const double &thresh, const double &hier_thresh)
{
    // Tracker results
    Rect result;

    VideoCapture cap;
    Mat Camera_CImg, Process_Img;
    int framenum = 0;
    int track_flag = 0;
    char str[25];
    char *frame_file = "./frame.jpg";
    box detect_box;
    // the rect at center with size of 720x576
    const Rect detect_rect(1280 / 2 - 720 / 2, 720 / 2 - 576 / 2, 720, 576);

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

        if (Camera_CImg.cols > 800)
        { //image size:1280*720
            Process_Img = Camera_CImg(detect_rect);
        }
        else
        { //image size:720*576
            Process_Img = Camera_CImg;
        }

        // new detection
        if (track_flag == 0)
        {
            cout << "Enter detection: 3 0 0" << endl;

            clock_t t;
            t = clock();
            detect_box = detection.detect_image(Process_Img, frame_file, thresh, hier_thresh);
            printf("All elapsed time for detecting: %f seconds.\n", sec(clock() - t));

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

                result = tagEdgeExtraction(Process_Img, result);
                track_flag = kcfTrack(Process_Img, result, track_flag);
            }
        }

        // just track
        else
        {
            clock_t t1;
            t1 = clock();
            track_flag = kcfTrack(Process_Img, result, track_flag);
            printf("All elapsed time for tracking: %f seconds.\n", sec(clock() - t1));
        }

        sprintf(str, "%d", framenum);
        putText(Process_Img, str, Point(100, 100), FONT_HERSHEY_PLAIN,
                2.0, Scalar(255, 255, 255), 1, 8, 0);
        imshow("input", Process_Img);
        printf("framenum = %d, i = %d\n", framenum, track_flag);
        if (cvWaitKey(20) == 'q')
            break;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    char *darknet_path = "./darknet";
    char *datacfg = "model/tiny-yolo_oneClass/voc.data";
    char *cfgfile = "model/tiny-yolo_oneClass/tiny-yolo.cfg";
    char *weightfile = "model/tiny-yolo_oneClass/tiny-yolo.backup";
    double hier_thresh = 0.8;

    // default value
    double thresh = 0.05; //UAV 0.25
    char *video_file = "./video/result08240802.avi";

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
            video_file = argv[i + 1];
            i += 1;
        }
        else
        {
            cout << "Invalid option " << argv[i] << endl;
            return -1;
        }
    }

    Detection detection(darknet_path, datacfg, cfgfile, weightfile);
    int ret = kcf_track(detection, video_file, thresh, hier_thresh);

    return ret;
}
