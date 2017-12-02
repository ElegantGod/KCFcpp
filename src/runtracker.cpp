#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "kcftracker.hpp"

//#include <dirent.h>

using namespace std;
using namespace cv;

Mat frame;
Mat frameCopy;
bool leftButtonDownFlag = false;
Point originalPoint; //矩形框起点  
Point processPoint; //矩形框终点 
bool bInit = false;
void onMouse(int event,int x,int y,int flags,void *ustc)    
{       
	frameCopy=frame.clone();  
    if(event==CV_EVENT_LBUTTONDOWN)    
    {    
        leftButtonDownFlag=true; //标志位  
        originalPoint=Point(x,y);  //设置左键按下点的矩形起点  
        processPoint=originalPoint;  
		bInit = false;
    }    
    if(event==CV_EVENT_MOUSEMOVE&&leftButtonDownFlag)    
    {    
        processPoint=Point(x,y);  
        if(originalPoint!=processPoint)  
        {  
            //在复制的图像上绘制矩形  
            rectangle(frameCopy,originalPoint,processPoint,Scalar(255,0,0),2);  
        }  
        imshow("KCF",frameCopy);  
    }    
    if(event==CV_EVENT_LBUTTONUP)    
    {    
        leftButtonDownFlag=false; 
		imshow("KCF",frameCopy);  
    }      
} 

#define WIDTH 640
#define HEIGHT 360
int main(int argc, char* argv[]){

	if (argc > 5) return -1;

	bool HOG = true;
	bool FIXEDWINDOW = false;
	bool MULTISCALE = true;
	bool SILENT = false;
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

	// Tracker results
	Rect result;
	namedWindow("KCF");
	setMouseCallback("KCF",onMouse);

	// Using min and max of X and Y for groundtruth rectangle
	int x;//min(x1, min(x2, min(x3, x4)));
	int y;//min(y1, min(y2, min(y3, y4)));
	int width;//max(x1, max(x2, max(x3, x4))) - xMin;
	int height;//max(y1, max(y2, max(y3, y4))) - yMin;

	// Write Results
	ofstream resultsFile;
	string resultsPath = "output.txt";
	resultsFile.open(resultsPath);

	// Frame counter
	int nFrames = 0;

	VideoCapture vc("/disk1/chengw/video/264a.mp4");
	if(!vc.isOpened())  
    {  
        return -1;  
    }  
	while ( 1 )
	{
		if(!leftButtonDownFlag)
			vc.read(frame);
		if(frame.rows != HEIGHT ||frame.cols != WIDTH)
        {
			resize(frame,frame, Size(WIDTH, HEIGHT));
        }
		// Read each frame from the list
		if(originalPoint!=processPoint&&!leftButtonDownFlag && !bInit)
		{
			x = originalPoint.x;
			y = originalPoint.y;
			width = processPoint.x - originalPoint.x;
			height = processPoint.y - originalPoint.y;
			// First frame, give the groundtruth to the tracker
			tracker.init( Rect(x, y, width, height), frame );
			rectangle( frame, Point( x, y ), Point( x+width, y+height), Scalar( 0, 255, 255 ), 1, 8 );
			resultsFile << x << "," << y << "," << width << "," << height << endl;
			bInit = true;
		}
			// Update
		if(bInit)
		{
			result = tracker.update(frame);
			rectangle( frame, Point( result.x, result.y ), Point( result.x+result.width, result.y+result.height), Scalar( 0, 255, 255 ), 1, 8 );
			resultsFile << result.x << "," << result.y << "," << result.width << "," << result.height << endl;
		}
		if (!SILENT){
			imshow("KCF", frame);
			waitKey(1);
		}
	}
	resultsFile.close();

}
