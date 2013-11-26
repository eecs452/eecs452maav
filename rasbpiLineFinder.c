#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <time.h>
#include <lcm/lcm.h>
#include "lcmtypes/image_lines_t.h"

#define DESIRED_WIDTH 256
#define IMAGE_PRINT
//#define AUTO_ADJUST_THRESH
#define COMM_TYPE LCM  // LCM, SERIAL, or NONE


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

int uart0_filestream;

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
lcm_t lcm*;

void houghHandler(int);
void drawHoughLinesP(int);
unsigned int countWhite(IplImage*);
int adjustWhiteThresh(int thresh, int whiteCount);

int main(int argc, char *argv[]) {
#if COMM_TYPE = LCM
    lcm = lcm_create("udpm://239.255.76.67:7667?ttl=1");
#elif COMM_TYPE = SERIAL
    unsigned char tx_buffer[] = "Hello World!!!!";
	uart0_filestream = -1;
    tx_buffer[13] = 10;
    tx_buffer[14] = 13;

    //Open in non blocking read/write mode
	uart0_filestream = open("/dev/ttyAMA0",
                        O_WRONLY | O_NOCTTY | O_NDELAY);
	if (uart0_filestream == -1) {
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}
	struct termios options;  
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);

	if (uart0_filestream != -1) {
		int count = write(uart0_filestream, tx_buffer, 15);
		if (count < 0) printf("UART TX error\n");
	}
#endif

    CvCapture* capture = cvCaptureFromCAM(-1); // open the default camera
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
    maxWhite = 0.035*(s.width*s.height);
    minWhite = 0.020*(s.width*s.height);

    // create memory for images with only one channel
    frameR =      cvCreateImage(s, d ,1);
    frameG =      cvCreateImage(s, d ,1);
    frameB =      cvCreateImage(s, d ,1);
    frameBlur =  cvCreateImage(s, d ,1);
    frameEdge =  cvCreateImage(s, d ,1);

    lineStorage = cvCreateMemStorage(0);

    while(cvGrabFrame(capture)) {
        frame = cvRetrieveFrame(capture,0);
        cvSplit(frame, frameB, frameG, frameR, 0);
        cvSmooth(frameB, frameBblur, CV_GAUSSIAN, blurDim*2+1, 0,0,0);
        cvCanny(frameBblur, frameBedge, edgeThreshB, edgeThreshB*ratio,3);
        houghHandler(0);
    }
    cvReleaseImage(&frame);
    cvReleaseImage(&frameR);
    cvReleaseImage(&frameG);
    cvReleaseImage(&frameB);
    cvReleaseImage(&frameBlur);
    cvReleaseImage(&frameEdge);
    cvReleaseImage(&frame);
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
void houghHandler(int foo){
    //lines = cvHoughLines2(frameBedge , lineStorage, CV_HOUGH_STANDARD,
    //        1, CV_PI/180, houghThreshold+1);
    lines = cvHoughLines2(frameBedge, lineStorage, CV_HOUGH_PROBABILISTIC,
            rhoRes, thetaRes, houghThreshold+1,minLineLength,minGapJump);
    //drawHoughLines(0);

    printf("\t\t\tNumber of lines detected = %d\n", lines->total);
    unsigned char newBuff[] = "this is a test!!";
    newBuff[14] = 10;
    newBuff[15] = 13;

	int count = write(uart0_filestream, newBuff, 16);
	if (count < 0) printf("UART TX error\n");
    //drawHoughLinesP(0);
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





