/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <SparsePointCloudCAPI.h>
#include <royale/ISparsePointCloudListener.hpp>

class SparsePointCloudListenerCAPI : public royale::ISparsePointCloudListener
{
public:
    explicit SparsePointCloudListenerCAPI (ROYALE_SPC_DATA_CALLBACK cb);

    void onNewData (const royale::SparsePointCloud *data) override;

private:
    ROYALE_SPC_DATA_CALLBACK m_externalDataCallback;
};

class SparsePointCloudCAPIConverter
{
public:
    static void fromRoyaleData (
        const royale::SparsePointCloud *data, royale_sparse_point_cloud *spcCAPI);
};
