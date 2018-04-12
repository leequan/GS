#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"

#include "detection.h"
#include "track.hpp"

using namespace std;
using namespace cv;

TargetData_TYPE TargetData_Parameter2;
int obj_x, obj_y;

void similar(int cx, int cy, int w, int h)
{
    if (cx < 0)
        cx = 0;
    if (cy < 0)
        cy = 0;
    if (w + cx > 720)
        w = w + cx - 720;
    if (h + cy > 576)
        h = h + cy - 576;
}

int ncc_track(Detection &detection, char *videofile,
              const double &thresh, const double &hier_thresh)
{
    VideoCapture cap;
    Mat Camera_CImg, Camera_GImg, sub_img, obj_img;
    int framenum = 0;
    int track_flag = 0;
    char str[25];
    char *frame_file = "./frame.jpg";
    int detect_flag = 0;
    box detect_box;
    detect_box.x = -1;
    detect_box.y = -1;
    detect_box.h = 0;
    detect_box.w = 0;

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
        sub_img = Camera_CImg;
        //sub_img = Camera_CImg(center_rect);
        cvtColor(sub_img, Camera_GImg, CV_RGB2GRAY);

        // new detection
        if (track_flag == 0)
        {
            cout << "Enter detection: 3 0 0" << endl;

            if (detect_box.x == -1)
            {
                detect_box = detection.detect_image(sub_img, frame_file, thresh, hier_thresh);
            }
            else if ((detect_box.x == -2) && (detect_flag == 5))
            {
                detect_flag = 0;
                detect_box = detection.detect_image(sub_img, frame_file, thresh, hier_thresh);
            }
            else
            { // 检测丢失前一帧的图像
                detect_box = detection.detect_image(sub_img, obj_img, frame_file, thresh, hier_thresh, obj_x, obj_y, detect_flag);
            }

            // TODO create templete
            cout << "x:" << detect_box.x << " y:" << detect_box.y << " w:" << detect_box.w << " h:" << detect_box.h << endl;

            if (detect_box.h == 0 || detect_box.w == 0)
                cout << "Detection failed: 2 0 0" << endl;
            else
                track_flag = track(Camera_GImg.data, track_flag, detect_box.x, detect_box.y,
                                   detect_box.w, detect_box.h, TargetData_Parameter2);
        }

        // just track
        else
        {
            track_flag = track(Camera_GImg.data, track_flag, detect_box.x, detect_box.y,
                               detect_box.w, detect_box.h, TargetData_Parameter2);

            if (track_flag == 1)
            {
                // 取目标跟踪丢失前一帧图像和对应的目标信息
                int cx = TargetData_Parameter2.TCenter_W - TargetData_Parameter2.TSizeHalf_W;
                int cy = TargetData_Parameter2.TCenter_H - TargetData_Parameter2.TSizeHalf_H;
                int w = TargetData_Parameter2.TSize_W;
                int h = TargetData_Parameter2.TSize_H;
                similar(cx, cy, w, h); //坐标越界时的赋值问题
                cout << "cx:" << cx << " cy:" << cy << " w:" << w << " h:" << h << endl;

                Rect obj_rect(cx, cy, w, h);
                obj_img = Camera_GImg(obj_rect); //对应目标的原图像
                obj_x = TargetData_Parameter2.TCenter_W;
                obj_y = TargetData_Parameter2.TCenter_H;
            }
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
    char *datacfg = "model/tiny-yolo_oneClass/voc.data";
    char *cfgfile = "model/tiny-yolo_oneClass/tiny-yolo.cfg";
    char *weightfile = "model/tiny-yolo_oneClass/tiny-yolo.backup";
    double hier_thresh = 0.7;

    double thresh = 0.5;
    char *videofile = "/home/userver/Qt5.5.1/detect_track_kcf_1023_makefile/video/result133201.avi";

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
    int ret = ncc_track(detection, videofile, thresh, hier_thresh);

    return ret;
}
