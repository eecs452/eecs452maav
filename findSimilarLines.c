#include <cv.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <highgui.h>

IplImage* frameEdge;   // Edge map
IplImage* frameEdgeC;  // Edge map with classic Hough
IplImage* frameEdgeP;  // Edge map with probablistic Hough
IplImage* frameTmp;
IplImage* frameColor;

CvPoint *currentLineP;
float   *currentLineC;
CvPoint pt1, pt2;
CvSeq *linesC;
CvSeq *linesP;
CvMemStorage* lineStorageC;
CvMemStorage* lineStorageP;

int rhoRes = 4;
float thetaRes = CV_PI/45;

int minLineLength = 100;
int const maxLineLength = 300;

int minGapJump = 20;
int const maxGapJump = 50;

int houghThreshold = 100;
int const maxHoughThreshold = 300;


void drawHoughLinesP(int);
void drawHoughLinesC(int);
static int compareTheta( const void* _a, const void* _b, void* userdata );
void onTrackbarSlide(void) {;}
int main(int argc, char *argv[]) {
    
    
    if(argc<2){
        printf("Usage: main <image-file-name>\n\7");
        exit(0);
    }
    
    frameEdge  = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    frameColor = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
    
    if(!frameEdge){
        printf("Could not load image file: %s\n",argv[1]);
        exit(0);
    }
    
    lineStorageC = cvCreateMemStorage(0);
    lineStorageP = cvCreateMemStorage(0);

    // show the images
    int scaleRow = 330;
    int scaleCol = 420;
    int offsetRow = 30;
    int offsetCol = 80;
    int tmpRow = 0;
    int tmpCol = 0;

    // Open windows for images
    tmpRow = 0; tmpCol = 0;
    cvNamedWindow("Edges Detected", CV_WINDOW_NORMAL);
    cvMoveWindow( "Edges Detected", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
    cvShowImage(  "Edges Detected", frameEdge);
    tmpRow = 0; tmpCol = 1;
    cvNamedWindow("Classic Hough", CV_WINDOW_NORMAL);
    cvMoveWindow( "Classic Hough", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
    tmpRow = 0; tmpCol = 2;
    cvNamedWindow("Probablistic Hough", CV_WINDOW_NORMAL);
    cvMoveWindow( "Probablistic Hough", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);


    cvCreateTrackbar("Hough:","Probablistic Hough",
                    &houghThreshold,maxHoughThreshold,drawHoughLinesP);
    cvCreateTrackbar("Min Line Lenght:","Probablistic Hough",
                    &minLineLength,maxLineLength,drawHoughLinesP);
    cvCreateTrackbar("Min Gap Jump:","Probablistic Hough",
                    &minGapJump,maxGapJump,drawHoughLinesP);
    printf("\n\n");

    drawHoughLinesP(0);
    drawHoughLinesC(0);

    int i;

    //for(i=0; i<10; i++) {
    //    pt1.
    //    cvSeqPush(lines,&i);
    //}

    //printf("\t\tNumber of classic lines detected = %d\n",linesC->total);
    
    /*
    for(i=0; i<10; i++) {
        currentLine = (CvPoint*) cvGetSeqElem(lines, i);
        pt1 = currentLine[0];
        pt2 = currentLine[1];
        
        //printf("",currentLine[0].
    }
    */

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
void drawHoughLinesC(int z){
    linesC = cvHoughLines2(frameEdge, lineStorageC, CV_HOUGH_STANDARD,
           rhoRes, thetaRes, houghThreshold+1,20,20);
    cvSeqSort(linesC, compareTheta, 0);

    int i;
    float a,b,x0,y0,rho,theta;
    frameTmp = cvCloneImage(frameColor);
    int numLines = linesC->total;
    printf("\t\tNumber of classic lines detected    = %d\n",numLines);
    for(i=0; i<numLines; i++) {
        currentLineC = (float*) cvGetSeqElem(linesC, i);
        rho   = (float)currentLineC[0];
        theta = (float)currentLineC[1];
        a = cos(theta);
        b = sin(theta);
        y0 = b*rho;
        x0 = a*rho;
        pt1.x = cvRound(x0 - 1000.0*b);
        pt1.y = cvRound(y0 + 1000.0*a);
        pt2.x = cvRound(x0 + 1000.0*b);
        pt2.y = cvRound(y0 - 1000.0*a);
        cvLine(frameTmp,pt1,pt2,cvScalar(0,0,255,0),0,CV_AA,0);

        //printf("\t\trho =%5.0f,theta =%7.4f\n",rho,theta);
        //cvShowImage("Classic Hough", frameTmp );
        //cvWaitKey(500);
    }
    /*for(i=0; i<numLines; i++) {
        currentLineP = (CvPoint*) cvGetSeqElem(linesP, i);
        pt1 = currentLineP[0];
        pt2 = currentLineP[1];

        cvLine(frameTmp,pt1,pt2,cvScalar(0,0,255,0),0,CV_AA,0);
    }*/

    cvShowImage("Classic Hough", frameTmp );
    cvReleaseImage(&frameTmp);
    
    return;
}
void drawHoughLinesP(int z){
    linesP = cvHoughLines2(frameEdge, lineStorageP, CV_HOUGH_PROBABILISTIC,
           rhoRes, thetaRes, houghThreshold+1, minLineLength, minGapJump);
    int i;
    //float a,b,x0,y0;
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

        //printf("\t\t(%d,%d),(%d,%d)\n",pt1.x,pt1.y,pt2.x,pt2.y);
        //cvShowImage("Probablistic Hough", frameTmp );
        //cvWaitKey(500);
    }

    cvShowImage("Probablistic Hough", frameTmp );
    cvReleaseImage(&frameTmp);
    
    return;
}

