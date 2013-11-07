#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

#define DESIRED_WIDTH 256


using namespace cv;

int edgeThresh = 1;
int lowThreshold = 35;
int houghThreshold = 80;
int blurDim = 3;
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
IplImage* frame;
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
  IplImage* frame = cvQueryFrame(capture);
  if(frame == NULL)
      return -1;
  cvShowImage("cap", frame);

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
  height    = img->height;
  width     = img->width;
  step      = img->widthStep;
  channels  = img->nChannels;
  data      = (uchar *)img->imageData;
  printf("Processing a %dx%d image with %d channels\n",width,height,channels); 

  // invert the image
  //for(i=0;i<height;i++) for(j=0;j<width;j++) for(k=0;k<channels;k++)
  //  data[i*step+j*channels+k]=255-data[i*step+j*channels+k];


  // Put a line down the middle just for fun
  /*for(i=0; i<height; i++) {
      data[i*step+width*channels/2+0] = 0;
      data[i*step+width*channels/2+1] = 0;
      data[i*step+width*channels/2+2] = 0;
  }*/

  // create memory for images with only one channel
  CvSize s = cvGetSize(img);
  int d = img->depth;
  imgR = cvCreateImage(s, d ,1);
  imgG = cvCreateImage(s, d ,1);
  imgB = cvCreateImage(s, d ,1);
  imgE = cvCreateImage(s, d ,1);
  imgTmp = cvCreateImage(s, d ,1);
  cvSplit(img, imgB, imgG, imgR, 0);
  lineStorage = cvCreateMemStorage();
  imgRes = cvCreateImage(cvSize(DESIRED_WIDTH,height*DESIRED_WIDTH/width),d,1);
  imgResColor = cvCreateImage(cvSize(DESIRED_WIDTH,height*DESIRED_WIDTH/width),d,3);

  cvResize(imgR, imgRes);
  cvResize(img, imgResColor);
  CvSize s2 = cvGetSize(imgRes);
  imgTmp2 = cvCreateImage(s2, d ,1);
  imgE2 = cvCreateImage(s2, d ,1);
  
  
  printf("Resized image %dx%d image with %d channels\n",imgRes->width,imgRes->height,imgRes->nChannels); 


  // Create a grayscale version if we want one
  imgGray = cvCreateImage(s, d ,1);
  cvCvtColor(img, imgGray, CV_RGB2GRAY);

  // show the images
  cvNamedWindow( "Original Image", CV_WINDOW_NORMAL);
  cvMoveWindow(  "Original Image", 0, 0);
  cvShowImage(   "Original Image", img );

  cvNamedWindow( "Scaled Image", CV_WINDOW_NORMAL);
  cvMoveWindow(  "Scaled Image", 0, 375);
  cvShowImage(   "Scaled Image", imgRes );
  //TODO  Use this image in the algorithm instead of the full size one

  cvNamedWindow("Blue Channel", CV_WINDOW_NORMAL);
  cvMoveWindow( "Blue Channel", 0, 725);
  cvShowImage(  "Blue Channel", imgB );

  cvNamedWindow("Blue Blured Image", CV_WINDOW_NORMAL);
  cvMoveWindow( "Blue Blured Image", 700, 0);
  
  cvNamedWindow("Edges Detected", CV_WINDOW_NORMAL);
  cvMoveWindow( "Edges Detected", 700, 500);

  cvNamedWindow("Final Image", CV_WINDOW_NORMAL);
  cvMoveWindow( "Final Image", 1500, 500);

  //cvShowImage("Gray Image", imgGray );
  //cvShowImage("Red Channel", imgR );
  //cvShowImage("Green Channel", imgG );

  //cvSmooth(imgB, imgB, CV_GAUSSIAN, 9, 0);
  //cvShowImage("Blue Blured Image", imgB );
  
  cvNamedWindow("Sliders", CV_WINDOW_NORMAL);
  cvMoveWindow( "Sliders", 1500, 0);
  cvCreateTrackbar("Blur:", "Sliders",&blurDim,       maxBlurDim,       blurHandler);
  cvCreateTrackbar("Canny:","Sliders",&lowThreshold,  maxLowThreshold,  cannyHandler);
  cvCreateTrackbar("Hough:","Sliders",&houghThreshold,maxHoughThreshold,houghHandler);
   
  blurHandler(0);

  
  
    // lopp until a key is pressed
    while(cvWaitKey(30) == -1) {
        
    }
    // release the image
    cvReleaseImage(&img );
    return 0;
}
void blurHandler(int) {
    //cvSmooth(imgB, imgTmp, CV_GAUSSIAN, blurDim*2+1, 0);
    cvSmooth(imgRes, imgTmp2, CV_GAUSSIAN, blurDim*2+1, 0);
    cvShowImage("Blue Blured Image", imgTmp2 );
    cannyHandler(0);
}
void cannyHandler(int) {
    //cvCanny(imgTmp, imgE, lowThreshold, lowThreshold*ratio);
    cvCanny(imgTmp2, imgE2, lowThreshold, lowThreshold*ratio);
    cvShowImage("Edges Detected", imgE2 );
    houghHandler(0);
}
void houghHandler(int){
    lines = cvHoughLines2(imgE2, lineStorage, CV_HOUGH_STANDARD, 1, CV_PI/180, houghThreshold+1);
    drawHoughLines(0);
}
void drawHoughLines(int){
    int i;
    float a,b,x0,y0;
    img2 = cvCloneImage(imgResColor);
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
        cvLine(img2,pt1,pt2,cvScalar(0,0,255),1,CV_AA);
    }
    printf("Number of lines detected = %d\n",numLines);

    cvShowImage("Final Image", img2 );
    cvReleaseImage(&img2 );
}





