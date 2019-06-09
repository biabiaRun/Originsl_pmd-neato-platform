/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#pragma once

namespace royale
{
    namespace common
    {
        class WeightedAverage
        {
        public:
            explicit WeightedAverage (float alpha = 0.03f);
            float calc (float value);

        private:
            float mAverage;  //!< current average value
            float mAlpha;    //!< weight of the actual temperature value
            bool mFirst;     //!< flag if calc is called for the first time
        };
    }
}
