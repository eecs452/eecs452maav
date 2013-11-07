#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

#define DESIRED_WIDTH 256


using namespace cv;

int edgeThresh = 1;
int lowThreshold = 20;
int houghThreshold = 50;
int blurDim = 10;
int const maxLowThreshold = 100;
int const maxBlurDim = 50;
int const maxHoughThreshold = 100;
int ratio = 3;
int kernal_size = 3;
unsigned int numLines;
float rho, theta;
float* currentLine;
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
void blurHandler(int);
void cannyHandler(int);
void houghHandler(int);
void drawHoughLines(int);

int main(int argc, char *argv[]) {
  CvCapture* capture = cvCaptureFromCAM(0); // open the default camera
  IplImage* frame = cvQueryFrame(capture);  // Grab an initial image for analysis
  if(frame == NULL) {
      // For some reason the default camera could not be initialized
      printf("No image caputre device detected!!\n\n");
      exit(0);
  }

  int height,width,step,channels;
  uchar *data;
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

  // get the image data
  //height    = frame->height;
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

  // create memory for images with only one channel
  //CvSize s = cvGetSize(img);
  //int d = img->depth;
  //imgR = cvCreateImage(s, d ,1);
  //imgG = cvCreateImage(s, d ,1);
  //imgB = cvCreateImage(s, d ,1);
  //imgE = cvCreateImage(s, d ,1);
  //imgTmp = cvCreateImage(s, d ,1);
  //cvSplit(img, imgB, imgG, imgR, 0);
  lineStorage = cvCreateMemStorage();
  //imgRes = cvCreateImage(cvSize(DESIRED_WIDTH,height*DESIRED_WIDTH/width),d,1);
  //imgResColor = cvCreateImage(cvSize(DESIRED_WIDTH,height*DESIRED_WIDTH/width),d,3);

  //cvResize(imgR, imgRes);
  //cvResize(img, imgResColor);
  //CvSize s2 = cvGetSize(imgRes);
  //imgTmp2 = cvCreateImage(s2, d ,1);
  //imgE2 = cvCreateImage(s2, d ,1);
  
  
  //printf("Resized image %dx%d image with %d channels\n",imgRes->width,imgRes->height,imgRes->nChannels); 


  // Create a grayscale version if we want one
  //imgGray = cvCreateImage(s, d ,1);
  //cvCvtColor(img, imgGray, CV_RGB2GRAY);

  // Open windows for images
  cvNamedWindow("Original Image",   CV_WINDOW_NORMAL);
  cvMoveWindow( "Original Image",   0, 0);
  cvNamedWindow("Scaled Image",     CV_WINDOW_NORMAL);
  cvMoveWindow( "Scaled Image",     0, 360);
  cvNamedWindow("Final Image",      CV_WINDOW_NORMAL);
  cvMoveWindow( "Final Image",      0, 700);

  cvNamedWindow("Red Channel",          CV_WINDOW_NORMAL);
  cvMoveWindow( "Red Channel",          470, 0);
  cvNamedWindow("Green Channel",        CV_WINDOW_NORMAL);
  cvMoveWindow( "Green Channel",        470, 360);
  cvNamedWindow("Blue Channel",         CV_WINDOW_NORMAL);
  cvMoveWindow( "Blue Channel",         470, 700);

  cvNamedWindow("Red Blured Image",     CV_WINDOW_NORMAL);
  cvMoveWindow( "Red Blured Image",     880, 0);
  cvNamedWindow("Green Blured Image",   CV_WINDOW_NORMAL);
  cvMoveWindow( "Green Blured Image",   880, 360);
  cvNamedWindow("Blue Blured Image",    CV_WINDOW_NORMAL);
  cvMoveWindow( "Blue Blured Image",    880, 700);
  
  cvNamedWindow("Red Edges Detected",   CV_WINDOW_NORMAL);
  cvMoveWindow( "Red Edges Detected",   1290, 0);
  cvNamedWindow("Green Edges Detected", CV_WINDOW_NORMAL);
  cvMoveWindow( "Green Edges Detected", 1290, 360);
  cvNamedWindow("Blue Edges Detected",  CV_WINDOW_NORMAL);
  cvMoveWindow( "Blue Edges Detected",  1290, 700);


  //cvShowImage("Gray Image", imgGray );
  //cvShowImage("Red Channel", imgR );
  //cvShowImage("Green Channel", imgG );

  //cvSmooth(imgB, imgB, CV_GAUSSIAN, 9, 0);
  //cvShowImage("Blue Blured Image", imgB );
  
  cvNamedWindow("Sliders", CV_WINDOW_NORMAL);
  cvMoveWindow( "Sliders", 1500, 0);
  cvCreateTrackbar("Blur:", "Sliders",&blurDim,       maxBlurDim,       blurHandler);
  cvCreateTrackbar("Canny:","Sliders",&lowThreshold,  maxLowThreshold,  blurHandler);
  cvCreateTrackbar("Hough:","Sliders",&houghThreshold,maxHoughThreshold,blurHandler);
   
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

            cvCanny(frameRblur, frameRedge, lowThreshold, lowThreshold*ratio);
            cvCanny(frameGblur, frameGedge, lowThreshold, lowThreshold*ratio);
            cvCanny(frameBblur, frameBedge, lowThreshold, lowThreshold*ratio);


            houghHandler(0);

            if(1){
            cvShowImage("Original Image",   frame);
            cvShowImage("Scaled Image",     frameScale);
            
            cvShowImage("Red Channel",      frameR );
            cvShowImage("Green Channel",    frameG );
            cvShowImage("Blue Channel",     frameB );
            
            cvShowImage("Red Blured Image",     frameRblur );
            cvShowImage("Green Blured Image",   frameGblur );
            cvShowImage("Blue Blured Image",    frameBblur );

            cvShowImage("Red Edges Detected",   frameRedge );
            cvShowImage("Green Edges Detected", frameGedge );
            cvShowImage("Blue Edges Detected",  frameBedge );
            }


            // create memory for images with only one channel
            //imgTmp = cvCreateImage(s, d ,1);
            //cvSplit(img, imgB, imgG, imgR, 0);
            //lineStorage = cvCreateMemStorage();
            //imgRes = cvCreateImage(cvSize(DESIRED_WIDTH,height*DESIRED_WIDTH/width),d,1);
            //imgResColor = cvCreateImage(cvSize(DESIRED_WIDTH,height*DESIRED_WIDTH/width),d,3);

  //cvResize(imgR, imgRes);
  //cvResize(img, imgResColor);
  //CvSize s2 = cvGetSize(imgRes);
  //imgTmp2 = cvCreateImage(s2, d ,1);
  //imgE2 = cvCreateImage(s2, d ,1);
  
  
  //printf("Resized image %dx%d image with %d channels\n",imgRes->width,imgRes->height,imgRes->nChannels); 

        }
    }
    // release the image
    cvReleaseImage(&img );
    return 0;
}
void blurHandler(int) {
    //cvSmooth(imgB, imgTmp, CV_GAUSSIAN, blurDim*2+1, 0);
    //cvSmooth(imgRes, imgTmp2, CV_GAUSSIAN, blurDim*2+1, 0);
    //cvShowImage("Blue Blured Image", imgTmp2 );
    //cannyHandler(0);
}
void cannyHandler(int) {
    //cvCanny(imgTmp, imgE, lowThreshold, lowThreshold*ratio);
    cvCanny(imgTmp2, imgE2, lowThreshold, lowThreshold*ratio);
    cvShowImage("Edges Detected", imgE2 );
    houghHandler(0);
}
void houghHandler(int){
    lines = cvHoughLines2(frameBedge ,
            lineStorage, CV_HOUGH_STANDARD, 1, CV_PI/180, houghThreshold+1);
    drawHoughLines(0);
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





