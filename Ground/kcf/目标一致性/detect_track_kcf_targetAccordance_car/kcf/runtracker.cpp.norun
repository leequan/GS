#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "kcftracker.hpp"

#include <dirent.h>

using namespace std;
using namespace cv;

int main(int argc, char* argv[]){

	if (argc > 5) return -1;

	bool HOG = true;
	bool FIXEDWINDOW = false;
	bool MULTISCALE = true;
	bool SILENT = true;
	bool LAB = false;

	for(int i = 0; i < argc; i++){
		if ( strcmp (argv[i], "hog") == 0 )
			HOG = true;
		if ( strcmp (argv[i], "fixed_window") == 0 )
			FIXEDWINDOW = true;
		if ( strcmp (argv[i], "singlescale") == 0 )
			MULTISCALE = false;
		if ( strcmp (argv[i], "show") == 0 )
			SILENT = false;
		if ( strcmp (argv[i], "lab") == 0 ){
			LAB = true;
			HOG = true;
		}
		if ( strcmp (argv[i], "gray") == 0 )
			HOG = false;
	}
	
	// Create KCFTracker object
	KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);

	// Frame readed
	Mat frame;

	// Tracker results
	Rect result;

	// Path to list.txt
//	ifstream listFile;
//    string fileName = "/home/userver/Downloads/KCFcpp-master-build/images.txt";
//  	listFile.open(fileName);
//    string name;
//    getline(listFile, name);
//    cout<<name<<endl;
    //frame01 = imread(name, CV_LOAD_IMAGE_COLOR);


  	// Read groundtruth for the 1st frame
  	ifstream groundtruthFile;
	string groundtruth = "region.txt";
  	groundtruthFile.open(groundtruth);

  	string firstLine;
  	getline(groundtruthFile, firstLine);
	groundtruthFile.close();
  	
  	istringstream ss(firstLine);

  	// Read groundtruth like a dumb
  	float x1, y1, x2, y2, x3, y3, x4, y4;
  	char ch;
	ss >> x1;
	ss >> ch;
	ss >> y1;
	ss >> ch;
	ss >> x2;
	ss >> ch;
	ss >> y2;
	ss >> ch;
	ss >> x3;
	ss >> ch;
	ss >> y3;
	ss >> ch;
	ss >> x4;
	ss >> ch;
	ss >> y4; 

	// Using min and max of X and Y for groundtruth rectangle
//	float xMin =  min(x1, min(x2, min(x3, x4)));
//	float yMin =  min(y1, min(y2, min(y3, y4)));
//	float width = max(x1, max(x2, max(x3, x4))) - xMin;
//  float height = max(y1, max(y2, max(y3, y4))) - yMin;
 //float xMin = 476, yMin = 240, width = 25, height = 21;//55.avix 1: 475  y1: 240 width: 27  height: 21
  //   float xMin = 470, yMin = 240, width = 36, height = 24;//02.avi
 // float xMin = 270, yMin = 272, width = 22, height = 19;//41.avi x1: 270  y1: 272 width: 22  height: 19
 float xMin = 136, yMin = 275, width = 21, height = 17;//51.avi x1: 136  y1: 275 width: 21  height: 17
 //float xMin = 355, yMin = 279, width = 17, height = 15;//54.avi x1: 355  y1: 280 width: 17  height: 13
 //float xMin = 94, yMin = 459, width = 21, height = 19;//03.avi x1: 94  y1: 459 width: 21  height: 19
 //float xMin = 295, yMin = 123, width = 17, height = 21;//result082401.avi x1: 295  y1: 123 width: 17  height: 21
 //float xMin = 388, yMin = 344, width = 18, height = 19;//result082401.avi x1: 388  y1: 344 width: 18  height: 19
 //float xMin = 4, yMin = 165, width = 46, height = 33;//car.avi x1: 4  y1: 165 width: 46  height: 33

	
	// Read Images
	ifstream listFramesFile;
    string listFrames = "images.txt";
	listFramesFile.open(listFrames);
	string frameName;


	// Write Results
	ofstream resultsFile;
	string resultsPath = "output.txt";
	resultsFile.open(resultsPath);

	// Frame counter
	int nFrames = 0;

    //Display framenum
    char str[25];

    //before and current target area
    Mat bef_tagImage, cur_tagImage;

    //before and current target similarity
    float similarity = 0, APCE = 0, value = 0;

    //Read video path
    string path = "/home/userver/concise-GOTURN-master01/videos/";
    string video=path+"51.avi";
    VideoCapture capture(video);
    if(!capture.isOpened())
    {
        cerr<<"Failed to open a video"<<endl;
        return -1;
    }

    for(;;){
    //while ( getline(listFramesFile, frameName) ){
    //	frameName = frameName;

		// Read each frame from the list
    //    frame = imread(frameName, CV_LOAD_IMAGE_COLOR);
        //frame = imread("/home/userver/Downloads/KCFcpp-master-build/car/00000001.jpg", CV_LOAD_IMAGE_COLOR);

        capture >> frame;
        if(frame.empty())
            break;

        // First frame, give the groundtruth to the tracker
		if (nFrames == 0) {
			tracker.init( Rect(xMin, yMin, width, height), frame );
			rectangle( frame, Point( xMin, yMin ), Point( xMin+width, yMin+height), Scalar( 0, 255, 255 ), 1, 8 );
			resultsFile << xMin << "," << yMin << "," << width << "," << height << endl;

            result.x = (int)xMin;
            result.y = (int)yMin;
            result.width = (int)width;
            result.height = (int)height;

            bef_tagImage = frame(result);
		}
		// Update
		else{
			result = tracker.update(frame);
			rectangle( frame, Point( result.x, result.y ), Point( result.x+result.width, result.y+result.height), Scalar( 0, 255, 255 ), 1, 8 );
			resultsFile << result.x << "," << result.y << "," << result.width << "," << result.height << endl;

            cur_tagImage = frame(result);
            APCE = tracker.tagAreaPeak(bef_tagImage, cur_tagImage, similarity);

            bef_tagImage = cur_tagImage;
		}
        value = similarity * APCE;
        printf("nFrames = %d, similarity = %f, APCE = %f, value = %f\n", nFrames, similarity, APCE, value);
		nFrames++;

        Mat display;
        frame.copyTo(display);
        rectangle(display,result,CV_RGB(255,0,0), 2);
        sprintf(str,"%d",nFrames);
        putText(display, str, cv::Point(100,100), FONT_HERSHEY_DUPLEX, 2.0, cv::Scalar(255,255,255), 1, 8, 0);
        imshow("tracking result",display);
        waitKey(100);

        if(nFrames == 50){
            break;
        }

//		if (!SILENT){
//			imshow("Image", frame);
//			waitKey(1);
//		}
	}
	resultsFile.close();

    //listFile.close();

}
