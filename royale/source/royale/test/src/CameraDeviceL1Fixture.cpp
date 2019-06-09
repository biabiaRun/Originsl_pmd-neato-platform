/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <CameraDeviceL1Fixture.hpp>

using namespace royale;

CameraDeviceL1Fixture::CameraDeviceL1Fixture() :
    cameraManager (new CameraManager()), validateUseCases (true)
{
}

CameraDeviceL1Fixture::CameraDeviceL1Fixture (std::unique_ptr<CameraManager> manager) :
    cameraManager (std::move (manager))
{
}

CameraDeviceL1Fixture::~CameraDeviceL1Fixture()
{
}

void CameraDeviceL1Fixture::SetUp()
{
#if defined(TARGET_PLATFORM_ANDROID)
    auto connectedCameras = cameraManager->getConnectedCameraList (0);
#else
    auto connectedCameras = cameraManager->getConnectedCameraList();
#endif
    ASSERT_EQ (static_cast<unsigned int> (connectedCameras.size()), 1u);

    camera = cameraManager->createCamera (connectedCameras[0]);

    ASSERT_NE (camera, nullptr);

    cameraId = connectedCameras[0];

    if (cameraNameIs ("PICOS_STANDARD"))
    {
        expectedUseCases.push_back ("MODE_5_45FPS_1000");
    }
    else if (cameraNameIs ("PICOFLEXX"))
    {
        expectedUseCases.push_back ("MODE_9_5FPS_2000");
        expectedUseCases.push_back ("MODE_9_10FPS_1000");
        expectedUseCases.push_back ("MODE_9_15FPS_700");
        expectedUseCases.push_back ("MODE_9_25FPS_450");
        expectedUseCases.push_back ("MODE_5_35FPS_600");
        expectedUseCases.push_back ("MODE_5_45FPS_500");
        expectedUseCases.push_back ("MODE_MIXED_30_5");
        expectedUseCases.push_back ("MODE_MIXED_50_5");
        expectedUseCases.push_back ("Low_Noise_Extended");
        expectedUseCases.push_back ("Fast_Acquisition");
    }
    else if (cameraNameIs ("PICOMAXX1") ||
             cameraNameIs ("PICOMAXX2"))
    {
        expectedUseCases.push_back ("MODE_9_5FPS_1900");
        expectedUseCases.push_back ("MODE_9_10FPS_900");
        expectedUseCases.push_back ("MODE_9_15FPS_600");
        expectedUseCases.push_back ("MODE_9_25FPS_300");
        expectedUseCases.push_back ("MODE_5_35FPS_500");
        expectedUseCases.push_back ("MODE_5_45FPS_400");
        expectedUseCases.push_back ("MODE_5_60FPS_300");
        expectedUseCases.push_back ("MODE_MIXED_30_5");
        expectedUseCases.push_back ("MODE_MIXED_50_5");
    }
    else if (cameraNameIs ("PICOMONSTAR1") ||
             cameraNameIs ("PICOMONSTAR2"))
    {
        expectedUseCases.push_back ("MODE_9_5FPS_1900");
        expectedUseCases.push_back ("MODE_9_10FPS_900");
        expectedUseCases.push_back ("MODE_9_15FPS_600");
        expectedUseCases.push_back ("MODE_9_25FPS_300");
        expectedUseCases.push_back ("MODE_5_35FPS_500");
        expectedUseCases.push_back ("MODE_5_45FPS_400");
        expectedUseCases.push_back ("MODE_5_60FPS_300");
        expectedUseCases.push_back ("MODE_MIXED_30_5");
        expectedUseCases.push_back ("MODE_MIXED_50_5");
    }
    else if (cameraNameStartsWith ("EVALBOARD_"))
    {
        expectedUseCases.push_back ("MODE_5_10FPS_2900");
        expectedUseCases.push_back ("MODE_5_20FPS_2900");
        expectedUseCases.push_back ("MODE_5_30FPS_2900");
        expectedUseCases.push_back ("MODE_5_45FPS_1000");
    }
    else if (cameraNameIs ("Alea945nm"))
    {
        expectedUseCases.push_back ("MODE_9_5FPS");
        expectedUseCases.push_back ("MODE_9_1FPS");
        expectedUseCases.push_back ("MODE_9_2FPS");
        expectedUseCases.push_back ("MODE_9_3FPS");
        expectedUseCases.push_back ("MODE_9_10FPS");
        expectedUseCases.push_back ("MODE_9_15FPS");
        expectedUseCases.push_back ("MODE_9_25FPS");
        expectedUseCases.push_back ("MODE_5_5FPS");
        expectedUseCases.push_back ("MODE_5_10FPS");
        expectedUseCases.push_back ("MODE_5_15FPS");
        expectedUseCases.push_back ("MODE_5_30FPS");
        expectedUseCases.push_back ("MODE_5_35FPS");
        expectedUseCases.push_back ("MODE_5_45FPS");
    }
    else if (cameraNameIs ("Salome940nm"))
    {
        expectedUseCases.push_back ("MODE_9_5FPS");
        expectedUseCases.push_back ("MODE_9_1FPS");
        expectedUseCases.push_back ("MODE_9_2FPS");
        expectedUseCases.push_back ("MODE_9_3FPS");
        expectedUseCases.push_back ("MODE_9_10FPS");
        expectedUseCases.push_back ("MODE_9_15FPS");
        expectedUseCases.push_back ("MODE_9_25FPS");
        expectedUseCases.push_back ("MODE_5_5FPS");
        expectedUseCases.push_back ("MODE_5_10FPS");
        expectedUseCases.push_back ("MODE_5_15FPS");
        expectedUseCases.push_back ("MODE_5_25FPS");
        expectedUseCases.push_back ("MODE_5_30FPS");
        expectedUseCases.push_back ("MODE_5_35FPS");
        expectedUseCases.push_back ("MODE_5_45FPS");
    }
    else
    {
        validateUseCases = false;
    }
}

void CameraDeviceL1Fixture::TearDown()
{

}

void CameraDeviceL1Fixture::initCamera()
{
    auto status = camera->initialize();
    if (status != CameraStatus::SUCCESS)
    {
        throw TestFixtureException ("initCamera failed", status);
    }
}

bool CameraDeviceL1Fixture::cameraNameIs (const royale::String &name)
{
    royale::String cameraName;
    auto status = camera->getCameraName (cameraName);
    if (status != CameraStatus::SUCCESS)
    {
        throw TestFixtureException ("Couldn't get camera name", status);
    }
    return name == cameraName;
}

bool CameraDeviceL1Fixture::cameraNameStartsWith (const royale::String &name)
{
    royale::String cameraName;
    auto status = camera->getCameraName (cameraName);
    if (status != CameraStatus::SUCCESS)
    {
        throw TestFixtureException ("Couldn't get camera name", status);
    }
    return cameraName.find (name) != std::string::npos;
}

bool CameraDeviceL1Fixture::setMixedModeUseCase()
{
    Vector<String> useCases;
    auto status = camera->getUseCases (useCases);
    if (status != CameraStatus::SUCCESS)
    {
        throw TestFixtureException ("getUseCases failed", status);
    }
    for (const auto &useCase : useCases)
    {
        uint32_t streamCount;
        status = camera->getNumberOfStreams (useCase, streamCount);
        if (status != CameraStatus::SUCCESS)
        {
            throw TestFixtureException ("getNumberOfStreams failed", status);
        }
        if (streamCount > 1)
        {
            if (camera->setUseCase (useCase) != CameraStatus::SUCCESS)
            {
                throw TestFixtureException ("setUseCase failed", status);
            }
            return true;
        }
    }

    return false;
}
