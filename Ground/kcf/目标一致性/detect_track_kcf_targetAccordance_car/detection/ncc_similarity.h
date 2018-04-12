#ifndef NCC_SIMILARITY
#define NCC_SIMILARITY

#include <iostream>
#include <cmath>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"

#include "detection.h"
extern "C"
{
    #include "libyolo.h"

}

using namespace std;
using namespace cv;

const int SUB_RECT_WIDTH2 = 256;
const int SUB_RECT_HEIGHT2 = 256;

struct Region2
{
    int x, y;
    int width = SUB_RECT_WIDTH2;
    int height = SUB_RECT_HEIGHT2;
};

        box select_obj_box_all(Mat frame, Mat obj_img, detection_info **info, int num,int xx, int yy, int &detect_flag);
        box select_obj_box_big_ncc(Mat img, Mat obj_img, detection_info **info1, int tagnum, int xx, int yy);
        void av(Mat tagimg, double &average, double &variance);
        double  ncc_similarity(Mat origintag,Mat tag1,double originaverage, double originvariance, double average, double variance);
        void tune_coordinate(int *x0, int *x_range, const int &x_limit);


#endif // NCC_SIMILARITY

