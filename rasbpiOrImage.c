#include <cv.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <highgui.h>
#include "common/timestamp.h"
#include <lcm/lcm.h>
#include "lcmtypes/image_lines_t.h"

#define DESIRED_WIDTH 176
#define DESIRED_HEIGHT 144

//#define SHOW_IMAGES
//#define DEBUG
#define SEND_LCM
#define DETECT_CIRCLES
#define LINE_COLOR

IplImage* frame;        // Original Image  (scaled Resolution)
IplImage* frameScale;   // Original Image  (scaled resolution)
IplImage* frameR;       // Red Coponent    (scaled)
IplImage* frameG;       // Green Coponent    (scaled)
IplImage* frameB;       // Blue Coponent    (scaled)
IplImage* frameOR;	// Red and green Bitwise-or'd image
IplImage* frameBlur;    // Blured image
IplImage* frameEdge;    // Edge map
IplImage* frameTmp;

CvMemStorage* lineStorage;

int rhoRes = 4;
float thetaRes = CV_PI/45;
float centerOverlap = 32;

int edgeThresh = 100;

int minLineLength = 20;
int const maxLineLength = 300;

int minGapJump = 20;
int const maxGapJump = 50;

int houghThreshold = 50;
int const maxHoughThreshold = 300;

int circleThreshold = 20;
int circleMinRadiusThreshold = 4;
int circleMaxRadiusThreshold = 100;

int blurDim = 2;

int i;
unsigned int numCircles;
float* currentCircle; // should be int16_t* or float* ?
CvPoint center;
CvMemStorage* circleStorage;

void fixFocus(void);
void initWindows(void);
CvSeq* findHoughLinesP(void);
CvSeq* findHoughCircles(void);
void drawHoughLinesP(CvSeq* lines, IplImage* frame);
int8_t detectLineColor(CvPoint, CvPoint);

int main(int argc, char *argv[]) {
#ifdef SEND_LCM
	lcm_t *lcm;
    // ttl = 0 for local, ttl = 1 for broadcast
    lcm = lcm_create("udpm://239.255.76.67:7667?ttl=1");
    //lcm = lcm_create("tcpq://192.168.1.102?ttl=0");
    if(!lcm)
        return 1;
#endif
    CvCapture* capture = cvCaptureFromCAM(-1); // open the default camera
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH,  176);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 144);
  
    IplImage* frameOrig = cvQueryFrame(capture);  // Grab an initial image for analysis
    if(frameOrig == NULL) {
        // For some reason the default camera could not be initialized
        printf("No image caputre device detected!!\n\n");
        exit(0);
    }

    // turn off auto focous and manually set the focus to inf
    fixFocus();
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

#ifdef DEBUG
    printf("Processing a %dx%d image with %d channels\n",
                                  frame->width,frame->height,frame->nChannels); 
#endif

#ifdef SHOW_IMAGES
    initWindows();
#endif
    lineStorage = cvCreateMemStorage(0);
    circleStorage = cvCreateMemStorage(0);

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
    int colorSwitch = 0;
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
    CvSeq* circles;
    CvPoint pt1, pt2;
    CvPoint *currentLine;

    int retVal = 0;
