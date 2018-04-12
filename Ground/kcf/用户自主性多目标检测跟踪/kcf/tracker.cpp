#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "kcftracker.hpp"
#include "tracker.h"

#include <dirent.h>

using namespace std;
using namespace cv;

bool HOG = true;
bool FIXEDWINDOW = false;
bool MULTISCALE = true;
bool SILENT = true;
bool LAB = false;

// Create KCFTracker object
KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);

Mat bef_tagImage;

//average similarity and average APCE
float average_simi, average_APCE;

//KCF track
//Mat image: current frame image
//Rect rect: target area(called when the trace is entered)
//char track_flag: the logo(called when the trace is entered)
char kcfTrack(cv::Mat image, cv::Rect &rect, char track_flag)
{
    //stable tracking result
    Rect result;

    //current target area
    Mat cur_tagImage;

    //before and current target similarity
    float similarity = 0, APCE = 0, value = 0;

    //stable tracking
    if(track_flag == 1){
        //calculate the current frame trace rerults
        result = tracker.update(image);

        if(result.x == 0XFFFF){
            return 0;
        }
        else{

            if((result.x + result.width) > 719){
                result.width = 719 - result.x;
            }
            if((result.y + result.height) > 575){
                result.height = 575 - result.y;
            }
            //current frame target area image
            cur_tagImage = image(result);
            //calculate response max response and APCE
            APCE = tracker.tagAreaPeak(bef_tagImage, cur_tagImage, similarity);
            //judgment value
            value = APCE * similarity;

            //replace target area
            bef_tagImage = cur_tagImage;
            rect = result;

            //continue trackings:yes or no  //value < 12 || similarity < 0.6 //|| APCE > (2.5 * average_APCE)
            if(average_APCE > (2.5 * APCE)){
                return 0;//no
            }
            else{

                if(result.x < 1 || result.y < 1 || result.x > 718 || result.y > 575){
                    return 0;
                }
                else{
                    //average similarity and APCE
                    average_simi = (average_simi + similarity) / 2;
                    average_APCE = (average_APCE + APCE) / 2;

                    //draw rectangle
                    rectangle( image, Point( result.x, result.y ), Point( result.x+result.width, result.y+result.height), Scalar( 0, 0, 255 ), 1, 8 );
                    return 1;//yes
                }
            }
        }
    }//enter tracking
    else{
        //tracking init
        tracker.init(rect, image);
        rectangle( image, Point( rect.x, rect.y ), Point( rect.x+rect.width, rect.y+rect.height), Scalar( 0, 255, 255 ), 1, 8 );
        //save target area
        bef_tagImage = image(rect);

        average_simi = 0;
        average_APCE = 0;

        return 1;
    }  
}
