/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <config/CoreConfigAdapter.hpp>

using namespace royale;
using namespace royale::config;

CoreConfigAdapter::CoreConfigAdapter (std::unique_ptr<CoreConfig> config)
    : m_config (std::move (config))
{
}

void CoreConfigAdapter::getLensCenterDesign (uint16_t &column, uint16_t &row) const
{
    column = m_config->lensCenterDesign.first;
    row = m_config->lensCenterDesign.second;
}

uint16_t CoreConfigAdapter::getMaxImageWidth() const
{
    return m_config->maxImageSize.first;
}

uint16_t CoreConfigAdapter::getMaxImageHeight() const
{
    return m_config->maxImageSize.second;
}

const usecase::UseCaseList &CoreConfigAdapter::getSupportedUseCases() const
{
    return m_config->useCases;
}

FrameTransmissionMode CoreConfigAdapter::getFrameTransmissionMode() const
{
    return m_config->frameTransmissionMode;
}

royale::String CoreConfigAdapter::getCameraName() const
{
    return m_config->cameraName;
}

float CoreConfigAdapter::getTemperatureLimitSoft() const
{
    return m_config->temperatureSoftLimit;
}

float CoreConfigAdapter::getTemperatureLimitHard() const
{
    return m_config->temperatureHardLimit;
}

bool CoreConfigAdapter::isAutoExposureSupported() const
{
    return m_config->isAutoExposureSupported;
}

BandwidthRequirementCategory CoreConfigAdapter::getBandwidthRequirementCategory() const
{
    return m_config->bandwidthCategory;
}
