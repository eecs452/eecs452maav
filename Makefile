
all:
	gcc -pg -I/usr/local/include/opencv hello-world.cpp -lopencv_core -lopencv_legacy -lopencv_imgproc -lopencv_photo -lopencv_highgui
