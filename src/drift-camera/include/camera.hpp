#include <cstdint>
#include <libcamera/libcamera.h>
#include <memory>

class Camera {
  public:
    Camera();
    ~Camera() = default;

    int8_t start_camera();

    void print_cameras();

  private:
    std::unique_ptr<libcamera::CameraManager> camera_manager;
};
