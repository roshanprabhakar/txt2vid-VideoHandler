#include <string>
#include <chrono>

#include <stdio.h>
#include <opencv2/opencv.hpp>

using std::string;
using TimeStamp = std::chrono::high_resolution_clock::time_point;

TimeStamp currentTime() {
	return std::chrono::high_resolution_clock::now();
}

float getTimeDifference(TimeStamp end, TimeStamp start) {
	std::chrono::duration<float> seconds = end - start;
	float delta = seconds.count();
	return delta;
}

int main() {

	cv::Mat img;
	cv::namedWindow("testWindow");

	cv::VideoCapture cap(0); // reads at just over 29 fps
	if (!cap.isOpened()) {
		printf("cannot open camera!\n");
		return -1;
	}

	TimeStamp start =	currentTime();
	TimeStamp cur;
	double frames = 0;
	while (true) {
		cap >> img;
		cv::imshow("testWindow", img);
		cv::waitKey(1);
		frames++;
		cur = currentTime();
		if (getTimeDifference(cur, start) > 1) {
			printf("fps: %f\n", frames / getTimeDifference(cur, start));
			start = cur;
			frames = 0;
		}	
	}

	return 0;
}
