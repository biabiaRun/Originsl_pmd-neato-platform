/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <FlashDatav100.hpp>

using namespace royale;

FlashDataV100::FlashDataV100 (std::unique_ptr<royale::ICameraDevice> cameraDevice, QWidget *parent) :
    FlashDataBase (std::move (cameraDevice), parent)
{
    setupUi (parent);
}

FlashDataV100::~FlashDataV100()
{

}

void FlashDataV100::writeFlashData()
{
    m_cameraDevice->setCalibrationData (m_calibrationData);
    m_cameraDevice->writeCalibrationToFlash();
}

void FlashDataV100::readFlashData()
{
    Vector<Pair<String, String>> camInfos;
    m_cameraDevice->getCameraInfo (camInfos);

    for (auto curInfo : camInfos)
    {
        if (curInfo.first == "MODULE_IDENTIFIER")
        {
            lbHardwareRevision->setText (curInfo.second.c_str());
        }
        else if (curInfo.first == "MODULE_SERIAL")
        {
            lbModuleSerial->setText (curInfo.second.c_str());
        }
    }

    m_cameraDevice->getCalibrationData (m_calibrationData);
}
