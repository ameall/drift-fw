/**
 * @file model.cpp
 * @brief Defines CV model functions that will be used for camera image
 *      processing
 */

#include <cstdint>
#include <opencv2/opencv.hpp>
#include <string>
#include <sys/socket.h>
#include <vector>

#include "logging.hpp"
#include "model.hpp"

static bool box_selected = false;
static std::vector<detection> detected_boxes;
static detection selected_box;

/**
 * @brief Checks if a click is inside any bounding box
 *
 * @param event 
 * @param click_x 
 * @param click_y 
 * @param flags 
 * @param param 
 */
static void mouse_callback(int32_t event, int32_t click_x, int32_t click_y, int32_t flags, void* param) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        for (const auto& box : detected_boxes) {
            if (box.box.contains(cv::Point(click_x, click_y))) {
                selected_box = box;
                box_selected = true;
                return;
            }
        }
    }
}

constexpr float CONFIDENCE_THRESHOLD = 0.1;
constexpr float NMS_THRESHOLD = 0.4;

static const int16_t MODEL_PIXEL_WIDTH = 416;
static const int16_t MODEL_PIXEL_HEIGHT = 416;

constexpr uint32_t WAITKEY_DELAY = 10u;
constexpr uint8_t ESCAPE_KEY_CODE = 27u;

Model::Model() : next_id(0), message_id(0)
{
    configure_model();

    socket.initialize_socket();
}

