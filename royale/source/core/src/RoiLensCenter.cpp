/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseDefinition.hpp>
#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>
#include <device/RoiLensCenter.hpp>

using namespace royale::usecase;
using namespace royale::common;
using namespace royale::device;

RoiLensCenter::RoiLensCenter (uint16_t designColumn, uint16_t designRow) :
    m_designCenterCol (designColumn),
    m_designCenterRow (designRow),
    m_lensOffsetCol (0),
    m_lensOffsetRow (0)
{

}

royale::usecase::VerificationStatus RoiLensCenter::verifySizeAndLensCombination (uint16_t sizeColumns, uint16_t sizeRows, int16_t offsetCol, int16_t offsetRow) const
{
    if ( (m_designCenterCol + offsetCol < static_cast<signed int> (sizeColumns / 2))
            || (m_designCenterRow + offsetRow < static_cast<signed int> (sizeRows / 2)))
    {
        LOG (ERROR) << "Lens offset out of bounds (too small)";
        return royale::usecase::VerificationStatus::REGION;
    }

    // The upper limits depend on the silicon, and must be checked by the software imager.
    return royale::usecase::VerificationStatus::SUCCESS;
}

void RoiLensCenter::setLensOffset (int16_t pixelColumn, int16_t pixelRow)
{
    m_lensOffsetCol = pixelColumn;
    m_lensOffsetRow = pixelRow;
}

royale::usecase::VerificationStatus RoiLensCenter::verifyUseCase (const UseCaseDefinition &useCase) const
{
    uint16_t ucdColumns, ucdRows;
    useCase.getImage (ucdColumns, ucdRows);
    return verifySizeAndLensCombination (ucdColumns, ucdRows, m_lensOffsetCol, m_lensOffsetRow);
}

void RoiLensCenter::getRoiCorner (const UseCaseDefinition &useCase,
                                  uint16_t &activeColumnOffset,
                                  uint16_t &activeRowOffset) const
{
    uint16_t ucdColumns, ucdRows;
    useCase.getImage (ucdColumns, ucdRows);

    activeColumnOffset =
        narrow_cast<uint16_t> ( (m_designCenterCol + m_lensOffsetCol) - (ucdColumns / 2));
    activeRowOffset =
        narrow_cast<uint16_t> ( (m_designCenterRow + m_lensOffsetRow) - (ucdRows / 2));
}
