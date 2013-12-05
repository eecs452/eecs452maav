#include <cv.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <highgui.h>
#include "common/timestamp.h"
#include <lcm/lcm.h>
#include "lcmtypes/image_lines_t.h"
#include "lcmtypes/point_t.h"
#include "typedefs.h"

#define DESIRED_WIDTH 176
#define DESIRED_HEIGHT 144
#define WHITE cvScalar(255,255,255,0)
#define GREEN cvScalar(0,255,0,0)
#define RED   cvScalar(0,0,255,0)

IplImage* frame;        // Original Image  (Full Resolution)
IplImage* frameTmp;


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
void drawHoughLinesP(int, line_t*, IplImage*);
void drawHoughCircles(int, circle_t*, IplImage*);


static void functionPtr(const lcm_recv_buf_t *rbuf,
             const char *channel, const image_lines_t *msg, void *user) {
    const int maxLines   = 15;
    const int maxCircles = 5;

    int numLines = (int)msg->numLines;
    int numCircles = (int)msg->numCircles;

    Color_u color;
    

    if(numLines > maxLines) numLines = maxLines;
    if(numCircles > maxCircles) numCircles = maxCircles;

    frameTmp = cvCloneImage(frame);
    drawHoughLinesP(numLines,msg->line, frameTmp);
    drawHoughCircles(numCircles,msg->circle, frameTmp);
    cvShowImage("Probablistic Hough", frameTmp );
    cvReleaseImage(&frameTmp);
    
    system("clear");
    printf("\n\n\nTime: %lli\n",msg->imageTimeStamp);
    printf("\nFound %2d lines.\n",numLines);

	for(i=0;i<numLines;i++) {
        printf("\tLine %2d = (%4d,%4d),(%4d,%4d)\t",i+1,
		    msg->line[i].point[0].x,
	    	msg->line[i].point[0].y,
		    msg->line[i].point[1].x, 
	        msg->line[i].point[1].y);
            
            color.raw = msg->line[i].color;
            int R = color.f.R;
            int G = color.f.G;
            int B = color.f.B;
            //CvScalar rgb = cvScalar(color.f.R*256,color.f.G*256,color.f.B*256,0);
            if(R&G&B) {
                printf("Color: White\n");
            }
            else if(!R &  G & !B) {
                printf("Color: Green\n");
            }
            else if( R & !G & !B) {
                printf("Color: Red\n");
            }
            else {
                printf("Color: Unkonwn\n");
            }
    }
    for(;i<maxLines;i++) printf("\n");
    
    printf("Found %2d circles.\n",numCircles);
    for(i=0;i<numCircles;i++) {
        printf("\tCircle %2d, Center = (%4d,%4d), Radius = %2d\n",i+1,
            (int)msg->circle[i].center.x,
            (int)msg->circle[i].center.y, 
            (int)msg->circle[i].radius); 
    }
    for(;i<maxCircles;i++) printf("\n");
    //printf("Strenth=%4i, (R,G,B)=(%d,%d,%d)\n",color1.raw,color1.f.R,color1.f.G,color1.f.B);

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

    //while(cvWaitKey(30) == -1) {
    while(1) { // loop forever (press ctrl+c to exit)
        lcm_handle(lcm);
        cvWaitKey(10);
    }
    return 0;
}
void drawHoughCircles(int numCircles, circle_t* circles, IplImage* frameTmp){
    int i;
    
    CvPoint center;
    int16_t radius;
    for(i=0; i<numCircles; i++) {
        center.x = circles[i].center.x;
        center.y = circles[i].center.y;
        radius   = circles[i].radius;

        cvCircle(frameTmp, center, radius, WHITE, 0, CV_AA, 0); 
    }

    return;
}
void drawHoughLinesP(int numLines, line_t* lines, IplImage* frameTmp){
    int i;
    
    Color_u color;
    CvScalar scalarColor;

    CvPoint pt1, pt2;

    for(i=0; i<numLines; i++) {
        pt1.x = lines[i].point[0].x;
    	pt1.y = lines[i].point[0].y;
	    pt2.x = lines[i].point[1].x;
        pt2.y = lines[i].point[1].y;
        
        color.raw = lines[i].color;

        scalarColor = cvScalar(color.f.B*256, color.f.G*256, color.f.R*256,0);
        cvLine(frameTmp,pt1,pt2,scalarColor,0,CV_AA,0);
    }

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
    tmpRow = 0; tmpCol = 2;
    cvNamedWindow( "Probablistic Hough", CV_WINDOW_NORMAL);
    cvResizeWindow("Probablistic Hough", 900,1000);
    cvMoveWindow(  "Probablistic Hough", 1000,0);
}

