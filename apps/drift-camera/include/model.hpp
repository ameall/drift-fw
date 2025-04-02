/**
 * @file model.hpp
 * @brief Model class declaration
 */

#pragma once

#include <cstdint>
#include <map>
#include <opencv2/opencv.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "message.hpp"
#include "socket.hpp"

static const std::string PERSON = "person";
static const std::string CAR = "car";
static const std::string BUS = "bus";
static const std::string TRUCK = "truck";

static const std::unordered_map<std::string, uint8_t> class_map = {
    { CAR, 2u },
    { TRUCK, 7u}
};

const std::vector<std::string> class_names = { CAR, TRUCK };
const std::vector<int32_t> class_values = { class_map.at(CAR), class_map.at(TRUCK) };

struct detection {
    int32_t class_id;
    float confidence;
    cv::Rect box;
};

/**
 * @class Model
 * @brief Manages configuration of and interaction with the CV model being used
 *      to process camera image frames
 */
class Model {
  public:
    explicit Model();

    ~Model() = default;

    // Deleting copy and move constructors
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&&) = delete;
    Model& operator=(Model&&) = delete;

    /**
     * @brief Configures the CV model based on a provided .onnx file, or .cfg/
     *      .weights file pair
     */
    void configure_model();

    /**
     * @brief Processes an individual frame via the CV model
     *
     * @param frame Individual frame to process
     */
    void process_frame(cv::Mat frame);

    /**
     * @brief Calculates the IoU (Intersection over Union) to determine the
     *      detection accuracy for a given object
     *
     * @param first_box 
     * @param second_box 
     * @return IoU as a float
     */
    float calculate_iou(const detection &first_box, const detection &second_box) const;

    /**
     * @brief Checks if the user presses the escape key to end processing
     *
     * @return true if the escape key was pressed
     * @return false if the escape key was not pressed
     */
    bool end_processing() const;

    /**
     * @brief Test function to allow us to run the model on a saved .mp4 video
     */
    void process_mp4(const std::string);

    // Function to process network outputs and return the bounding boxes after NMS
    /**
     * @brief Processes the network outputs and returns the bounding boxes
     *      after NMS (Non Maximum Suppression)
     *
     * @param frame 
     * @param outputs 
     * @param class_values 
     */
    std::vector<detection> process_outputs(const cv::Mat& frame, const std::vector<cv::Mat>& outputs, const std::vector<int>& class_values);

    /**
     * @brief Draws bounding boxes on the image frame
     *
     * @param frame 
     * @param detections 
     * @param classNames 
     * @param class_values 
     */
    void draw_bounding_boxes(cv::Mat& frame, const std::vector<detection>& detections, const std::vector<std::string>& classNames, const std::vector<int>& class_values);

    /**
     * @brief Sends the results of CV processing over the UNIX socket
     *
     * @param detection_area Area of an individual detection box
     * @param delta_x X distancee in pixels between center of detection box and
     *      center of image
     * @param delta_y Y distancee in pixels between center of detection box and
     *      center of image
     */
    void send_results(const int32_t detection_area, const int16_t delta_x, const int16_t delta_y);

  private:
    // Model and processing objects
    cv::dnn::Net net;
    cv::Mat frame;
    cv::Mat blob;
    std::vector<cv::Mat> outputs;
    std::map<int32_t, detection> tracked_boxes;

    // Output processing objects
    std::vector<int8_t> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    std::vector<int32_t> indices;
    std::vector<detection> detections;
    std::map<int32_t, detection> new_tracked_boxes;
    int32_t next_id;

    // Results sending objects
    SocketManager socket;
    OutputsMessage message;
    int32_t message_id;

    // Output processing functions
    void extract_outputs(const cv::Mat &frame, const std::vector<cv::Mat> &outputs, const std::vector<int32_t> &class_values);
};
