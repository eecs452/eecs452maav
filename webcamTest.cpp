#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

#define DESIRED_WIDTH 128


using namespace cv;

int blurDim = 5;
int const maxBlurDim = 50;

int lowThreshold = 20;
int edgeThreshR = 20;
int edgeThreshG = 20;
int edgeThreshB = 20;

int const maxEdgeThresh = 100;

int houghThreshold = 50;
int const maxHoughThreshold = 100;

int edgeThresh = 1;
int ratio = 3;
int kernal_size = 3;
unsigned int numLines;
unsigned int maxWhite;
unsigned int minWhite;
float rho, theta;
float* currentLine;
CvPoint* currentLineP;
CvPoint pt1, pt2;
char* window_name = "Edge map";



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

IplImage* img = 0; 
IplImage* img2 = 0;
IplImage* imgGray = 0;
IplImage* imgR = 0;
IplImage* imgG = 0;
IplImage* imgB = 0;
IplImage* imgE = 0;
IplImage* imgE2 = 0;
IplImage* imgTmp = 0;
IplImage* imgTmp2 = 0;
IplImage* imgRes = 0;
IplImage* imgResColor = 0;

CvSeq* lines;
CvMemStorage* lineStorage;
CvMemStorage* lineStorageP;
uchar *data;

void onTrackbarSlide(int){;}
void blurHandler(int);
void cannyHandler(int);
void houghHandler(int);
void drawHoughLines(int);
void drawHoughLinesP(int);
unsigned int countWhite(IplImage*);
int adjustWhiteThresh(int thresh, int whiteCount);

