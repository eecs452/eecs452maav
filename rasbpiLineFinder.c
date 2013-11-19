#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <time.h>
#include <lcm/lcm.h>
#include "lcmtypes/image_lines_t.h"

#define DESIRED_WIDTH 256
//#define IMAGE_PRINT
//#define AUTO_ADJUST_THRESH

//using namespace cv;

int blurDim = DESIRED_WIDTH/100;
int const maxBlurDim = 10;

//int lowThreshold = 20;
int edgeThreshR = 100;
int edgeThreshG = 100;
int edgeThreshB = 100;
int const minEdgeThresh = 50;
int const maxEdgeThresh = 200;

int rhoRes = 4;
float thetaRes = CV_PI/45;

int minLineLength = 50;
int const maxLineLength = 300;

int minGapJump = 20;
int const maxGapJump = 50;

int houghThreshold = 100;
int const maxHoughThreshold = 300;
int const minHoughThreshold = 100;

//int edgeThresh = 1;
int ratio = 3;
//int kernal_size = 3;
unsigned int numLines;
unsigned int maxWhite;
unsigned int minWhite;
float rho, theta;

float* currentLine;
CvPoint* currentLineP;
CvPoint pt1, pt2;

CvCapture* capture; // open the default camera

IplImage* frame;        // Original Image  (Full Resolution)
IplImage* frameScale;   // Original Image  (scaled resolution)
IplImage* frameR;       // Blue Comonent   (Scaled)
IplImage* frameG;       // Green Component (scaled)
IplImage* frameB;       // Red Coponent    (scaled)
IplImage* frameRblur;   // Blured image
IplImage* frameGblur;   // Blured image
IplImage* frameBblur;   // Blured image
IplImage* frameRedge;   // Edge map
IplImage* frameGedge;   // Edge map
IplImage* frameBedge;   // Edge map
IplImage* frameTmp;


CvSeq* lines;
CvMemStorage* lineStorage;
uchar *data;

void onTrackbarSlide(int foo){;}
void blurHandler(int);
void cannyHandler(int);
void houghHandler(int);
void drawHoughLines(int);
void drawHoughLinesP(int);
unsigned int countWhite(IplImage*);
int adjustWhiteThresh(int thresh, int whiteCount);

int main(int argc, char *argv[]) {
  CvCapture* capture = cvCaptureFromCAM(-1); // open the default camera
  //cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH,  320);
  //cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 240);
  cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH,  176);
  cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 144);
  
  IplImage* frame = cvQueryFrame(capture);  // Grab an initial image for analysis
  if(frame == NULL) {
      // For some reason the default camera could not be initialized
      printf("No image caputre device detected!!\n\n");
      exit(0);
  }

  int height,width,step,channels;
  int i,j,k;

  printf("Processing a %dx%d image with %d channels\n",
                                  frame->width,frame->height,frame->nChannels); 

  CvSize s = cvGetSize(frame);
  int d = frame->depth;
  //frameScale = cvCreateImage(cvSize(DESIRED_WIDTH,frame->height*DESIRED_WIDTH/frame->width),d,3);
  //cvResize(frame, frameScale, CV_INTER_LINEAR);
  //printf("Resized image to %dx%d with %d channels\n",
  //              frameScale->width,frameScale->height,frameScale->nChannels); 
  //s = cvGetSize(frameScale);
  //d = frameScale->depth;

  maxWhite = 0.035*(s.width*s.height);
  minWhite = 0.020*(s.width*s.height);

  // create memory for images with only one channel
  frameR =      cvCreateImage(s, d ,1);
  frameG =      cvCreateImage(s, d ,1);
  frameB =      cvCreateImage(s, d ,1);
  frameRblur =  cvCreateImage(s, d ,1);
  frameGblur =  cvCreateImage(s, d ,1);
  frameBblur =  cvCreateImage(s, d ,1);
  frameRedge =  cvCreateImage(s, d ,1);
  frameGedge =  cvCreateImage(s, d ,1);
  frameBedge =  cvCreateImage(s, d ,1);

  lineStorage = cvCreateMemStorage(0);

  time_t start, end;
  double fps;
  int counter = 0;
  double sec;

  time(&start);
    // lopp until a key is pressed
    //while(cvWaitKey(10) == -1) {
       while(cvGrabFrame(capture)) {
            frame = cvRetrieveFrame(capture,0);

            //cvResize(frame, frameScale,CV_INTER_LINEAR);
            cvSplit(frame, frameB, frameG, frameR, 0);
            //cvSmooth(frameR, frameRblur, CV_GAUSSIAN, blurDim*2+1, 0);
            //cvSmooth(frameG, frameGblur, CV_GAUSSIAN, blurDim*2+1, 0);
            cvSmooth(frameB, frameBblur, CV_GAUSSIAN, blurDim*2+1, 0,0,0);

            //cvCanny(frameRblur, frameRedge, edgeThreshR, edgeThreshR*ratio);
            //cvCanny(frameGblur, frameGedge, edgeThreshG, edgeThreshG*ratio);
            cvCanny(frameBblur, frameBedge, edgeThreshB, edgeThreshB*ratio,3);

            houghHandler(0);

            //cvShowImage("Original Image",   frame);
            //cvShowImage("Scaled Image",     frameScale);
            //cvShowImage("Blue Channel",     frameB );
            //cvShowImage("Blue Blured Image",    frameBblur );
            //cvShowImage( "Blue Edges Detected",  frameBedge );

            if((++counter)%100 == 0){
                time(&end);
                sec = difftime(end, start);
                fps = counter/sec;
                printf("\t\tFPS = %.2f\n",fps);
            }
        //}
    }
    // release the image
    //cvReleaseImage(&img );
    return 0;
}
int adjustWhiteThresh(int thresh, int whiteCount) {
    if(     whiteCount>maxWhite) thresh++;
    else if(whiteCount<minWhite) thresh--;
    
    if(     thresh < minEdgeThresh) thresh = minEdgeThresh;
    else if(thresh > maxEdgeThresh) thresh = maxEdgeThresh;

    return thresh;
}
unsigned int countWhite(IplImage* image) {
    int i, j;
    unsigned int sum = 0;
    int step = image->widthStep;
    uchar *data = (uchar *)image->imageData;
    for(i=0;i<image->height;i++) for(j=0;j<image->width;j++) {
        if(data[i*step+j]) sum++;
    }
    return sum;
}

