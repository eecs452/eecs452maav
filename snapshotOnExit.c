#include <cv.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <highgui.h>
//#include "lcmtypes/point_t.h"

#define DESIRED_WIDTH 176
#define DESIRED_HEIGHT 144

IplImage* frame;        // Original Image  (scaled Resolution)

void initWindows(void);

int main(int argc, char *argv[]) {
    CvCapture* capture = cvCaptureFromCAM(-1); // open the default camera
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH,  176);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 144);
  
    IplImage* frameOrig = cvQueryFrame(capture);  // Grab an initial image for analysis
    if(frameOrig == NULL) {
        // For some reason the default camera could not be initialized
        printf("No image caputre device detected!!\n\n");
        exit(0);
    }
    int i,j,k;

    int resizeNeeded = 0;
    if(frameOrig->width > 200) {
        printf("Hey, the size is wrong!!\n\n");
        printf("the size is actually %d\n\n",frameOrig->width);
        resizeNeeded = 1;
    }

    if(resizeNeeded) {
        frame = cvCreateImage(cvSize(176,144), frameOrig->depth,3);
        cvResize(frameOrig, frame, CV_INTER_LINEAR);
    }
    else {
        frame = frameOrig;
    }

    printf("Processing a %dx%d image with %d channels\n",
                                  frame->width,frame->height,frame->nChannels); 

    CvSize s = cvSize(frame->width, frame->height);
    int    d = frame->depth;

    initWindows();
    cvShowImage("Original Image", frame);

    
    while(cvWaitKey(30) == -1) {
        cvGrabFrame(capture);
        
        frameOrig = cvRetrieveFrame(capture,0);
        if(resizeNeeded) cvResize(frameOrig, frame, CV_INTER_LINEAR);

        cvShowImage("Original Image", frame);

    }
    cvSaveImage("foo.png",frame,0);
    cvReleaseImage(&frame);
    return 0;
}
void initWindows(void) {
    // show the images
    int scaleRow = 330;
    int scaleCol = 420;
    int offsetRow = 30;
    int offsetCol = 80;
    int tmpRow = 0;
    int tmpCol = 0;

    // Open windows for images
    tmpRow = 0; tmpCol = 0;
    cvNamedWindow("Original Image", CV_WINDOW_NORMAL);
    cvMoveWindow( "Original Image", tmpCol*scaleCol+offsetCol,
            tmpRow*scaleRow+offsetRow);
}

