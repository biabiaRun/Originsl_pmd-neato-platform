/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include "PixelInfoManager.hpp"

PixelInfoManager::PixelInfoManager() :
    m_dataWidth (0u),
    m_dataHeight (0u)
{
}

PixelInfoManager::~PixelInfoManager()
{
    wipeOut();
}

void PixelInfoManager::addPixelInfoView (std::unique_ptr<PixelInfoView> pixPtr)
{
    m_pixelInfos.push_back (std::move (pixPtr));
}

void PixelInfoManager::addPositions (QPoint windowPosition, QPoint imagePosition, royale::DepthPoint p)
{
    m_positionParams.push_back (positionParams (windowPosition, imagePosition, p));
}

void PixelInfoManager::wipeOut()
{
    if (!m_pixelInfos.empty())
    {
        m_pixelInfos.clear();
        m_positionParams.clear();

        m_dataWidth = 0u;
        m_dataHeight = 0u;
    }
}

void PixelInfoManager::popLastAdded()
{
    m_pixelInfos.pop_back();
    m_positionParams.pop_back();
}

size_t PixelInfoManager::getPixelInfoSize()
{
    return m_pixelInfos.size();
}

void PixelInfoManager::updateValues (const royale::DepthData *dataset, int imageWidth)
{
    if (m_dataHeight == 0u && m_dataWidth == 0u)
    {
        m_dataWidth = dataset->width;
        m_dataHeight = dataset->height;
    }

    if (m_positionParams.back().imagePosition.y() < dataset->height &&
            m_positionParams.back().imagePosition.x() < dataset->width &&
            m_dataWidth == dataset->width &&
            m_dataHeight == dataset->height)
    {
        for (std::size_t i{}; i < m_pixelInfos.size(); i++)
        {
            m_positionParams.at (i).p = dataset->points[m_positionParams.at (i).imagePosition.y() * imageWidth + m_positionParams.at (i).imagePosition.x()];
            m_pixelInfos.at (i)->updateDepthPointInfo (m_positionParams.at (i).windowPosition, m_positionParams.at (i).imagePosition, m_positionParams.at (i).p);
        }
    }
    else
    {
        wipeOut();
    }
}

void PixelInfoManager::showValues()
{
    m_pixelInfos.back()->showDepthPointInfo (m_positionParams.back().windowPosition, m_positionParams.back().imagePosition, m_positionParams.back().p);
}

int PixelInfoManager::getPixelInfoViewWidth()
{
    return m_pixelInfos.back()->width();
}

int PixelInfoManager::getPixelInfoViewHeight()
{
    return m_pixelInfos.back()->height();
}
