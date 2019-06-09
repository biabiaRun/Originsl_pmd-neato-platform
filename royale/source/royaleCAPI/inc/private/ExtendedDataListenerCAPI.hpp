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

#include <ExtendedDataCAPI.h>
#include <IntermediateDataCAPI.h>
#include <royale/RawData.hpp>
#include <royale/DepthData.hpp>
#include <royale/IntermediateData.hpp>
#include <royale/IExtendedDataListener.hpp>

class ExtendedDataListenerCAPI : public royale::IExtendedDataListener
{
public:
    explicit ExtendedDataListenerCAPI (ROYALE_EXTENDED_DATA_CALLBACK cb);

    void onNewData (const royale::IExtendedData *data) override;

private:
    ROYALE_EXTENDED_DATA_CALLBACK m_externalDataCallback;
};

class ExtendedDataCAPIConverter
{
public:
    static void fromRoyaleData (
        const royale::IExtendedData *data, royale_extended_data *edCAPI);
};

class IntermediateDataCAPIConverter
{
public:
    static void fromRoyaleData (
        const royale::IntermediateData *data, royale_intermediate_data *idCAPI);
    static void freeIntermediateData (royale_intermediate_data *idCAPI);
};

class RawDataCAPIConverter
{
public:
    static void fromRoyaleData (
        const royale::RawData *data, royale_raw_data *rdCAPI);
    static void freeRawData (royale_raw_data *rdCAPI);
};
