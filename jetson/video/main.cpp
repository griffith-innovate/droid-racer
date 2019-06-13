// main.cpp
// Griffith Droid Racer 2019

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

cv::Rect getROI(cv::Mat*);
void drawHyperplane(cv::Mat*, cv::Point2f, cv::Point2f, cv::Scalar, int);
void drawStaticCrosshair(cv::Mat*);
int capDev = 0;

int main(int, char**) {
	cv::VideoCapture cap(capDev);
	if (!cap.isOpened()) {
		std::cout << "Failed to open camera." << std::endl;
		return -1;
	}
	else {
		std::cout << "Opened camera successfully." << std::endl;
	}

	cv::Mat edges, edges_b, edges_y, frame;

	// Calculate the region-of-interest (ROI), i.e. lower half of input.
	cap >> frame;
	cv::Rect ROI = getROI(&frame);

	cv::Scalar blueLower(102, 80, 40);
	cv::Scalar blueUpper(122, 255, 255);

	cv::Scalar yellowLower(20, 90, 160);
	cv::Scalar yellowUpper(60, 255, 255);

	while (1) {
		// Get next frame from camera and crop to ROI.
		cap >> frame;

		// Segment/Reduce to ROI
		frame = frame(ROI);

		// Blur frame.
		// GaussianBlur(frame, edges, cv::Size(7, 7), 4, 4);
		bilateralFilter(frame, edges, 9, 50, 50);

		// Convert frame to HSV.
		cvtColor(edges, edges, cv::COLOR_BGR2HSV);
		// imshow("Colors", edges);

		// Reduce to color range seeking
		inRange(edges, blueLower, blueUpper, edges_b);
		inRange(edges, yellowLower, yellowUpper, edges_y);
		// imshow("Range", edges);

		// Apply Canny edge detection.
		Canny(edges_b, edges_b, 0, 30, 3);
		Canny(edges_y, edges_y, 0, 30, 3);
		// imshow("Canny B", edges_b);
		// imshow("Canny Y", edges_y);

		// Apply Hough Transform line detection and draw hyperplanes.
		std::vector<cv::Vec4i> lines, lines_b, lines_y;
		std::vector<int> line  = { 0, 0, 0, 0 };
		std::vector<int> line_b = { 0, 0, 0, 0 };
		std::vector<int> line_y = { 0, 0, 0, 0 };
		HoughLinesP(edges_b, lines_b, 1, CV_PI / 180, 50, 50, 10);
		HoughLinesP(edges_y, lines_y, 1, CV_PI / 180, 50, 50, 10);
		lines.insert(lines.end(), lines_b.begin(), lines_b.end());
		lines.insert(lines.end(), lines_y.begin(), lines_y.end());

		// Get average line detected from blue range
		if (lines_b.size() > 0) {
			// Draw individual detected blue lines
			for (size_t j = 0; j < lines_b.size(); j++) {
				cv::Vec4i l = lines_b[j];
				cv::line(frame, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]),
					cv::Scalar(255, 0, 0), 1, CV_AA);
			}

			for (size_t j = 0; j < lines_b.size(); j++) {
				for (int k = 0; k < 4; k++) {
					line_b[k] += lines_b[j][k];
				}
			}

			for (int k = 0; k < 4; k++) {
				line_b[k] /= lines_b.size();
			}

			drawHyperplane(&frame, cv::Point(line_b[0], line_b[1]), cv::Point(line_b[2], line_b[3]),
				cv::Scalar(255, 0, 0), 2);
		}

		// Get average line detected from yellow range
		if (lines_y.size() > 0) {
			// Draw individual detected yellow lines
			for (size_t j = 0; j < lines_y.size(); j++) {
				cv::Vec4i l = lines_y[j];
				cv::line(frame, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]),
					cv::Scalar(0, 255, 255), 1, CV_AA);
			}

			// Draw the average yellow hyperplane
			for (size_t j = 0; j < lines_y.size(); j++) {
				for (int k = 0; k < 4; k++) {
					line_y[k] += lines_y[j][k];
				}
			}

			for (int k = 0; k < 4; k++) {
				line_y[k] /= lines_y.size();
			}

			drawHyperplane(&frame, cv::Point(line_y[0], line_y[1]), cv::Point(line_y[2], line_y[3]),
				cv::Scalar(0, 255, 255), 2);
		}

		// Draw the average hyperplane
		for (int k = 0; k < 4; k++) {
			line[k] = (line_b[k] + line_y[k]) / 2;
		}

		drawHyperplane(&frame, cv::Point(line[0], line[1]-10), cv::Point(line[2], line[3]-10),
			cv::Scalar(0, 0, 255), 2);

		drawHyperplane(&frame, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]),
			cv::Scalar(0, 0, 255), 2);

		drawHyperplane(&frame, cv::Point(line[0], line[1]+10), cv::Point(line[2], line[3]+10),
			cv::Scalar(0, 0, 255), 2);

		// Apply crosshairs
		// drawStaticCrosshair(&frame);

		// Show frame 
		cv::putText(frame, "Lanes", cv::Point(10, frame.rows - 10), cv::FONT_HERSHEY_DUPLEX, 2, cv::Scalar(255, 255, 255), 2);
		imshow("Lines", frame);

		if (cv::waitKey(30) >= 0) {
			std::cout << "Terminating." << std::endl;
			break;
		}
	}

	return 0;
}

