#include <cv.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <highgui.h>
#include "common/timestamp.h"
#include <lcm/lcm.h>
#include "lcmtypes/image_lines_t.h"
//#include "lcmtypes/point_t.h"

#define DESIRED_WIDTH 176
#define DESIRED_HEIGHT 144

#define SHOW_IMAGES
#define DEBUG

IplImage* frame;        // Original Image  (Full Resolution)
IplImage* frameScale;   // Original Image  (scaled resolution)
IplImage* frameR;       // Red Coponent    (scaled)
IplImage* frameG;       // Green Coponent    (scaled)
IplImage* frameB;       // Blue Coponent    (scaled)
IplImage* frameBlur;    // Blured image
IplImage* frameEdge;    // Edge map
IplImage* frameTmp;

CvMemStorage* lineStorage;

int rhoRes = 4;
float thetaRes = CV_PI/45;

int edgeThresh = 100;

int minLineLength = 20;
int const maxLineLength = 300;

int minGapJump = 20;
int const maxGapJump = 50;

int houghThreshold = 50;
int const maxHoughThreshold = 300;

int blurDim = 2;

int i;

void initWindows(void);
CvSeq* findHoughLinesP(void);
void drawHoughLinesP(CvSeq* lines, IplImage* frame);
int main(int argc, char *argv[]) {
	lcm_t *lcm;
    // ttl = 0 for local, ttl = 1 for broadcast
    lcm = lcm_create("udpm://239.255.76.67:7667?ttl=1");
    //lcm = lcm_create("tcpq://192.168.1.102?ttl=0");
    if(!lcm)
        return 1;

    CvCapture* capture = cvCaptureFromCAM(-1); // open the default camera
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH,  176);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 144);
  
    IplImage* frame = cvQueryFrame(capture);  // Grab an initial image for analysis
    if(frame == NULL) {
        // For some reason the default camera could not be initialized
        printf("No image caputre device detected!!\n\n");
        exit(0);
    }
    //int height,width,step,channels;
    int i,j,k;

#ifdef DEBUG
    printf("Processing a %dx%d image with %d channels\n",
                                  frame->width,frame->height,frame->nChannels); 
#endif

#ifdef SHOW_IMAGES
    initWindows();
#endif
    lineStorage = cvCreateMemStorage(0);

    CvSize s = cvSize(frame->width, frame->height);
    int    d = frame->depth;
    frameScale = cvCreateImage(s,d,3);

    // create memory for images with only one channel
    frameR =      cvCreateImage(s, d ,1);
    frameG =      cvCreateImage(s, d ,1);
    frameB =      cvCreateImage(s, d ,1);
    frameBlur =   cvCreateImage(s, d ,1);
    frameEdge =   cvCreateImage(s, d ,1);
    
    cvSplit(frame, frameB, frameG, frameR, 0);
    cvSmooth(frameR, frameBlur, CV_GAUSSIAN, blurDim*2+1, 0,0,0);
    cvCanny(frameBlur, frameEdge, edgeThresh, edgeThresh*3,3);

#ifdef SHOW_IMAGES
    initWindows();
    cvShowImage("Original Image", frame);
    cvShowImage("Edges Detected", frameEdge);
#endif

    line_t *linePtr;
    image_lines_t lineAndCircleInfo;
    line_t* line;
    circle_t* circle;
    CvSeq* lines;
    CvPoint pt1, pt2;
    CvPoint *currentLine;
    
