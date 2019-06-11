// main.cpp
// Griffith Droid Racer 2019

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

using namespace cv;

Rect getROI(Mat*);

int capDev = 0;

int main(int, char**)
{
	VideoCapture cap(capDev);
	if (!cap.isOpened()) {
		std::cout << "Failed to open camera." << std::endl;
		return -1;
	}
	else {
		std::cout << "Opened camera successfully." << std::endl;
	}

	// Calculate the region-of-interest (ROI), i.e. lower half of input.
	Mat edges, frame;
	cap >> frame;
	Rect ROI = getROI(&frame);

	while (1) {
		// Get next frame from camera and crop to ROI.
		cap >> frame;
		frame = frame(ROI);

		// Convert frame to gray.
		cvtColor(frame, edges, COLOR_BGR2GRAY);

		// Blur frame.
		GaussianBlur(edges, edges, Size(7, 7), 4, 4);

		// Apply Canny edge detection.
		Canny(edges, edges, 0, 30, 3);

		// Show frame.
		imshow("Camera", edges);
		
		if (waitKey(30) >= 0) {
			std::cout << "Terminating." << std::endl;
			break;
		}
	}
	
	return 0;
}

// Returns a mask for bottom-half of image.
Rect getROI(Mat* sampleFrame) {
	int x = 0;
	int y = (sampleFrame->rows) / 2;
	int width = (sampleFrame->cols);
	int height = (sampleFrame->rows) / 2;

	std::cout << "ROI from (" << x << ", " << y << "): " << width << "x" << height << "." << std::endl;
	return Rect(x, y, width, height);
}