#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>


#include "message.hpp"
#include "socket.hpp"
#include "tracker.hpp"

using namespace cv;
using namespace std;

HSV_Tracker::HSV_Tracker() {
	iouThreshold = 0.5;
}

class HSV_Tracker {

    void getHSVRange(const Mat& img) {
        lower = Scalar(0, 50, 50);
        upper = Scalar(10, 255, 255);
    }

    double calculateIoU(const Rect& box1, const Rect& box2) {
		int x1 = max(box1.x, box2.x);
		int y1 = max(box1.y, box2.y);
		int x2 = min(box1.x + box1.width, box2.x + box2.width);
		int y2 = min(box1.y + box1.height, box2.y + box2.height);
		
		int intersectionArea = max(0, x2 - x1) * max(0, y2 - y1);
		int unionArea = box1.area() + box2.area() - intersectionArea;
		
		return (double)intersectionArea / unionArea;
	}

    bool initialize(const string& colorImagePath) {
        Mat colorImg = imread(colorImagePath);
        if (colorImg.empty()) {
            cerr << "Error loading color reference image" << endl;
            return false;
        }
        getHSVRange(colorImg);
        return true;
    }

    void processVideo(shared_ptr<Camera> camera) {

        Mat frame, hsv, mask;
        while (true) {
            frame = camera->get_frame();
            if (frame.empty()) continue;

            cvtColor(frame, hsv, COLOR_BGR2HSV);
            inRange(hsv, lower, upper, mask);

            imshow("Mask", mask);

            vector<vector<Point>> contours;
            findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

            vector<Rect> detectedObjects;
            for (const auto& contour : contours) {
                if (contourArea(contour) > 500) {
                    detectedObjects.push_back(boundingRect(contour));
                }
            }

            vector<Rect> newTrackedObjects;
            vector<bool> matched(detectedObjects.size(), false);

            for (auto& tracked : trackedObjects) {
                double bestIoU = 0;
                int bestMatchIdx = -1;

                for (size_t i = 0; i < detectedObjects.size(); ++i) {
                    double iou = calculateIoU(detectedObjects[i], tracked);
                    if (iou > bestIoU) {
                        bestIoU = iou;
                        bestMatchIdx = i;
                    }
                }

                if (bestMatchIdx != -1 && bestIoU > iouThreshold) {
                    newTrackedObjects.push_back(detectedObjects[bestMatchIdx]);
                    matched[bestMatchIdx] = true;
                }
            }

            for (size_t i = 0; i < detectedObjects.size(); ++i) {
                if (!matched[i]) {
                    newTrackedObjects.push_back(detectedObjects[i]);
                }
            }

            trackedObjects = newTrackedObjects;

            for (const auto& box : trackedObjects) {
                rectangle(frame, box, Scalar(0, 255, 0), 2);
            }

            imshow("Tracked Objects", frame);
            if (waitKey(30) == 27) break;
        }
        cap.release();
        destroyAllWindows();
    }

};