#ifdef SHOW_IMAGES
    while(cvWaitKey(30) == -1) {
        cvGrabFrame(capture);
#else
    while(cvGrabFrame(capture)) {
#endif
        lineAndCircleInfo.imageTimeStamp = timestamp_now();
        frame = cvRetrieveFrame(capture,0);
        cvSplit(frame, frameB, frameG, frameR, 0);
        cvSmooth(frameR, frameBlur, CV_GAUSSIAN, blurDim*2+1, 0,0,0);
        cvCanny(frameBlur, frameEdge, edgeThresh, edgeThresh*3,3);
        
        lines = findHoughLinesP();
#ifdef SHOW_IMAGES
        cvShowImage("Original Image", frame);
        cvShowImage("Edges Detected", frameEdge);
        frameTmp = cvCloneImage(frame);
        drawHoughLinesP(lines, frameTmp);
        cvReleaseImage(&frameTmp);
#endif

        int numLines   = lines->total;
        int numCircles = 0;
        lineAndCircleInfo.numLines   = numLines;
        lineAndCircleInfo.numCircles = numCircles;

        lineAndCircleInfo.line  =(line_t*)  malloc(numLines  *sizeof(line_t));
        lineAndCircleInfo.circle=(circle_t*)malloc(numCircles*sizeof(circle_t));
        line_t* line     = lineAndCircleInfo.line;
        circle_t* circle = lineAndCircleInfo.circle;

#ifdef DEBUG
        printf("I've found %2d lines for you! :Time = %lli\n",numLines,
                                            lineAndCircleInfo.imageTimeStamp);
#endif
        for(i=0; i<numLines; i++) {
            currentLine = (CvPoint*) cvGetSeqElem(lines, i);
            pt1 = currentLine[0];
            pt2 = currentLine[1];

            line[i].point[0].x = pt1.x;
            line[i].point[0].y = pt1.y;
            line[i].point[1].x = pt2.x;
            line[i].point[1].y = pt2.y;
            line[i].confidence = 50;

#ifdef DEBUG
            printf("\tLine %2d=(%4d,%4d),(%4d,%4d)\n",i+1,
                                    lineAndCircleInfo.line[i].point[0].x,
                                    lineAndCircleInfo.line[i].point[0].y,
                                    lineAndCircleInfo.line[i].point[1].x,
                                    lineAndCircleInfo.line[i].point[1].y);
#endif
        }
#ifdef DEBUG
        printf("\n\n\n");
#endif

        int encodedSize = image_lines_t_encoded_size(&lineAndCircleInfo);
        //uint8_t* buff = (uint8_t*)malloc(encodedSize);
        //if(!buff) return -1;
        lineAndCircleInfo.transmissionTimeStamp = timestamp_now();
        //__image_lines_t_encode_array(buff, 0, encodedSize,
        //                                &lineAndCircleInfo,1);

        image_lines_t_publish(lcm, "LINES_AND_CIRCLES_AND_IMAGES, OH_MY",&lineAndCircleInfo);
        
        free(line);
        free(circle);
        //free(buff);
    }
    cvReleaseImage(&frame);
    cvReleaseImage(&frameR);
    cvReleaseImage(&frameG);
    cvReleaseImage(&frameB);
    cvReleaseImage(&frameScale);
    cvReleaseImage(&frameBlur);
    cvReleaseImage(&frameEdge);
    cvClearMemStorage(lineStorage);
    return 0;
}
CvSeq* findHoughLinesP(void){ // just to make main cleaner
    return cvHoughLines2(   frameEdge,
                            lineStorage,
                            CV_HOUGH_PROBABILISTIC,
                            rhoRes,
                            thetaRes,
                            houghThreshold+1,
                            minLineLength,
                            minGapJump);
}
void drawHoughLinesP(CvSeq* lines, IplImage* frame){
    int i;
    CvPoint *currentLine;
    CvPoint pt1, pt2;
    for(i=0; i<lines->total; i++) {
        currentLine = (CvPoint*) cvGetSeqElem(lines, i);
        pt1 = currentLine[0];
        pt2 = currentLine[1];
        cvLine(frame,pt1,pt2,cvScalar(0,0,255,0),0,CV_AA,0);
    }
    cvShowImage("Probablistic Hough", frame );

    return;
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
    tmpRow = 0; tmpCol = 1;
    cvNamedWindow("Edges Detected", CV_WINDOW_NORMAL);
    cvMoveWindow( "Edges Detected", tmpCol*scaleCol+offsetCol,
            tmpRow*scaleRow+offsetRow);
    tmpRow = 0; tmpCol = 2;
    cvNamedWindow("Probablistic Hough", CV_WINDOW_NORMAL);
    cvMoveWindow( "Probablistic Hough", tmpCol*scaleCol+offsetCol,
            tmpRow*scaleRow+offsetRow);
}

