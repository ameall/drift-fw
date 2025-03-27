#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// Function to get HSV range from an image
void getHSVRange(const Mat& img, Scalar& lower, Scalar& upper) {
    // Red has two ranges in HSV
    lower = Scalar(0, 50, 50);   // Lower range for red
    upper = Scalar(10, 255, 255); // Upper range for lighter red
    // Alternatively, for darker red, use the second range:
    // Scalar lower2(170, 50, 50), upper2(180, 255, 255);
}

// Function to calculate IoU (Intersection over Union)
double calculateIoU(const Rect& box1, const Rect& box2) {
    int x1 = max(box1.x, box2.x);
    int y1 = max(box1.y, box2.y);
    int x2 = min(box1.x + box1.width, box2.x + box2.width);
    int y2 = min(box1.y + box1.height, box2.y + box2.height);
    
    int intersectionArea = max(0, x2 - x1) * max(0, y2 - y1);
    int unionArea = box1.area() + box2.area() - intersectionArea;
    
    return (double)intersectionArea / unionArea;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cout << "Usage: ./color_tracker <color_image> <video_path>" << endl;
        return -1;
    }
    
    // Load color reference image
    Mat colorImg = imread(argv[1]);
    cvtColor(colorImg, colorImg, COLOR_RGB2BGR);
    if (colorImg.empty()) {
        cerr << "Error loading color reference image" << endl;
        return -1;
    }
    
    Scalar lower, upper;
    getHSVRange(colorImg, lower, upper);
    cout << "Lower HSV: " << lower << " Upper HSV: " << upper << endl;
    
    // Open video file
    VideoCapture cap(argv[2]);
    if (!cap.isOpened()) {
        cerr << "Error opening video file" << endl;
        return -1;
    }
    
    Mat frame, hsv, mask;
    vector<Rect> trackedObjects;
    double iouThreshold = 0.3;
    
    while (cap.read(frame)) {
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        inRange(hsv, lower, upper, mask);
        
        imshow("Mask", mask);
        
        // Find contours
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        
        vector<Rect> detectedObjects;
        for (const auto& contour : contours) {
            if (contourArea(contour) > 500) { // Filter small contours
                detectedObjects.push_back(boundingRect(contour));
            }
        }
                
        // Update tracked objects using IoU-based tracking
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
        
        // Add new detections that were not matched
        for (size_t i = 0; i < detectedObjects.size(); ++i) {
            if (!matched[i]) {
                newTrackedObjects.push_back(detectedObjects[i]);
            }
        }
        
        trackedObjects = newTrackedObjects;
        
        // Draw bounding boxes
        for (const auto& box : trackedObjects) {
            rectangle(frame, box, Scalar(0, 255, 0), 2);
        }
        
        imshow("Tracked Objects", frame);
        if (waitKey(30) == 27) break; // Press ESC to exit
    }
    
    cap.release();
    destroyAllWindows();
    return 0;
}
