#ifndef HSV_TRACKER_H
#define HSV_TRACKER_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "camera.hpp"

using namespace cv;
using namespace std;


class HSV_Tracker {
private:
    Scalar lower, upper;
    vector<Rect> trackedObjects;
    double iouThreshold;

    void getHSVRange(const Mat& img);
    double calculateIoU(const Rect& box1, const Rect& box2);

public:

    HSV_Tracker() : iouThreshold(0.3)

    bool initialize(const string& colorImagePath)
    void processVideo(shared_ptr<Camera> camera)


};
#endif