#ifdef SHOW_IMAGES
    while(cvWaitKey(100) == -1) {
        cvGrabFrame(capture);
#else
    //int retVal = 0; moved up a few lines by dh
    while(1) {
        //retVal = cvGrabFrame(capture);
        //printf("retVal = %d\n", retVal);
#endif
        lineAndCircleInfo.imageTimeStamp = timestamp_now();
        
        //frameOrig = cvRetrieveFrame(capture,0);
        frameOrig = cvQueryFrame(capture);

        if(resizeNeeded) cvResize(frameOrig, frame, CV_INTER_LINEAR);
        else frame = frameOrig;
        retVal = (int)frame;

        cvSplit(frame, frameB, frameG, frameR, 0);
        cvOr(frameR,frameG,frameOR);
#ifdef SHOW_IMAGES
	cvShowImage("Or'd Image", frameOR);
#endif
	cvSmooth(frameOR, frameBlur, CV_GAUSSIAN, blurDim*2+1, 0,0,0);
        /*if(colorSwitch) {
            colorSwitch = 0;
            cvSmooth(frameR, frameBlur, CV_GAUSSIAN, blurDim*2+1, 0,0,0);
        }
        else {
            colorSwitch = 1;
            cvSmooth(frameG, frameBlur, CV_GAUSSIAN, blurDim*2+1, 0,0,0);
        }*/
        cvCanny(frameBlur, frameEdge, edgeThresh, edgeThresh*3,3);
        
        lines = findHoughLinesP();
        int numLines     = lines->total;

        int numCircles = 0;
#ifdef DETECT_CIRCLES
        circles = findHoughCircles();
        numCircles   = circles->total;
#endif

#ifdef SHOW_IMAGES
        cvShowImage("Original Image", frame);
        cvShowImage("Edges Detected", frameEdge);
        frameTmp = cvCloneImage(frame);
        drawHoughLinesP(lines, frameTmp);
        cvReleaseImage(&frameTmp);
#endif

        
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
            line[i].color = 0;
#ifdef LINE_COLOR
            line[i].color = detectLineColor(pt1,pt2);
#endif

#ifdef DEBUG
            printf("\tLine %2d=(%4d,%4d),(%4d,%4d)\n",i+1,
                                    lineAndCircleInfo.line[i].point[0].x,
                                    lineAndCircleInfo.line[i].point[0].y,
                                    lineAndCircleInfo.line[i].point[1].x,
                                    lineAndCircleInfo.line[i].point[1].y);

#ifdef LINE_COLOR
            if (line[i].color==1)
                printf("\tLine color: white\n");
            else if (line[i].color==2)
                printf("\tLine color: red\n");
            else if (line[i].color==3)
                printf("\tLine color: green\n");
            else
                printf("\tLine color: unknown\n");
#endif

#endif
        }

#ifdef DETECT_CIRCLES
#ifdef DEBUG
        printf("I've found %2d circles for you! :Time = %lli\n",numCircles,
                                            lineAndCircleInfo.imageTimeStamp);
#endif
        CvPoint center;
        int16_t radius;
        for(i=0; i<numCircles; i++) {
            currentCircle = (float*) cvGetSeqElem(circles, i);
            center.x = (int16_t)currentCircle[0];
            center.y = (int16_t)currentCircle[1];
            radius   = (int16_t)currentCircle[2]; 

            // note to David:  this works now

            circle[i].center.x = center.x;
            circle[i].center.y = center.y;
            circle[i].radius   = radius;
            circle[i].confidence = 50;

#ifdef DEBUG
            printf("\tCircle %2d: c=(%4d,%4d), r=%4d\n",i+1,
                                    lineAndCircleInfo.circle[i].center.x,
                                    lineAndCircleInfo.circle[i].center.y,
                                    lineAndCircleInfo.circle[i].radius );
#endif

        }
        //printf("outside of circle detection while loop\n"); // delete this later
#endif


        int nSize = frame->nSize;
        //int64_t imageSize = frame->imageSize;
        int imageSize = 0;
        lineAndCircleInfo.nSize = nSize;
        lineAndCircleInfo.imageSize = imageSize;
        //lineAndCircleInfo.rawIplData = &frame;
        lineAndCircleInfo.imageData = frame->imageData;
        int64_t encodedSize = image_lines_t_encoded_size(&lineAndCircleInfo);

        lineAndCircleInfo.transmissionTimeStamp = timestamp_now();
        //__image_lines_t_encode_array(buff, 0, encodedSize,
        //                                &lineAndCircleInfo,1);
#ifdef SEND_LCM
        image_lines_t_publish(lcm, "LINES_AND_CIRCLES_AND_IMAGES, OH_MY",&lineAndCircleInfo);
#endif

#ifdef DEBUG
        printf("Image size = %d\n\n\n",imageSize);
        //printf("Embedded size = %lli\n\n\n",encodedSize);
        //usleep(500000);
#endif
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
CvSeq* findHoughCircles(void) {
    return cvHoughCircles(  frameEdge,
                            circleStorage,
                            CV_HOUGH_GRADIENT,
                            1,
                            centerOverlap,
                            200,
                            circleThreshold,
                            circleMinRadiusThreshold,
                            circleMaxRadiusThreshold);
}
int8_t detectLineColor(CvPoint pt1, CvPoint pt2) {
    CvPoint midpt, near;
    uchar R,G,B;
    uchar highThresh=180;
    uchar lowThresh=150;
    int8_t colorVal;
    //float theta;
    //float PI = 3.14159265; // or should we use a double?

    midpt.x = (pt1.x+pt2.x) >> 1;
    midpt.y = (pt1.y+pt2.y) >> 1;

    B = ((uchar*)(frame->imageData + frame->widthStep*midpt.y))[midpt.x*3  ];
    G = ((uchar*)(frame->imageData + frame->widthStep*midpt.y))[midpt.x*3+1];
    R = ((uchar*)(frame->imageData + frame->widthStep*midpt.y))[midpt.x*3+2];

    if ((R>highThresh) && (G>highThresh) && (B>highThresh)) // white line
        colorVal = 1;
    else if ((R>highThresh) && (G<lowThresh) && (B<lowThresh)) // red line
        colorVal = 2;
    else if ((R>highThresh) && (G<lowThresh) && (B<lowThresh)) // green line
        colorVal = 3;
    else { // midpoint color is unknown, try a nearby pixel
        colorVal = 0;
        /*
        theta = atan2(pt1.y-pt2.y, pt1.x-pt2.x); // angle of line
        theta = theta + PI/2; // angle of perpendicular line
        near.x = cvRound( midpt.x + 1.5*cos(theta) ); // a point perpendicular to line,
        near.y = cvRound( midpt.y + 1.5*sin(theta) ); // on one side of the midpt

        B = ((uchar*)(frame->imageData + frame->widthStep*near.y))[near.x*3  ];
        G = ((uchar*)(frame->imageData + frame->widthStep*near.y))[near.x*3+1];
        R = ((uchar*)(frame->imageData + frame->widthStep*near.y))[near.x*3+2];
    
        if ((R>highThresh) && (G>highThresh) && (B>highThresh)) // white line
            colorVal = 1;
        else if ((R>highThresh) && (G<lowThresh) && (B<lowThresh)) // red line
            colorVal = 2;
        else if ((R>highThresh) && (G<lowThresh) && (B<lowThresh)) // green line
            colorVal = 3;
        else { // midpoint color is STILL unknown, try another nearby pixel

            near.x = cvRound( midpt.x - 1.5*cos(theta) ); // a point perpendicular to line,
            near.y = cvRound( midpt.y - 1.5*sin(theta) ); // on other side of midpt

            B = ((uchar*)(frame->imageData + frame->widthStep*near.y))[near.x*3  ];
            G = ((uchar*)(frame->imageData + frame->widthStep*near.y))[near.x*3+1];
            R = ((uchar*)(frame->imageData + frame->widthStep*near.y))[near.x*3+2];
    
            if ((R>highThresh) && (G>highThresh) && (B>highThresh)) // white line
                colorVal = 1;
            else if ((R>highThresh) && (G<lowThresh) && (B<lowThresh)) // red line
                colorVal = 2;
            else if ((R>highThresh) && (G<lowThresh) && (B<lowThresh)) // green line
                colorVal = 3;
            else
                colorVal = 0; // midpoint is still unknown, but we'll give up trying to find it
        }
        */
    }
#ifdef DEBUG
    printf("\t\tColor of point:(%d,%d,%d)\n",R,G,B);
#endif
    return colorVal;
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

void fixFocus(void) {
    system("uvcdynctrl -d /dev/video0 -s 'Focus, Auto' 0");
    system("uvcdynctrl -d /dev/video0 -s 'Focus (absolute)' 0");
    return;
}
