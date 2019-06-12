// main.cpp
// Griffith Droid Racer 2019

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

cv::Rect getROI(cv::Mat*);
void drawHyperplane(cv::Mat*, cv::Point2f, cv::Point2f, cv::Scalar);
void drawStaticCrosshair(cv::Mat*);
int capDev = 0;

int main(int, char**) {
    cv::VideoCapture cap(capDev);
    if (!cap.isOpened()) {
        std::cout << "Failed to open camera." << std::endl;
        return -1;
    } else {
        std::cout << "Opened camera successfully." << std::endl;
    }

    cv::Mat edges, frame;

    // Calculate the region-of-interest (ROI), i.e. lower half of input.
    cap >> frame;
    cv::Rect ROI = getROI(&frame);

    // Define colour regions. Below is a light blue-ish.
    cv::Scalar rgbLower(85, 60, 40);
    cv::Scalar rgbUpper(125, 255, 255);

    while (1) {
        // Get next frame from camera and crop to ROI.
        cap >> frame;

        // Blur frame.
        // Old: GaussianBlur(frame, edges, cv::Size(7, 7), 4, 4);
        bilateralFilter(frame, edges, 9, 50, 50);

        // Convert frame to HSV.
        cvtColor(edges, edges, cv::COLOR_BGR2HSV);
        // imshow("Colors", edges);

        // Reduce to color range seeking
        inRange(edges, rgbLower, rgbUpper, edges);
        // imshow("Range", edges);

        // Apply Canny edge detection.
        Canny(edges, edges, 0, 30, 3);
        // imshow("Canny", edges);

        // Apply Hough Transform line detection and draw hyperplanes.
        std::vector<cv::Vec4i> lines;
        HoughLinesP(edges, lines, 1, CV_PI / 180, 50, 50, 10);
        for (size_t j = 0; j < lines.size(); j++) {
            cv::Vec4i l = lines[j];
            drawHyperplane(&frame, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]),
                           cv::Scalar(0, 0, 255));
            cv::line(frame, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]),
                     cv::Scalar(255, 255, 255), 2, CV_AA);
        }

        // Apply crosshairs
        drawStaticCrosshair(&frame);

        // Show frame
        imshow("Lines - seeking: blues", frame);

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
                    cv::Scalar colour) {
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
    } else {
        p.x = q.x = p2.x;
        p.y = 0;
        q.y = frame->rows;
    }

    cv::line(*frame, p, q, colour, 1);
}

void drawStaticCrosshair(cv::Mat* frame) {
    int x_min = 0;
    int x_max = frame->cols;
    int x_mid = x_max / 2;
    int y_min = 0;
    int y_max = frame->rows;
    int y_mid = y_max / 2;

    // Draw horizontal bar
    cv::line(*frame, cv::Point2i(x_min, y_mid), cv::Point2i(x_max, y_mid),
             cv::Scalar(255, 255, 255), 1);

    // Draw vertical bar
    cv::line(*frame, cv::Point2i(x_mid, y_min), cv::Point2i(x_mid, y_max),
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