// Returns a mask for bottom-half of image.
cv::Rect getROI(cv::Mat* sampleFrame) {
	int x = 0;
	int y = (sampleFrame->rows) / 2;
	int width = (sampleFrame->cols);
	int height = (sampleFrame->rows) / 2;

	std::cout << "ROI from (" << x << ", " << y << "): " << width << "x"
		<< height << "." << std::endl;
	return /*ROI: */ cv::Rect(x, y, width, height);
}

// Draws the hyperplane formed from two given points on the given image.
// Slightly edited from
// https://stackoverflow.com/questions/13160722/how-to-draw-line-not-line-segment-opencv-2-4-2.
void drawHyperplane(cv::Mat* frame, cv::Point2f p1, cv::Point2f p2,
	cv::Scalar colour, int thickness=1) {
	cv::Point p, q;

	if (p1.x != p2.x) {
		// Slope equation: m = (y1 - y2) / (x1 - x2)
		float m = (p1.y - p2.y) / (p1.x - p2.x);
		// Line equation:  y = mx + b => b = y - mx
		float b = p1.y - (m * p1.x);
		p.x = 0;
		p.y = b;
		q.x = frame->cols;
		q.y = m * q.x + b;
	}
	else {
		p.x = q.x = p2.x;
		p.y = 0;
		q.y = frame->rows;
	}

	cv::line(*frame, p, q, colour, thickness);
}

void drawStaticCrosshair(cv::Mat* frame) {
	int x_max = frame->cols;
	int x_mid = x_max / 2;
	int y_max = frame->rows;
	int y_mid = y_max / 2;

	// Draw horizontal bar
	cv::line(*frame, cv::Point2i(0, y_mid), cv::Point2i(x_max, y_mid),
		cv::Scalar(255, 255, 255), 1);

	// Draw vertical bar
	cv::line(*frame, cv::Point2i(x_mid, 0), cv::Point2i(x_mid, y_max),
		cv::Scalar(255, 255, 255), 1);

	// Draw top-left flavour
	cv::line(*frame, cv::Point2i(x_mid - 2, y_mid - 32),
		cv::Point2i(x_mid - 2, y_mid - 2), cv::Scalar(255, 255, 255), 1);
	cv::line(*frame, cv::Point2i(x_mid - 42, y_mid - 2),
		cv::Point2i(x_mid - 2, y_mid - 2), cv::Scalar(255, 255, 255), 1);

	// Draw bottom-right flavour
	cv::line(*frame, cv::Point2i(x_mid + 2, y_mid + 32),
		cv::Point2i(x_mid + 2, y_mid + 2), cv::Scalar(255, 255, 255), 1);
	cv::line(*frame, cv::Point2i(x_mid + 42, y_mid + 2),
		cv::Point2i(x_mid + 2, y_mid + 2), cv::Scalar(255, 255, 255), 1);
}