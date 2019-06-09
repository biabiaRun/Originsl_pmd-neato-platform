/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale.hpp>

#include <map>
#include <string>
#include <memory>


class CameraBlocker
{
public:
    // Nested types
    typedef std::map<std::string, std::unique_ptr<royale::ICameraDevice>> CameraMap;
    class Iterator
    {
    public:
        Iterator (CameraMap::iterator it);
        bool operator!= (const Iterator &rhs) const;
        bool operator== (const Iterator &rhs) const;
        Iterator operator++();
        Iterator operator*();

        std::string name() const;
        void lock();
        void unlock();
        bool locked() const;

    private:
        CameraMap::iterator it;
    };

public:
    CameraBlocker();
    ~CameraBlocker();

    Iterator begin();
    Iterator end();
    Iterator find (const std::string &n);

private:

    CameraMap cameras;
};
