/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once
#include <royale/DepthData.hpp>
#include "PixelInfoView.hpp"

class PixelInfoView;

class PixelInfoManager
{
public:
    PixelInfoManager();
    ~PixelInfoManager();
    void addPixelInfoView (std::unique_ptr<PixelInfoView> pixPtr);
    void addPositions (QPoint windowPosition, QPoint imagePosition, royale::DepthPoint p);
    void updateValues (const royale::DepthData *dataset, int imageWidth);
    void showValues();
    void wipeOut();
    void popLastAdded();
    size_t getPixelInfoSize();
    int getPixelInfoViewWidth();
    int getPixelInfoViewHeight();

    struct positionParams
    {
        positionParams (QPoint _windowPosition, QPoint _imagePosition, royale::DepthPoint _p) :
            windowPosition (_windowPosition),
            imagePosition (_imagePosition),
            p (_p)
        {}
        QPoint windowPosition;
        QPoint imagePosition;
        royale::DepthPoint p;
    };
private:
    std::vector<std::unique_ptr<PixelInfoView>> m_pixelInfos;
    std::vector<positionParams> m_positionParams;
    uint16_t m_dataWidth, m_dataHeight;
};
