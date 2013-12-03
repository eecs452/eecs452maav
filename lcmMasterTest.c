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
    
    //char data = (char*)frame->imageData;

    //for(i=0; i< frame->height; i++) for(j=0;j<;j++)
    cvZero(frame);  
    //frameG =      cvCreateImage(s, d ,1);
    //frameB =      cvCreateImage(s, d ,1);
    //frameBlur =   cvCreateImage(s, d ,1);
    //frameEdge =   cvCreateImage(s, d ,1);
    

    //printf("\n\n");
    //line_t *linePtr;
    //image_lines_t lineAndCircleInfo;
    //line_t* line;
    //circle_t* circle;
    while(cvWaitKey(30) == -1) {
        lcm_handle(lcm);
        //CvSeq* lines;
        //CvPoint pt1, pt2;
        //CvPoint *currentLine;

        //lineAndCircleInfo.imageTimeStamp = timestamp_now();
        
        //lines = findHoughLinesP();
        //drawHoughLinesP(lines);

        //int numLines   = lines->total;
        //int numCircles = 0;
        //lineAndCircleInfo.numLines   = numLines;
        //lineAndCircleInfo.numCircles = numCircles;

        //lineAndCircleInfo.line  =(line_t*)  malloc(numLines  *sizeof(line_t));
        //lineAndCircleInfo.circle=(circle_t*)malloc(numCircles*sizeof(circle_t));
        //line_t* line     = lineAndCircleInfo.line;
        //circle_t* circle = lineAndCircleInfo.circle;

        /*
        printf("I've found %2d lines for you! :Time = %lli\n",numLines,
                                            lineAndCircleInfo.imageTimeStamp);
        for(i=0; i<numLines; i++) {
            currentLine = (CvPoint*) cvGetSeqElem(lines, i);
            pt1 = currentLine[0];
            pt2 = currentLine[1];

            line[i].point[0].x = pt1.x;
            line[i].point[0].y = pt1.y;
            line[i].point[1].x = pt2.x;
            line[i].point[1].y = pt2.y;
            line[i].confidence = 50;

            printf("\tLine %2d=(%4d,%4d),(%4d,%4d)\n",i+1,
                                    lineAndCircleInfo.line[i].point[0].x,
                                    lineAndCircleInfo.line[i].point[0].y,
                                    lineAndCircleInfo.line[i].point[1].x,
                                    lineAndCircleInfo.line[i].point[1].y);
        }
        printf("\n\n\n");
        */

        //int encodedSize = image_lines_t_encoded_size(&lineAndCircleInfo);
        //uint8_t* buff = (uint8_t*)malloc(encodedSize);
        //if(!buff) return -1;
        //lineAndCircleInfo.transmissionTimeStamp = timestamp_now();
        //__image_lines_t_encode_array(buff, 0, encodedSize,
        //                                &lineAndCircleInfo,1);

        //image_lines_t_publish(lcm, "LINES_AND_CIRCLES_AND_IMAGES, OH_MY",&lineAndCircleInfo);
        
        //if(houghThreshold++ > 100) houghThreshold = 50;

        //free(line);
        //free(circle);
        //free(buff);
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