void Model::configure_model()
{
    // static const std::string MODEL_CFG_PATH = "/home/mealla/Documents/GitHub/drift-tracking/untracked-models/yolov4-tiny.cfg";
    // static const std::string MODEL_WEIGHTS_PATH = "/home/mealla/Documents/GitHub/drift-tracking/untracked-models/yolov4-tiny.weights";
    // static const std::string MODEL_ONNX_PATH = "/home/mealla/Documents/GitHub/drift-tracking/models/yolov5su.onnx";
    static const std::string MODEL_CFG_PATH = "/home/drift/drift-tracking/untracked-models/yolov4-tiny.cfg";
    static const std::string MODEL_WEIGHTS_PATH = "/home/drift/drift-tracking/untracked-models/yolov4-tiny.weights";
    static const std::string MODEL_ONNX_PATH = "/home/drift/drift-tracking/models/yolov5su.onnx";

    // net = cv::dnn::readNetFromONNX(MODEL_ONNX_PATH);
    net = cv::dnn::readNetFromDarknet(MODEL_CFG_PATH, MODEL_WEIGHTS_PATH);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

void Model::process_frame(cv::Mat frame)
{
    //cv::setMouseCallback("Frame", mouse_callback, nullptr);

    //cv::imshow("Frame", frame);
    cv::resize(frame, frame, cv::Size(MODEL_PIXEL_WIDTH, MODEL_PIXEL_HEIGHT));  // YOLOv3 typically uses 416x416 input size
    blob = cv::dnn::blobFromImage(frame, 1/255.0, cv::Size(MODEL_PIXEL_WIDTH, MODEL_PIXEL_HEIGHT), cv::Scalar(0, 0, 0), true, false);
    net.setInput(blob);
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    detected_boxes = process_outputs(frame, outputs, class_values);

    //draw_bounding_boxes(frame, detected_boxes, class_names, class_values);
    //cv::imshow("Frame", frame);
}

bool Model::end_processing() const
{
    return (cv::waitKey(WAITKEY_DELAY) == ESCAPE_KEY_CODE) ? true : false;
}

float Model::calculate_iou(const detection &first_box, const detection &second_box) const
{
    int16_t intersection_x1 = std::max(first_box.box.x, second_box.box.x);
    int16_t intersection_y1 = std::max(first_box.box.y, second_box.box.y);
    int16_t intersection_x2 = std::min(first_box.box.x + first_box.box.width, second_box.box.x + second_box.box.width);
    int16_t intersection_y2 = std::min(first_box.box.y + first_box.box.height, second_box.box.y + second_box.box.height);

    int16_t intersection_width = std::max(0, intersection_x2 - intersection_x1);
    int16_t intersection_height = std::max(0, intersection_y2 - intersection_y1);
    int16_t intersection_area = intersection_width * intersection_height;

    int32_t first_box_area = first_box.box.width * first_box.box.height;
    int32_t second_box_area = second_box.box.width * second_box.box.height;

    int32_t union_area = first_box_area + second_box_area - intersection_area;
    if (union_area <= 0) {
        return 0.0f;
    }

    return static_cast<float>(intersection_area) / static_cast<float>(union_area);
}

void Model::process_mp4(const std::string video_path)
{
    cv::VideoCapture cap(video_path);
    if (!cap.isOpened()) {
        std::cerr << "Error opening video file" << std::endl;
    }

    box_selected = false;
    while (cap.read(frame)) {
        process_frame(frame);

        if (end_processing()) {
            break;
        }
    }
}

void Model::extract_outputs(const cv::Mat &frame, const std::vector<cv::Mat> &outputs, const std::vector<int32_t> &class_values)
{
    log_message(INFO, "Model::extract_outputs(): Extracting outputs from model");
    for (size_t i = 0; i < outputs.size(); ++i) {
        float* data = reinterpret_cast<float*>(outputs[i].data);
        for (int32_t j = 0; j < outputs[i].rows; ++j, data += outputs[i].cols) {
            float confidence = data[4];
            if (confidence >= CONFIDENCE_THRESHOLD) {
                int8_t class_id = std::max_element(data + 5, data + outputs[i].cols) - (data + 5);
                float score = data[5 + class_id];

                if (score >= CONFIDENCE_THRESHOLD && find(class_values.begin(), class_values.end(), class_id) != class_values.end()) {
                    int32_t center_x = static_cast<int>(data[0] * frame.cols);
                    int32_t center_y = static_cast<int>(data[1] * frame.rows);
                    int32_t width = static_cast<int>(data[2] * frame.cols);
                    int32_t height = static_cast<int>(data[3] * frame.rows);
                    int32_t left = center_x - width / 2;
                    int32_t top = center_y - height / 2;

                    class_ids.push_back(class_id);
                    confidences.push_back(score);
                    boxes.push_back(cv::Rect(left, top, width, height));

                    int32_t area = width * height;
                    int16_t delta_x = center_x - MODEL_PIXEL_WIDTH;
                    int16_t delta_y = center_y - MODEL_PIXEL_HEIGHT;
                    log_message(INFO, "Model::extract_outputs(): Results: Area: %d, X Delta: %d, Y Delta: %d", area, delta_x, delta_y);
                    send_results(width * height, center_x - MODEL_PIXEL_WIDTH, center_y - MODEL_PIXEL_HEIGHT);
                }
            }
        }
    }
}

std::vector<detection> Model::process_outputs(const cv::Mat &frame, const std::vector<cv::Mat> &outputs, const std::vector<int32_t> &class_values)
{
    extract_outputs(frame, outputs, class_values);

    cv::dnn::NMSBoxes(boxes, confidences, CONFIDENCE_THRESHOLD, NMS_THRESHOLD, indices);

    for (size_t i = 0; i < indices.size(); ++i) {
        int32_t idx = indices[i];
        detections.push_back( {class_ids[idx], confidences[idx], boxes[idx]} );
    }

    // TODO: Pull this out into a separate static function
    // Match existing boxes or assign new IDs
    for (const auto& detection : detections) {
        // Find best matching box by iou
        float max_iou = 0.0f;
        int8_t best_match_id = -1;
        for (const auto& [id, box] : tracked_boxes) {
            float iou = calculate_iou(box, detection);
            if (iou > max_iou) {
                max_iou = iou;
                best_match_id = id;
            }
        }

        // Update the existing box or assign a new ID
        (max_iou > 0.5) ? new_tracked_boxes[best_match_id] = detection : new_tracked_boxes[next_id++] = detection;
    }

    tracked_boxes = std::move(new_tracked_boxes);
    return detections;
}

void Model::draw_bounding_boxes(cv::Mat& frame, const std::vector<detection>& detections, const std::vector<std::string>& classNames, const std::vector<int32_t>& class_values)
{
    for (const auto& [id, detection] : tracked_boxes) {
        const cv::Rect& box = detection.box;
        float iou = calculate_iou(selected_box, detection);

        // Determine color
        cv::Scalar color = (box_selected && iou > 0.5)
                           ? cv::Scalar(255, 0, 0) // Blue for selected boxes
                           : cv::Scalar(0, 255, 0); // Green for others

        if (iou > 0.5) {
            selected_box = detection;
        }

        // Draw bounding box
        cv::rectangle(frame, box, color, 2);

        // Prepare label with ID and class name
        //std::string label = cv::format("ID: %d | %.2f", id, detection.confidence);
        //label = classNames[targetClasses[detection.classId]] + ": " + label;

        // Calculate position for label
        //int32_t baseLine;
        //cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        //int32_t top = std::max(box.y, labelSize.height);
        //cv::putText(frame, label, cv::Point(box.x, top), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
    }
}

void Model::send_results(const int32_t detection_area, const int16_t delta_x, const int16_t delta_y)
{
    message.create_message(message_id, detection_area, delta_x, delta_y);

    socket.send_message(message);
}
