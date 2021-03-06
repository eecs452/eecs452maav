#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

#define DESIRED_WIDTH 256
#define PI 3.14159265

using namespace cv;

int blurDim = DESIRED_WIDTH/100;
int const maxBlurDim = 50;

int lowThreshold = 0;
int const maxLowThreshold = 200;

int minGapJump = 20;
int const maxGapJump = 50;

int houghThreshold = 100;
int const maxHoughThreshold = 300;

int edgeThresh = 1;
int ratio = 3;
int kernal_size = 3;
uchar colorThresh = 150; // checks color of line
unsigned int numLines;
unsigned int maxWhite;
unsigned int minWhite;
float rho, theta;
float* currentLine;
CvPoint* currentLineP;
CvPoint pt1, pt2, midpt, nearby;

CvCapture* capture; // open the default camera

IplImage* frame;
IplImage* img = 0; 
IplImage* img2 = 0;
IplImage* img3 = 0;
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
uchar *data;

CvSeq* lines;
CvMemStorage* lineStorage;
CvMemStorage* lineStorageP;
void blurHandler(int);
void cannyHandler(int);
void houghHandler(int);
void drawHoughLines(int);
void drawHoughLinesP(int);

int main(int argc, char *argv[]) {
  //CvCapture* capture = cvCaptureFromCAM(0); // open the default camera
  //IplImage* frame = cvQueryFrame(capture);

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
  lineStorageP = cvCreateMemStorage();
  imgRes = cvCreateImage(cvSize(DESIRED_WIDTH,height*DESIRED_WIDTH/width),d,1);
  imgResColor = cvCreateImage(cvSize(DESIRED_WIDTH,height*DESIRED_WIDTH/width),d,3);

  cvResize(imgR, imgRes);
  cvResize(img, imgResColor);
  CvSize s2 = cvGetSize(imgRes);
  imgTmp2 = cvCreateImage(s2, d ,1);
  imgE2 = cvCreateImage(s2, d ,1);

  //maxWhite = 4.0*DESIRED_WIDTH+35;
  //minWhite = 3.8*DESIRED_WIDTH+35;
  maxWhite = 5*s2.width;
  minWhite = 4*s2.width;
  
  
  printf("Resized image %dx%d image with %d channels\n",imgRes->width,imgRes->height,imgRes->nChannels); 


  // Create a grayscale version if we want one
  imgGray = cvCreateImage(s, d ,1);
  cvCvtColor(img, imgGray, CV_RGB2GRAY);

  // show the images
  int scaleRow = 330;
  int scaleCol = 420;
  int offsetRow = 30;
  int offsetCol = 80;
  int tmpRow = 0;
  int tmpCol = 0;

  tmpRow = 0; tmpCol = 0;
  cvNamedWindow( "Original Image", CV_WINDOW_NORMAL);
  cvMoveWindow(  "Original Image", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
  cvShowImage(   "Original Image", img );
  
  tmpRow = 1; tmpCol = 0;
  cvNamedWindow("Blue Channel", CV_WINDOW_NORMAL);
  cvMoveWindow( "Blue Channel", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
  cvShowImage(  "Blue Channel", imgB );

  tmpRow = 0; tmpCol = 1;
  cvNamedWindow( "Scaled Image", CV_WINDOW_NORMAL);
  cvMoveWindow(  "Scaled Image", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
  cvShowImage(   "Scaled Image", imgRes );

  tmpRow = 1; tmpCol = 1;
  cvNamedWindow("Blue Blured Image", CV_WINDOW_NORMAL);
  cvMoveWindow( "Blue Blured Image", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
  
  tmpRow = 0; tmpCol = 2;
  cvNamedWindow("Edges Detected", CV_WINDOW_NORMAL);
  cvMoveWindow( "Edges Detected", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);

  tmpRow = 2; tmpCol = 1;
  cvNamedWindow( "Final Image", CV_WINDOW_NORMAL);
  cvMoveWindow(  "Final Image", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);

  tmpRow = 2; tmpCol = 1;
  cvNamedWindow( "Final Image Color", CV_WINDOW_NORMAL);
  cvMoveWindow(  "Final Image Color", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);

  //cvShowImage("Gray Image", imgGray );
  //cvShowImage("Red Channel", imgR );
  //cvShowImage("Green Channel", imgG );

  //cvSmooth(imgB, imgB, CV_GAUSSIAN, 9, 0);
  //cvShowImage("Blue Blured Image", imgB );
  
  tmpRow = 1; tmpCol = 2;
  cvNamedWindow("Sliders", CV_WINDOW_NORMAL);
  cvMoveWindow( "Sliders", tmpCol*scaleCol+offsetCol, tmpRow*scaleRow+offsetRow);
  cvCreateTrackbar("Blur:", "Sliders",&blurDim,       maxBlurDim,       blurHandler);
  cvCreateTrackbar("Canny:","Sliders",&lowThreshold,  maxLowThreshold,  cannyHandler);
  cvCreateTrackbar("Hough:","Sliders",&houghThreshold,maxHoughThreshold,houghHandler);
   
  blurHandler(0);

  
  
    // lopp until a key is pressed
    while(cvWaitKey(1) == -1) {
        if(lowThreshold < 0) lowThreshold = 0;
        if(lowThreshold > maxLowThreshold) lowThreshold = maxLowThreshold;
        if(houghThreshold < 1) houghThreshold = 1;
        cvSetTrackbarPos("Canny:", "Sliders", lowThreshold);
        cvSetTrackbarPos("Hough:", "Sliders", houghThreshold);
        //frame = cvQueryFrame(capture);
        //if(frame) {
        //    cvShowImage("cap", frame);
        //}
        //cvReleaseImage(&frame);
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

    int i;
    int j;

    //cvCanny(imgTmp, imgE, lowThreshold, lowThreshold*ratio);
    cvCanny(imgTmp2, imgE2, lowThreshold, lowThreshold*ratio);
    cvShowImage("Edges Detected", imgE2 );

  // count the white
    unsigned int sum = 0;
    int step = imgE2->widthStep;
    data = (uchar *)imgE2->imageData;
    for(i=0;i<imgE2->height;i++) for(j=0;j<imgE2->width;j++) {
        if(data[i*step+j]) sum++;
    }
    //printf("\t\t\tThe image is %2.4f percent white!\n",sum*100.0/(imgE2->width*imgE2->height));
    //printf("\t\tNumber of total pixles = %d\n",imgE2->width*imgE2->height);
    //printf("\t\tmaxWhite = %d\n",maxWhite);
    //printf("\t\tminWhite = %d\n",minWhite);
    if(sum>maxWhite) {
        printf("\t\t\t%d is too many white pixles.  Adjust down\n",sum);
        lowThreshold++;
    }
    else if(sum<minWhite) {
        printf("\t\t\t%d is too few white pixles.  Adjust up\n",sum);
        lowThreshold--;
    }
    printf("\t\t\t%d white pixlesfound.\n",sum);

    houghHandler(0);
}
void houghHandler(int){
    //lines = cvHoughLines2(imgE2, lineStorage, CV_HOUGH_STANDARD, 1, CV_PI/180, houghThreshold+1);
    lines = cvHoughLines2(imgE2, lineStorageP, CV_HOUGH_PROBABILISTIC, 4, CV_PI/90, houghThreshold+1,20,20);
    //lines = cvHoughLines2(imgE2, lineStorage, CV_HOUGH_MULTI_SCALE, 1, CV_PI/180, houghThreshold+1);
    //drawHoughLines(0);
    drawHoughLinesP(0);
}
void drawHoughLinesP(int){
    int i;
    float a,b,x0,y0, theta;
    uchar R,G,B;
    img2 = cvCloneImage(imgResColor);
    img3 = cvCloneImage(imgResColor); 
    numLines = lines->total;
    if(numLines>10) {
        numLines = 10;
        houghThreshold++;
    }
    else if(numLines<8) {
        houghThreshold--;
    }
    for(i=0; i<numLines; i++) {
        currentLineP = (CvPoint*) cvGetSeqElem(lines, i);
        pt1 = currentLineP[0];
        pt2 = currentLineP[1];

	midpt.x = (pt1.x + pt2.x) >> 1;
	midpt.y = (pt1.y + pt2.y) >> 1;

	B = ((uchar*)(img2->imageData + img2->widthStep*midpt.y))[midpt.x*3];
	G = ((uchar*)(img2->imageData + img2->widthStep*midpt.y))[midpt.x*3+1];
	R = ((uchar*)(img2->imageData + img2->widthStep*midpt.y))[midpt.x*3+2];

	//printf("\t\tLine color = (%d,%d,%d)\n",R,G,B);
	
	if ((R > colorThresh) && (G < colorThresh) && (B < colorThresh)) { // red	
		cvLine(img2,pt1,pt2,cvScalar(0,0,255),1,CV_AA);
		printf("\t\tLine color = (%d,%d,%d), red\n",R,G,B);
	} else if ((R < colorThresh) && (G > colorThresh) && (B < colorThresh)) { // green
		cvLine(img2,pt1,pt2,cvScalar(0,255,0),1,CV_AA);
		printf("\t\tLine color = (%d,%d,%d), green\n",R,G,B);
	} else if ((R < colorThresh) && (G < colorThresh) && (B > colorThresh)) { // blue
		cvLine(img2,pt1,pt2,cvScalar(255,0,0),1,CV_AA);
		printf("\t\tLine color = (%d,%d,%d), blue\n",R,G,B);
	} else if ((R > colorThresh) && (G > colorThresh) && (B > colorThresh)) { // white
		cvLine(img2,pt1,pt2,cvScalar(255,255,255),1,CV_AA);
		printf("\t\tLine color = (%d,%d,%d), white\n",R,G,B);
	} else { 								     // unknown
		cvLine(img2,pt1,pt2,cvScalar(0,0,0),1,CV_AA);
		printf("\t\tLine color = (%d,%d,%d), unknown\n",R,G,B);
		printf("\t\t\tChecking nearby points:\n");
		
		theta = atan2(pt1.y-pt2.y, pt1.x-pt2.x);
		printf("\t\t\ttheta = %f\n",theta*180.0/PI);
		theta = theta + PI/2;
		nearby.x = cvRound( midpt.x + 1.5*cos(theta) );
		nearby.y = cvRound( midpt.y + 1.5*sin(theta) );

		printf("\t\t\tMidpt = (%d,%d), Nearby = (%d,%d)\n", midpt.x, midpt.y, nearby.x, nearby.y);

		B = ((uchar*)(img2->imageData + img2->widthStep*nearby.y))[nearby.x*3];
		G = ((uchar*)(img2->imageData + img2->widthStep*nearby.y))[nearby.x*3+1];
		R = ((uchar*)(img2->imageData + img2->widthStep*nearby.y))[nearby.x*3+2];

	
	
		if ((R > colorThresh) && (G < colorThresh) && (B < colorThresh)) { // red	
			cvLine(img2,pt1,pt2,cvScalar(0,0,255),1,CV_AA);
			printf("\t\t\tLine color = (%d,%d,%d), red\n",R,G,B);
		} else if ((R < colorThresh) && (G > colorThresh) && (B < colorThresh)) { // green
			cvLine(img2,pt1,pt2,cvScalar(0,255,0),1,CV_AA);
			printf("\t\t\tLine color = (%d,%d,%d), green\n",R,G,B);
		} else if ((R < colorThresh) && (G < colorThresh) && (B > colorThresh)) { // blue
			cvLine(img2,pt1,pt2,cvScalar(255,0,0),1,CV_AA);
			printf("\t\t\tLine color = (%d,%d,%d), blue\n",R,G,B);
		} else if ((R > colorThresh) && (G > colorThresh) && (B > colorThresh)) { // white
			cvLine(img2,pt1,pt2,cvScalar(255,255,255),1,CV_AA);
			printf("\t\t\tLine color = (%d,%d,%d), white\n",R,G,B);
		} else { 								     // unknown
			//cvLine(img2,pt1,pt2,cvScalar(0,0,0),1,CV_AA);
			//printf("\t\t\tLine color = (%d,%d,%d), unknown\n",R,G,B);
			theta = theta - PI;
			nearby.x = cvRound( midpt.x + 1.5*cos(theta) );
			nearby.y = cvRound( midpt.y + 1.5*sin(theta) );

			printf("\t\t\tMidpt = (%d,%d), Nearby = (%d,%d)\n", midpt.x, midpt.y, nearby.x, nearby.y);

			B = ((uchar*)(img2->imageData + img2->widthStep*nearby.y))[nearby.x*3];
			G = ((uchar*)(img2->imageData + img2->widthStep*nearby.y))[nearby.x*3+1];
			R = ((uchar*)(img2->imageData + img2->widthStep*nearby.y))[nearby.x*3+2];

	
	
			if ((R > colorThresh) && (G < colorThresh) && (B < colorThresh)) { // red	
				cvLine(img2,pt1,pt2,cvScalar(0,0,255),1,CV_AA);
				printf("\t\t\tLine color = (%d,%d,%d), red\n",R,G,B);
			} else if ((R < colorThresh) && (G > colorThresh) && (B < colorThresh)) { // green
				cvLine(img2,pt1,pt2,cvScalar(0,255,0),1,CV_AA);
				printf("\t\t\tLine color = (%d,%d,%d), green\n",R,G,B);
			} else if ((R < colorThresh) && (G < colorThresh) && (B > colorThresh)) { // blue
				cvLine(img2,pt1,pt2,cvScalar(255,0,0),1,CV_AA);
				printf("\t\t\tLine color = (%d,%d,%d), blue\n",R,G,B);
			} else if ((R > colorThresh) && (G > colorThresh) && (B > colorThresh)) { // white
				cvLine(img2,pt1,pt2,cvScalar(255,255,255),1,CV_AA);
				printf("\t\t\tLine color = (%d,%d,%d), white\n",R,G,B);
			} else { 								     // unknown
				cvLine(img2,pt1,pt2,cvScalar(0,0,0),1,CV_AA);
				printf("\t\t\tLine color = (%d,%d,%d), unknown\n",R,G,B);
			}

		}
	}

	//cvLine(img2,pt1,pt2,cvScalar(R,G,B),1,CV_AA);
	cvLine(img3,pt1,pt2,cvScalar(0,0,255),1,CV_AA);
    }
    printf("\t\t\tNumber of lines detected = %d\n",numLines);
    printf("\t\t\theader_size = %d\n",lines->header_size);
    printf("\t\t\telem_size = %d\n",sizeof(CvPoint)*2);

    cvShowImage("Final Image Color", img2 );
    cvReleaseImage(&img2 );
    cvShowImage("Final Image", img3 );
    cvReleaseImage(&img3 );
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
        //printf("\t\t\tRho=%f, Theta=%f\n",rho,theta);
        cvLine(img2,pt1,pt2,cvScalar(0,0,255),1,CV_AA);
    }
    printf("\t\t\tNumber of lines detected = %d\n",numLines);

    cvShowImage("Final Image", img2 );
    cvReleaseImage(&img2 );
}


