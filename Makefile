LCM_TYPES = lcmtypes/point_t.c lcmtypes/line_t.c lcmtypes/circle_t.c lcmtypes/image_lines_t.c
cam:
	gcc -pg -I/usr/local/include/opencv webcamTest.cpp -lopencv_core -lopencv_legacy -lopencv_imgproc -lopencv_photo -lopencv_highgui
rasbcam:
	gcc -pg -I/usr/local/include/opencv rasbpiCamTest.cpp -lopencv_core -lopencv_legacy -lopencv_imgproc -lopencv_photo -lopencv_highgui
rasbpiLine:
	gcc -I/usr/local/include/opencv rasbpiLineFinder.c -lopencv_core -lopencv_legacy -lopencv_imgproc -lopencv_photo -lopencv_highgui
still:
	gcc -pg -I/usr/local/include/opencv hello-world.cpp -lopencv_core -lopencv_legacy -lopencv_imgproc -lopencv_photo -lopencv_highgui
demo:
	gcc -pg -I/usr/local/include/opencv -I/homeleonard/lcm rasbpiLineFinderLCM.c $(LCM_TYPES) common/timestamp.c -llcm -lopencv_core -lopencv_legacy -lopencv_imgproc -lopencv_photo -lopencv_highgui
slave:
	gcc -pg -I/usr/local/include/opencv -I/homeleonard/lcm lcmSlaveTest.c $(LCM_TYPES) common/timestamp.c -llcm -lopencv_core -lopencv_legacy -lopencv_imgproc -lopencv_photo -lopencv_highgui
lines:
	gcc -pg -I/usr/local/include/opencv findSimilarLines.c -lopencv_core -lopencv_legacy -lopencv_imgproc -lopencv_photo -lopencv_highgui
video:
	gcc -pg -I/usr/local/include/opencv videoTest.cpp -lopencv_core -lopencv_legacy -lopencv_imgproc -lopencv_photo -lopencv_highgui
circle:
	gcc -pg -I/usr/local/include/opencv circleTest.cpp -lopencv_core -lopencv_legacy -lopencv_imgproc -lopencv_photo -lopencv_highgui
circlebasic:
	gcc -pg -I/usr/local/include/opencv houghcircles_scrollbars.cpp -lopencv_core -lopencv_legacy -lopencv_imgproc -lopencv_photo -lopencv_highgui
