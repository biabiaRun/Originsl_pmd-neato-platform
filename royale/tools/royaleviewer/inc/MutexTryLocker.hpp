/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

class MutexTryLocker
{
private:
    QMutex &m_qMutex;
    bool m_locked;

public:
    MutexTryLocker (QMutex &mutex) : m_qMutex (mutex), m_locked (mutex.tryLock()) {}
    ~MutexTryLocker()
    {
        if (m_locked)
        {
            m_qMutex.unlock();
        }
    }
    bool isLocked() const
    {
        return m_locked;
    }
};
