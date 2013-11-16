#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

using namespace cv;

CvCapture* capture; // open the default camera

IplImage* frame;        // Original Image  (Full Resolution)

int main(int argc, char *argv[]) {
  CvCapture* capture = cvCaptureFromCAM(0); // open the default camera
  cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 240);
  cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH,  320);
  
  IplImage* frame = cvQueryFrame(capture);  // Grab an initial image for analysis
  if(frame == NULL) {
      // For some reason the default camera could not be initialized
      printf("No image caputre device detected!!\n\n");
      exit(0);
  }

  printf("Processing a %dx%d image with %d channels\n",
                                  frame->width,frame->height,frame->nChannels); 

    while(cvWaitKey(30) == -1) {
        frame = cvQueryFrame(capture);
        cvShowImage( "Raspberryu Pi Camera!",  frame );
    }
    return 0;
}
