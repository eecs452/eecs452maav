#include <cv.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <highgui.h>
#include "common/timestamp.h"
#include <lcm/lcm.h>
#include "lcmtypes/image_lines_t.h"
#include "lcmtypes/point_t.h"

#define DESIRED_WIDTH 176
#define DESIRED_HEIGHT 144

IplImage* frame;        // Original Image  (Full Resolution)
IplImage* frameTmp;

//CvMemStorage* lineStorage;

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
lcm_t *lcm;

void initWindows(void);
CvSeq* findHoughLinesP(void);
void drawHoughLinesP(int, line_t*);


static void functionPtr(const lcm_recv_buf_t *rbuf,
             const char *channel, const image_lines_t *msg, void *user) {

    drawHoughLinesP(msg->numLines,msg->line);

    printf("\n\n\nI have %d lines.Time: %lli\n",msg->numLines,msg->imageTimeStamp);

	for(i=0;i<msg->numLines;i++)
    printf("\tLine %2d=(%4d,%4d),(%4d,%4d)\n",i+1,
		msg->line[i].point[0].x,
		msg->line[i].point[0].y,
		msg->line[i].point[1].x, 
	        msg->line[i].point[1].y);


    printf("Message Recieved!! from ");
    printf(channel);
    printf("\n");
    return;
}


int main(int argc, char *argv[]) {
    lcm = lcm_create("udpm://239.255.76.67:7667?ttl=1");
    if(!lcm)
        return 1;

    image_lines_t_subscription_t* dataSub = image_lines_t_subscribe(lcm,
            "LINES_AND_CIRCLES_AND_IMAGES, OH_MY",
            &functionPtr, NULL);

    initWindows();

    // create memory for images with only one channel
    frame = cvCreateImage(cvSize(DESIRED_WIDTH, DESIRED_HEIGHT),
                                           IPL_DEPTH_8U ,3);
    
    cvZero(frame);  

    while(cvWaitKey(30) == -1) {
        lcm_handle(lcm);
    }
    return 0;
}
void drawHoughLinesP(int numLines, line_t* lines){
    CvSize s = cvSize(DESIRED_WIDTH,DESIRED_HEIGHT);
    int d = IPL_DEPTH_8U;
    frameTmp = cvCloneImage(frame);
    int i;
    
    CvPoint pt1, pt2;

    for(i=0; i<numLines; i++) {
        pt1.x = lines[i].point[0].x;
    	pt1.y = lines[i].point[0].y;
	    pt2.x = lines[i].point[1].x;
        pt2.y = lines[i].point[1].y;
        cvLine(frameTmp,pt1,pt2,cvScalar(0,0,255,0),0,CV_AA,0);
    }
    cvShowImage("Probablistic Hough", frameTmp );
    cvReleaseImage(&frameTmp);

    return;
}
void initWindows(void) {
    int scaleRow = 330;
    int scaleCol = 420;
    int offsetRow = 30;
    int offsetCol = 80;
    int tmpRow = 0;
    int tmpCol = 0;

    // Open windows for images
    tmpRow = 0; tmpCol = 0;
    //cvNamedWindow("Original Image", CV_WINDOW_NORMAL);
    //cvMoveWindow( "Original Image", tmpCol*scaleCol+offsetCol,
    //        tmpRow*scaleRow+offsetRow);
    tmpRow = 0; tmpCol = 1;
    //cvNamedWindow("Edges Detected", CV_WINDOW_NORMAL);
    //cvMoveWindow( "Edges Detected", tmpCol*scaleCol+offsetCol,
    //        tmpRow*scaleRow+offsetRow);
    tmpRow = 2; tmpCol = 2;
    cvNamedWindow("Probablistic Hough", CV_WINDOW_NORMAL);
    cvMoveWindow( "Probablistic Hough", tmpCol*scaleCol+offsetCol,
            tmpRow*scaleRow+offsetRow);
}

