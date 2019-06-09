/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <gtest/gtest.h>
#include <common/exceptions/LogicError.hpp>
#include <device/RoiLensCenter.hpp>
#include <usecase/UseCaseFourPhase.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::device;
using namespace royale::usecase;

namespace
{
    struct ColRow
    {
        uint16_t c;
        uint16_t r;
    };

    // Some examples used in all of the tests
    const ColRow sensorSize = {160, 96};
    const ColRow designCenter = {80, 48};
    /**
     * A ROI that will have its corner at (32, 28) for an imager with no offset.
     */
    const ColRow smallerImage = {96, 40};
}

TEST (TestRoiLensCenter, PositionWithoutOffset)
{
    UseCaseFourPhase usecase { 5, 30000000, { 50u, 1000u }, 1000u, 0 };

    RoiLensCenter lensCenter (designCenter.c, designCenter.r);

    ColRow roiCorner;

    // A full-size image should fit
    usecase.setImage (sensorSize.c, sensorSize.r);
    ASSERT_EQ (VerificationStatus::SUCCESS, lensCenter.verifyUseCase (usecase));
    ASSERT_NO_THROW (lensCenter.getRoiCorner (usecase, roiCorner.c, roiCorner.r));
    ASSERT_EQ (0u, roiCorner.c);
    ASSERT_EQ (0u, roiCorner.r);

    // And this smaller image should fit
    usecase.setImage (smallerImage.c, smallerImage.r);
    ASSERT_EQ (VerificationStatus::SUCCESS, lensCenter.verifyUseCase (usecase));
    ASSERT_NO_THROW (lensCenter.getRoiCorner (usecase, roiCorner.c, roiCorner.r));
    ASSERT_EQ (32u, roiCorner.c);
    ASSERT_EQ (28, roiCorner.r);
}

TEST (TestRoiLensCenter, PositionWithNegativeCOffset)
{
    UseCaseFourPhase usecase { 5, 30000000, { 50u, 1000u }, 1000u, 0 };

    RoiLensCenter lensCenter (designCenter.c, designCenter.r);
    lensCenter.setLensOffset (-1, 1);

    ColRow roiCorner;

    // A full-size image should not fit when a negative offset is added
    usecase.setImage (sensorSize.c, sensorSize.r);
    ASSERT_EQ (VerificationStatus::REGION, lensCenter.verifyUseCase (usecase));
    ASSERT_ANY_THROW (lensCenter.getRoiCorner (usecase, roiCorner.c, roiCorner.r));

    // But this smaller image should fit
    usecase.setImage (smallerImage.c, smallerImage.r);
    ASSERT_EQ (VerificationStatus::SUCCESS, lensCenter.verifyUseCase (usecase));
    ASSERT_NO_THROW (lensCenter.getRoiCorner (usecase, roiCorner.c, roiCorner.r));
    ASSERT_EQ (31u, roiCorner.c);
    ASSERT_EQ (29u, roiCorner.r);
}

TEST (TestRoiLensCenter, PositionWithNegativeROffset)
{
    UseCaseFourPhase usecase { 5, 30000000, { 50u, 1000u }, 1000u, 0 };

    RoiLensCenter lensCenter (designCenter.c, designCenter.r);
    lensCenter.setLensOffset (1, -1);

    ColRow roiCorner;

    // A full-size image should not fit when a negative offset is added
    usecase.setImage (sensorSize.c, sensorSize.r);
    ASSERT_EQ (VerificationStatus::REGION, lensCenter.verifyUseCase (usecase));
    ASSERT_ANY_THROW (lensCenter.getRoiCorner (usecase, roiCorner.c, roiCorner.r));

    // But this smaller image should fit
    usecase.setImage (smallerImage.c, smallerImage.r);
    ASSERT_EQ (VerificationStatus::SUCCESS, lensCenter.verifyUseCase (usecase));
    ASSERT_NO_THROW (lensCenter.getRoiCorner (usecase, roiCorner.c, roiCorner.r));
    ASSERT_EQ (33u, roiCorner.c);
    ASSERT_EQ (27u, roiCorner.r);
}

TEST (TestRoiLensCenter, PositionWithPositiveOffset)
{
    UseCaseFourPhase usecase { 5, 30000000, { 50u, 1000u }, 1000u, 0 };

    RoiLensCenter lensCenter (designCenter.c, designCenter.r);
    lensCenter.setLensOffset (1, 1);

    ColRow roiCorner;

    // The RoiLensCenter class does not know the upper limits of the sensor.  Therefore, while a
    // complete device that includes the software imager will reject this, the RoiLensCenter class
    // itself should accept it.
    usecase.setImage (sensorSize.c, sensorSize.r);
    ASSERT_EQ (VerificationStatus::SUCCESS, lensCenter.verifyUseCase (usecase));
    ASSERT_NO_THROW (lensCenter.getRoiCorner (usecase, roiCorner.c, roiCorner.r));
    ASSERT_EQ (1u, roiCorner.c);
    ASSERT_EQ (1u, roiCorner.r);

    // And this smaller image should fit
    usecase.setImage (smallerImage.c, smallerImage.r);
    ASSERT_EQ (VerificationStatus::SUCCESS, lensCenter.verifyUseCase (usecase));
    ASSERT_NO_THROW (lensCenter.getRoiCorner (usecase, roiCorner.c, roiCorner.r));
    ASSERT_EQ (33u, roiCorner.c);
    ASSERT_EQ (29u, roiCorner.r);
}

