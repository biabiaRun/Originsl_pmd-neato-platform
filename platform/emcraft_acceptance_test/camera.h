#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <string>

class Camera : public royale::IDepthDataListener
{
private:
    void onNewData(const royale::DepthData *data) override;
    std::unique_ptr<royale::ICameraDevice> camera_; // The camera device
    int access_level_;
    std::string id_;                                // Unique ID for the camera device
    std::string use_case_;                          // Camera use_case_
    royale::StreamId stream_id_;                    // FIRST stream ID for the given use_case_
public:
    enum CameraError
    {
        NONE = 0,
        CAM_NOT_DETECTED,
        CAM_NOT_CREATED,
        CAM_NOT_INITIALIZED,
        CAM_STREAM_ERROR,
        ACCESS_LEVEL_ERROR,
        EXPOSURE_MODE_ERROR,
        USE_CASE_ERROR,
        PROCESSING_PARAMETER_ERROR,
    };

    Camera() : camera_(nullptr) {};
    Camera(std::unique_ptr<royale::ICameraDevice> c)
        : camera_(std::move(c))
    {};
    inline const std::string GetID() const { return id_; }

    CameraError RunAccessLevelTests(int user_level);
    CameraError RunExposureTests();
    CameraError RunInitializeTests();
    CameraError RunProcessingParametersTests();
    CameraError RunStreamTests();
    CameraError RunUseCaseTests();

};

#endif // __CAMERA_H__
