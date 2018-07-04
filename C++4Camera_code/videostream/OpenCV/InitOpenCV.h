
#ifndef INITOPENCV_H__
#define INITOPENCV_H__

#include "Include\highgui.h"
#include "Include\opencv.hpp"




#ifdef _DEBUG
#pragma comment (lib, "OpenCV/Lib/opencv_world300d.lib")
#pragma comment (lib, "OpenCV/Lib/opencv_ts300d.lib")

#else
#pragma comment (lib, "OpenCV/Lib/opencv_world300.lib")
#pragma comment (lib, "OpenCV/Lib/opencv_ts300.lib")
#endif



using namespace cv;

#endif

