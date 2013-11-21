#include <cv.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <highgui.h>

#define DESIRED_WIDTH 176
#define DESIRED_HEIGHT 144

IplImage* frame;        // Original Image  (Full Resolution)
IplImage* frameScale;   // Original Image  (scaled resolution)
IplImage* frameC;       // One Coponent    (scaled)
IplImage* frameCblur;   // Componenet Blured image
IplImage* frameEdge;    // Edge map
IplImage* frameTmp;

CvPoint *currentLine;
CvPoint pt1, pt2;
CvSeq *lines;
CvMemStorage* lineStorage;

int rhoRes = 4;
float thetaRes = CV_PI/45;

int minLineLength = 20;
int const maxLineLength = 300;

int minGapJump = 20;
int const maxGapJump = 50;

int houghThreshold = 100;
int const maxHoughThreshold = 300;

int i;

void drawHoughLinesP(int);
int main(int argc, char *argv[]) {
    if(argc<2){
        printf("Usage: main <image-file-name> \n\7");
        exit(0);
    }
    frame = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
    if(!frame){
        printf("Could not load image file: %s\n",argv[1]);
        exit(0);
    }
    
    lineStorage = cvCreateMemStorage(0);

    CvSize s = cvSize(DESIRED_WIDTH,DESIRED_HEIGHT);
    int    d = frame->depth;
    imgColor = cvCreateImage(s,d,3);
    cvResize(frame, frameColor);
    

    cvSmooth(frameB, imgTmp2, CV_GAUSSIAN, blurDim*2+1, 0);




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
    cvMoveWindow( "Original Image", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
    cvShowImage(  "Original Image", frameColor);
    tmpRow = 0; tmpCol = 1;
    cvNamedWindow("Edges Detected", CV_WINDOW_NORMAL);
    cvMoveWindow( "Edges Detected", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
    cvShowImage(  "Edges Detected", frameEdge);
    tmpRow = 0; tmpCol = 2;
    cvNamedWindow("Probablistic Hough", CV_WINDOW_NORMAL);
    cvMoveWindow( "Probablistic Hough", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);


    printf("\n\n");

    drawHoughLinesP(0);


    cvWaitKey(0);
    return 0;
}
static int compareTheta( const void* _a, const void* _b, void* userdata ) {
    float* a = (float*)_a;
    float* b = (float*)_b;
    //printf("\t\tcomparing %5.2f with %5.2f\n",a[1], b[1]);
    if(a[0] < b[0]) return -1;
    if(a[0] > b[0]) return  1;
    return 0;
}
void drawHoughLinesP(int z){
    linesP = cvHoughLines2(frameEdge, lineStorageP, CV_HOUGH_PROBABILISTIC,
           rhoRes, thetaRes, houghThreshold+1, minLineLength, minGapJump);
    int i;
    frameTmp = cvCloneImage(frameColor);
    int numLines = linesP->total;
    printf("\t\tNumber of probablistic lines detected = %d\n",numLines);

    for(i=0; i<numLines; i++) {
        currentLineP = (CvPoint*) cvGetSeqElem(linesP, i);
        pt1 = currentLineP[0];
        pt2 = currentLineP[1];

        int a = pt1.x-pt2.x;
        int b = pt1.y-pt2.y;
        if(0) { // if you want the lines to be exteded
            pt1.x += 100*a;
            pt1.y += 100*b;
            pt2.x -= 100*a;
            pt2.y -= 100*b;
        }
        cvLine(frameTmp,pt1,pt2,cvScalar(0,0,255,0),0,CV_AA,0);
    }
    cvShowImage("Probablistic Hough", frameTmp );
    cvReleaseImage(&frameTmp);
    return;
}

