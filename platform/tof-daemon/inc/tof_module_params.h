/*
 * tof_module_params.h
 *
 * Settings for the TOF Sensor are defined here.
 */

#ifndef _TOF_MODULE_PARAMS_H_
#define _TOF_MODULE_PARAMS_H_

const bool USE_FLYING_PIXEL = false;
const bool USE_STRAY_LIGHT = true;
const bool USE_ADAPTIVE_NOISE_FILTER = true;
const bool USE_VALIDATE_IMAGE = false;
const bool USE_SMOOTHING_FILTER = false;
const bool USE_SBI_FLAG = false;
const bool USE_HOLE_FILLING = false;

// Multipath interference filters
const bool USE_MPI_AVERAGE = true;
const bool USE_MPI_AMP = true;
const bool USE_MPI_DIST = true;
const bool USE_FILTER_2_FREQ = true;

const int ADAPTIVE_NOISE_FILTER_TYPE = 2;
const float NOISE_THRESHOLD = 0.07f;
const int GLOBAL_BINNING = 1;

// Exposure time in microseconds
const float AUTO_EXPOSURE_REF_VALUE = 400.0f;// AutoExposureRefValue_Float

#endif