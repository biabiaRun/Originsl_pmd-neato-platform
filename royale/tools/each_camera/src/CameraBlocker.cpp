/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include "CameraBlocker.hpp"


CameraBlocker::CameraBlocker()
{
    royale::CameraManager cm;
    auto cameraList = cm.getConnectedCameraList();

    for (auto c : cameraList)
    {
        cameras[c.toStdString()] = cm.createCamera (c);
    }
}

CameraBlocker::~CameraBlocker() = default;

CameraBlocker::Iterator
CameraBlocker::begin()
{
    return Iterator (cameras.begin());
}

CameraBlocker::Iterator
CameraBlocker::end()
{
    return Iterator (cameras.end());
}

CameraBlocker::Iterator
CameraBlocker::find (const std::string &n)
{
    return Iterator (cameras.find (n));
}

CameraBlocker::Iterator::Iterator (CameraMap::iterator it)
    : it (it)
{
}

bool
CameraBlocker::Iterator::operator!= (const Iterator &rhs) const
{
    return it != rhs.it;
}

bool
CameraBlocker::Iterator::operator== (const Iterator &rhs) const
{
    return it == rhs.it;
}

CameraBlocker::Iterator
CameraBlocker::Iterator::operator++()
{
    return Iterator (++it);
}

CameraBlocker::Iterator
CameraBlocker::Iterator::operator*()
{
    return *this;
}

std::string
CameraBlocker::Iterator::name() const
{
    return it->first;
}

void
CameraBlocker::Iterator::lock()
{
    assert (!locked());

    royale::CameraManager cm;
    cm.getConnectedCameraList();
    it->second = cm.createCamera (it->first);
}

void
CameraBlocker::Iterator::unlock()
{
    assert (locked());
    it->second.reset();
}

bool
CameraBlocker::Iterator::locked() const
{
    return it->second != nullptr;
}
