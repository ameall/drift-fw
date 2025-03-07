

#include <cwchar>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <libcamera/libcamera/formats.h>
#include <memory>
#include <sys/mman.h>
#include <thread>

#include <libcamera/libcamera.h>

#include <jpeglib.h>
#include <vector>

void save_jpeg_image(const uint8_t *data, int width, int height, const std::string &filename)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    std::FILE *outfile = fopen(filename.c_str(), "wb");
    if (!outfile) {
        fprintf(stderr, "Failed to open file for writing: %s\n", filename.c_str());
        return;
    }

    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 90, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    std::vector<uint8_t> rgb_data(width * height * 3);
    for (int i = 0; i < width * height; ++i) {
        rgb_data[i * 3 + 0] = data[i * 4 + 2];  // R
        rgb_data[i * 3 + 1] = data[i * 4 + 1];  // G
        rgb_data[i * 3 + 2] = data[i * 4 + 0];  // B
    }

    while (cinfo.next_scanline < cinfo.image_height) {
        JSAMPROW row_pointer = (JSAMPROW)&rgb_data[cinfo.next_scanline * width * 3];
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
}

static void *map_frame_buffer(const libcamera::FrameBuffer::Plane &plane)
{
    void *mapped_memory = mmap(nullptr, plane.length, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), plane.offset);
    if (mapped_memory == MAP_FAILED) {
        fprintf(stderr, "Failed to map memory\n");
        return nullptr;
    }
    return mapped_memory;
}

using namespace libcamera;
using namespace std::chrono_literals;

static std::shared_ptr<Camera> camera;

static void requestComplete(Request *request)
{
    if (request->status() == Request::RequestCancelled)
        return;

    const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();

    for (auto bufferPair : buffers) {
        FrameBuffer *buffer = bufferPair.second;
        const FrameMetadata &metadata = buffer->metadata();
        std::cout << " seq: " << std::setw(6) << std::setfill('0') << metadata.sequence << " bytesused: ";

        unsigned int nplane = 0;
        for (const FrameMetadata::Plane &plane : metadata.planes())
        {
            std::cout << plane.bytesused;
            if (++nplane < metadata.planes().size()) std::cout << "/";
        }

        std::cout << std::endl;

        libcamera::FrameBuffer::Plane plane = buffer->planes().at(0);

        void *data = map_frame_buffer(plane);

        const uint8_t *pixel_data = static_cast<const uint8_t *>(data);
        int width = 800;
        int height = 600;

        std::string filename = "image1.jpg";
        save_jpeg_image(pixel_data, width, height, filename);
        fprintf(stdout, "Saved frame: %s\n", filename.c_str());
    }

    request->reuse(Request::ReuseBuffers);
    camera->queueRequest(request);
}

int main()
{
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();

    for (auto const &camera : cm->cameras())
        std::cout << camera->id() << std::endl;

    auto cameras = cm->cameras();
    if (cameras.empty()) {
        std::cout << "No cameras were identified on the system."
                  << std::endl;
        cm->stop();
        return EXIT_FAILURE;
    }

    std::string cameraId = cameras[0]->id();

    camera = cm->get(cameraId);
    /*
     * Note that `camera` may not compare equal to `cameras[0]`.
     * In fact, it might simply be a `nullptr`, as the particular
     * device might have disappeared (and reappeared) in the meantime.
     */

    camera->acquire();

    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::Viewfinder } );

    StreamConfiguration &streamConfig = config->at(0);
    std::cout << "Default viewfinder configuration is: " << streamConfig.toString() << std::endl;

    streamConfig.pixelFormat = libcamera::formats::XRGB8888;
    streamConfig.size.width = 800;
    streamConfig.size.height = 600;

    config->validate();
    std::cout << "Validated viewfinder configuration is: " << streamConfig.toString() << std::endl;

    camera->configure(config.get());

    FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);

    for (StreamConfiguration &cfg : *config) {
        int ret = allocator->allocate(cfg.stream());
        if (ret < 0) {
            std::cerr << "Can't allocate buffers" << std::endl;
            return -ENOMEM;
        }

        size_t allocated = allocator->buffers(cfg.stream()).size();
        std::cout << "Allocated " << allocated << " buffers for stream" << std::endl;
    }

    Stream *stream = streamConfig.stream();
    const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
    std::vector<std::unique_ptr<Request>> requests;

    for (unsigned int i = 0; i < buffers.size(); ++i) {
        std::unique_ptr<Request> request = camera->createRequest();
        if (!request)
        {
            std::cerr << "Can't create request" << std::endl;
            return -ENOMEM;
        }

        const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
        int ret = request->addBuffer(stream, buffer.get());
        if (ret < 0)
        {
            std::cerr << "Can't set buffer for request"
                  << std::endl;
            return ret;
        }

        requests.push_back(std::move(request));
    }

    camera->requestCompleted.connect(requestComplete);

    camera->start();
    for (std::unique_ptr<Request> &request : requests)
        camera->queueRequest(request.get());

    std::this_thread::sleep_for(1000ms);

    camera->stop();
    allocator->free(stream);
    delete allocator;
    camera->release();
    camera.reset();
    cm->stop();

    return 0;
}
