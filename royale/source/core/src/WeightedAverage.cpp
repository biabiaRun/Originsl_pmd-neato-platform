/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <common/WeightedAverage.hpp>

using namespace royale::common;

WeightedAverage::WeightedAverage (float alpha) :
    mAverage (0.0f), mAlpha (alpha), mFirst (true)
{

}


float WeightedAverage::calc (float value)
{
    if (mFirst)
    {
        mAverage = value;
        mFirst = false;
    }
    else
    {
        mAverage = mAlpha * value + (1 - mAlpha) * mAverage;
    }
    return mAverage;
}
