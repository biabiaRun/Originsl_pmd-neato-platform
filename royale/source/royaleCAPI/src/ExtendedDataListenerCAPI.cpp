/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <private/ExtendedDataListenerCAPI.hpp>
#include <private/DepthDataListenerCAPI.hpp>


ExtendedDataListenerCAPI::ExtendedDataListenerCAPI (ROYALE_EXTENDED_DATA_CALLBACK cb) :
    m_externalDataCallback (cb)
{

}

void ExtendedDataListenerCAPI::onNewData (const royale::IExtendedData *data)
{
    if (m_externalDataCallback != nullptr)
    {
        royale_extended_data edCAPI;
        ExtendedDataCAPIConverter::fromRoyaleData (data, &edCAPI);
        m_externalDataCallback (&edCAPI);

        if (edCAPI.has_depth_data)
        {
            free (edCAPI.depth_data);
        }

        if (edCAPI.has_intermediate_data)
        {
            IntermediateDataCAPIConverter::freeIntermediateData (edCAPI.intermediate_data);
        }

        if (edCAPI.has_raw_data)
        {
            RawDataCAPIConverter::freeRawData (edCAPI.raw_data);
        }
    }
}

void ExtendedDataCAPIConverter::fromRoyaleData (const royale::IExtendedData *data, royale_extended_data *edCAPI)
{
    edCAPI->has_depth_data = data->hasDepthData();
    edCAPI->has_intermediate_data = data->hasIntermediateData();
    edCAPI->has_raw_data = data->hasRawData();

    if (edCAPI->has_depth_data)
    {
        edCAPI->depth_data = (royale_depth_data *) malloc (sizeof (royale_depth_data));
        DepthDataCAPIConverter::fromRoyaleData (data->getDepthData(), (royale_depth_data *) edCAPI->depth_data);
    }
    if (edCAPI->has_intermediate_data)
    {
        edCAPI->intermediate_data = (royale_intermediate_data *) malloc (sizeof (royale_intermediate_data));
        IntermediateDataCAPIConverter::fromRoyaleData (data->getIntermediateData(), edCAPI->intermediate_data);
    }
    if (edCAPI->has_raw_data)
    {
        edCAPI->raw_data = (royale_raw_data *) malloc (sizeof (royale_raw_data));
        RawDataCAPIConverter::fromRoyaleData (data->getRawData(), edCAPI->raw_data);
    }
}

void IntermediateDataCAPIConverter::fromRoyaleData (
    const royale::IntermediateData *data, royale_intermediate_data *idCAPI)
{
    idCAPI->version = data->version;
    idCAPI->timestamp = data->timeStamp.count();
    idCAPI->stream_id = data->streamId;
    idCAPI->width = data->width;
    idCAPI->height = data->height;
    idCAPI->nr_exposure_times = (uint32_t) data->exposureTimes.size();
    idCAPI->exposure_times = (uint32_t *) &data->exposureTimes[0];
    idCAPI->nr_points = (uint32_t) data->width * data->height;

    idCAPI->points = (royale_intermediate_point *) &data->points[0];

    idCAPI->nr_mod_frequencies = (uint32_t) data->modulationFrequencies.size();
    idCAPI->modulation_frequencies = (uint32_t *) &data->modulationFrequencies[0];
    idCAPI->nr_frequencies = data->numFrequencies;
}

void IntermediateDataCAPIConverter::freeIntermediateData (royale_intermediate_data *idCAPI)
{
    free (idCAPI);
}

void RawDataCAPIConverter::fromRoyaleData (
    const royale::RawData *data, royale_raw_data *rdCAPI)
{
    rdCAPI->timestamp = data->timeStamp.count();
    rdCAPI->stream_id = data->streamId;
    rdCAPI->width = data->width;
    rdCAPI->height = data->height;
    rdCAPI->nr_raw_frames = (uint16_t) data->rawData.size();
    rdCAPI->nr_data_per_raw_frame = (uint32_t) data->width * data->height;

    rdCAPI->raw_data = (uint16_t **) malloc (sizeof (uint16_t *) * rdCAPI->nr_raw_frames);
    for (int i = 0; i < rdCAPI->nr_raw_frames; i++)
    {
        rdCAPI->raw_data[i] = (uint16_t *) &data->rawData[i][0];
    }

    rdCAPI->nr_mod_frequencies = (uint32_t) data->modulationFrequencies.size();
    rdCAPI->modulation_frequencies = (uint32_t *) &data->modulationFrequencies[0];
    rdCAPI->nr_exposure_times = (uint32_t) data->exposureTimes.size();
    rdCAPI->exposure_times = (uint32_t *) &data->exposureTimes[0];
    rdCAPI->illumination_temperature = data->illuminationTemperature;
    rdCAPI->nr_phase_angles = (uint32_t) data->phaseAngles.size();
    rdCAPI->phase_angles = (uint16_t *) &data->phaseAngles[0];
    rdCAPI->nr_illumination_enabled = (uint32_t) data->illuminationEnabled.size();
    rdCAPI->illumination_enabled = (uint8_t *) &data->illuminationEnabled[0];
}

void RawDataCAPIConverter::freeRawData (royale_raw_data *rdCAPI)
{
    free (rdCAPI->raw_data);
    free (rdCAPI);
}