int main(int argc, char *argv[]) {
  CvCapture* capture = cvCaptureFromCAM(0); // open the default camera
  IplImage* frame = cvQueryFrame(capture);  // Grab an initial image for analysis
  if(frame == NULL) {
      // For some reason the default camera could not be initialized
      printf("No image caputre device detected!!\n\n");
      exit(0);
  }

  int height,width,step,channels;
  int i,j,k;

  if(argc<2){
    printf("Usage: main <image-file-name>\n\7");
    exit(0);
  }

  // load an image  
  img=cvLoadImage(argv[1]);
  if(!img){
    printf("Could not load image file: %s\n",argv[1]);
    exit(0);
  }

  //width     = frame->width;
  //step      = frame->widthStep;
  //channels  = frame->nChannels;
  //data      = (uchar *)frame->imageData;
  printf("Processing a %dx%d image with %d channels\n",
                                  frame->width,frame->height,frame->nChannels); 

  CvSize s = cvGetSize(frame);
  int d = frame->depth;
  frameScale = cvCreateImage(cvSize(DESIRED_WIDTH,frame->height*DESIRED_WIDTH/frame->width),d,3);
  cvResize(frame, frameScale);
  s = cvGetSize(frameScale);
  d = frameScale->depth;

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
  frameTmp   =  cvCreateImage(s, d ,1);

  lineStorage = cvCreateMemStorage();

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
  tmpRow = 1; tmpCol = 0;
  cvNamedWindow("Scaled Image",   CV_WINDOW_NORMAL);
  cvMoveWindow(  "Scaled Image",  tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
  tmpRow = 2; tmpCol = 0;
  cvNamedWindow("Final Image",    CV_WINDOW_NORMAL);
  cvMoveWindow( "Final Image",    tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);

  tmpRow = 0; tmpCol = 1;
  cvNamedWindow("Red Channel",    CV_WINDOW_NORMAL);
  cvMoveWindow( "Red Channel",    tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
  tmpRow = 1; tmpCol = 1;
  cvNamedWindow("Green Channel",  CV_WINDOW_NORMAL);
  cvMoveWindow( "Green Channel",  tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
  tmpRow = 2; tmpCol = 1;
  cvNamedWindow("Blue Channel",   CV_WINDOW_NORMAL);
  cvMoveWindow( "Blue Channel",   tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);

  tmpRow = 0; tmpCol = 2;
  cvNamedWindow("Red Blured Image",  CV_WINDOW_NORMAL);
  cvMoveWindow( "Red Blured Image",  tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
  tmpRow = 1; tmpCol = 2;
  cvNamedWindow("Green Blured Image",CV_WINDOW_NORMAL);
  cvMoveWindow( "Green Blured Image",tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
  tmpRow = 2; tmpCol = 2;
  cvNamedWindow("Blue Blured Image", CV_WINDOW_NORMAL);
  cvMoveWindow( "Blue Blured Image", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
  
  tmpRow = 0; tmpCol = 3;
  cvNamedWindow("Red Edges Detected",  CV_WINDOW_NORMAL);
  cvMoveWindow( "Red Edges Detected",  tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
  tmpRow = 1; tmpCol = 3;
  cvNamedWindow("Green Edges Detected",CV_WINDOW_NORMAL);
  cvMoveWindow( "Green Edges Detected",tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
  tmpRow = 2; tmpCol = 3;
  cvNamedWindow("Blue Edges Detected", CV_WINDOW_NORMAL);
  cvMoveWindow( "Blue Edges Detected", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);

  //cvNamedWindow("Sliders", CV_WINDOW_NORMAL);
  //cvMoveWindow( "Sliders", 1500, 0);
  cvCreateTrackbar("Blur:", "Final Image",&blurDim,       maxBlurDim,       onTrackbarSlide);
  cvCreateTrackbar("Hough:","Final Image",&houghThreshold,maxHoughThreshold,onTrackbarSlide);
   
  cvCreateTrackbar("Thresh:",  "Red Edges Detected",&edgeThreshR,maxEdgeThresh,onTrackbarSlide);
  cvCreateTrackbar("Thresh:","Green Edges Detected",&edgeThreshG,maxEdgeThresh,onTrackbarSlide);
  cvCreateTrackbar("Thresh:", "Blue Edges Detected",&edgeThreshB,maxEdgeThresh,onTrackbarSlide);
  //blurHandler(0);

  
  
    // lopp until a key is pressed
    while(cvWaitKey(1) == -1) {
        frame = cvQueryFrame(capture);
        if(frame) {

            cvResize(frame, frameScale);
            cvSplit(frameScale, frameB, frameG, frameR, 0);
            cvSmooth(frameR, frameRblur, CV_GAUSSIAN, blurDim*2+1, 0);
            cvSmooth(frameG, frameGblur, CV_GAUSSIAN, blurDim*2+1, 0);
            cvSmooth(frameB, frameBblur, CV_GAUSSIAN, blurDim*2+1, 0);

            cvCanny(frameRblur, frameRedge, edgeThreshR, lowThreshold*ratio);
            cvCanny(frameGblur, frameGedge, edgeThreshG, lowThreshold*ratio);
            cvCanny(frameBblur, frameBedge, edgeThreshB, lowThreshold*ratio);

            houghHandler(0);


            if(1){ // print the images in real time (if you like)
            cvShowImage("Original Image",   frame);
            cvShowImage("Scaled Image",     frameScale);
            
            cvShowImage("Red Channel",      frameR );
            cvShowImage("Green Channel",    frameG );
            cvShowImage("Blue Channel",     frameB );
            
            cvShowImage("Red Blured Image",     frameRblur );
            cvShowImage("Green Blured Image",   frameGblur );
            cvShowImage("Blue Blured Image",    frameBblur );

            cvShowImage(  "Red Edges Detected",   frameRedge );
            cvShowImage("Green Edges Detected", frameGedge );
            cvShowImage( "Blue Edges Detected",  frameBedge );
            }

            if(1) {
            // count the white
            unsigned int whiteR = countWhite(frameRedge);
            unsigned int whiteG = countWhite(frameGedge);
            unsigned int whiteB = countWhite(frameBedge);

            edgeThreshR = adjustWhiteThresh(edgeThreshR, whiteR);
            edgeThreshG = adjustWhiteThresh(edgeThreshG, whiteG);
            edgeThreshB = adjustWhiteThresh(edgeThreshB, whiteB);
            if(houghThreshold<1) houghThreshold = 1;
            else if(houghThreshold>maxHoughThreshold)
                houghThreshold = maxHoughThreshold;

            cvSetTrackbarPos("Thresh:","Red Edges Detected",  edgeThreshR);
            cvSetTrackbarPos("Thresh:","Green Edges Detected",edgeThreshG);
            cvSetTrackbarPos("Thresh:","Blue Edges Detected", edgeThreshB);
            cvSetTrackbarPos("Hough:", "Final Image",         houghThreshold);
            }
        }
    }
    // release the image
    cvReleaseImage(&img );
    return 0;
}
int adjustWhiteThresh(int thresh, int whiteCount) {
    if(     whiteCount>maxWhite) thresh++;
    else if(whiteCount<minWhite) thresh--;
    
    if(     thresh < 0)             thresh = 0;
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

void blurHandler(int) {
    //cvSmooth(imgB, imgTmp, CV_GAUSSIAN, blurDim*2+1, 0);
    //cvSmooth(imgRes, imgTmp2, CV_GAUSSIAN, blurDim*2+1, 0);
    //cvShowImage("Blue Blured Image", imgTmp2 );
    //cannyHandler(0);
}
void cannyHandler(int) {
    //cvCanny(imgTmp, imgE, lowThreshold, lowThreshold*ratio);
    //cvCanny(imgTmp2, imgE2, lowThreshold, lowThreshold*ratio);
    //cvShowImage("Edges Detected", imgE2 );
    //houghHandler(0);
}
void houghHandler(int){
    //lines = cvHoughLines2(frameBedge , lineStorage, CV_HOUGH_STANDARD,
    //        1, CV_PI/180, houghThreshold+1);
    lines = cvHoughLines2(frameBedge, lineStorage, CV_HOUGH_PROBABILISTIC,
            1, CV_PI/180, houghThreshold+1,20,20);
    //drawHoughLines(0);
    drawHoughLinesP(0);
}
void drawHoughLinesP(int){
    int i;
    float a,b,x0,y0;
    frameTmp = cvCloneImage(frameScale);
    numLines = lines->total;
    if(numLines>10) {
        numLines = 10;
        houghThreshold++;
    }
    else if(numLines<8) {
        houghThreshold--;
    }
    houghThreshold?houghThreshold<1:1;
    for(i=0; i<numLines; i++) {
        currentLineP = (CvPoint*) cvGetSeqElem(lines, i);
        pt1 = currentLineP[0];
        pt2 = currentLineP[1];

        cvLine(frameTmp,pt1,pt2,cvScalar(0,0,255),1,CV_AA);
    }
    printf("\t\t\tNumber of lines detected = %d\n",numLines);

    cvShowImage("Final Image", frameTmp );
    cvReleaseImage(&frameTmp);
}

void drawHoughLines(int){
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
        cvLine(frameTmp,pt1,pt2,cvScalar(0,0,255),1,CV_AA);
    }
    //printf("Number of lines detected = %d\n",numLines);

    cvShowImage("Final Image", frameTmp );
    cvReleaseImage(&frameTmp );
}