void blurHandler(int foo) {
    //cvSmooth(imgB, imgTmp, CV_GAUSSIAN, blurDim*2+1, 0);
    //cvSmooth(imgRes, imgTmp2, CV_GAUSSIAN, blurDim*2+1, 0);
    //cvShowImage("Blue Blured Image", imgTmp2 );
    //cannyHandler(0);
}
void cannyHandler(int foo) {
    //cvCanny(imgTmp, imgE, lowThreshold, lowThreshold*ratio);
    //cvCanny(imgTmp2, imgE2, lowThreshold, lowThreshold*ratio);
    //cvShowImage("Edges Detected", imgE2 );
    //houghHandler(0);
}
void houghHandler(int foo){
    //lines = cvHoughLines2(frameBedge , lineStorage, CV_HOUGH_STANDARD,
    //        1, CV_PI/180, houghThreshold+1);
    lines = cvHoughLines2(frameBedge, lineStorage, CV_HOUGH_PROBABILISTIC,
            rhoRes, thetaRes, houghThreshold+1,minLineLength,minGapJump);
    //drawHoughLines(0);
    printf("\t\t\tNumber of lines detected = %d\n", lines->total);
    drawHoughLinesP(0);
}
void drawHoughLinesP(int foo){
    int i;
    float a,b,x0,y0;
    //frameTmp = cvCloneImage(frame);
    numLines = lines->total;
    for(i=0; i<numLines; i++) {
        currentLineP = (CvPoint*) cvGetSeqElem(lines, i);
        pt1 = currentLineP[0];
        pt2 = currentLineP[1];

        cvLine(frameTmp,pt1,pt2,cvScalar(0,0,255,0),1,CV_AA,0);
    }

    //cvShowImage("Final Image", frameTmp );
    //cvReleaseImage(&frameTmp);
}

void drawHoughLines(int foo){
    int i;
    float a,b,x0,y0;
    frameTmp = cvCloneImage(frameScale);
    numLines = lines->total;
    for(i=0; i<numLines; i++) {
        currentLine = (float*) cvGetSeqElem(lines, i);
        rho   = (float)currentLine[0];
        theta = (float)currentLine[1];
        a = cos(theta);
        b = sin(theta);
        y0 = b*rho;
        x0 = a*rho;
        pt1.x = cvRound(x0 - 1000.0*b);
        pt1.y = cvRound(y0 + 1000.0*a);
        pt2.x = cvRound(x0 + 1000.0*b);
        pt2.y = cvRound(y0 - 1000.0*a);
        cvLine(frameTmp,pt1,pt2,cvScalar(0,0,255,0),1,CV_AA,0);
    }
    //printf("Number of lines detected = %d\n",numLines);

    cvShowImage("Final Image", frameTmp );
    cvReleaseImage(&frameTmp );
}





