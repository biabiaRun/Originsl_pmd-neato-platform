#include <royale/DepthData.hpp>
#include <gtest/gtest.h>

using namespace royale;

TEST (TestDepthData, testDataStructure)
{
    const uint32_t width  = 4;
    const uint32_t height = 4;
    const uint32_t nrPoints = 4;

    DepthData data;

    data.height = height;
    data.width = width;
    data.points.reserve (nrPoints);
    DepthPoint p1{1.0, 8.0, 9.0,  0.1f, 24};
    DepthPoint p2{2.0, 7.0, 10.0, 0.2f, 23};
    DepthPoint p3{3.0, 6.0, 11.0, 0.3f, 22};
    DepthPoint p4{4.0, 5.0, 12.0, 0.4f, 21};
    data.points.push_back (p1);
    data.points.push_back (p2);
    data.points.push_back (p3);
    data.points.push_back (p4);

    ASSERT_EQ (data.points[0].x, p1.x);
}