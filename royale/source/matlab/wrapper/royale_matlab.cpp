#include "mex.h"
#include <royale.hpp>
#include <royale/IReplay.hpp>
#include <common/RoyaleLogger.hpp>

#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <vector>
#include <unordered_map>
#include <stdint.h>
#include <string>
#include <sstream>
#include <algorithm>

namespace royale_matlab
{
    static uint32_t g_matlabTimeout = 3000;

#if !defined(NDEBUG)

    std::thread::id matlabThreadId;

    // redirect stdout to Matlab command window
    class mystream : public std::streambuf
    {
    protected:
        virtual std::streamsize xsputn (const char *s, std::streamsize n)
        {
            auto thisId = std::this_thread::get_id();
            if (thisId != matlabThreadId)
            {
                return n;
            }
            mexPrintf ("%.*s", n, s);
            return n;
        }
        virtual int overflow (int c = EOF)
        {
            auto thisId = std::this_thread::get_id();
            if (thisId != matlabThreadId)
            {
                return 1;
            }
            if (c != EOF)
            {
                mexPrintf ("%.1s", &c);
            }
            return 1;
        }
    };
    class scoped_redirect_cout
    {
    public:
        scoped_redirect_cout ()
        {
            matlabThreadId = std::this_thread::get_id();
            old_buf = std::cout.rdbuf ();
            std::cout.rdbuf (&mout);
        }
        ~scoped_redirect_cout ()
        {
            std::cout.rdbuf (old_buf);
        }
    private:
        mystream mout;
        std::streambuf *old_buf;
    };
    scoped_redirect_cout mycout_redirect;
#endif

    // Matlab helper functions
    void GetMatlabData (const mxArray *m_Data, int16_t &value)
    {
        if (m_Data == 0)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data == 0");
        }
        size_t len = mxGetNumberOfElements (m_Data);
        if (len != 1)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data != 1");
        }
        mxClassID category = mxGetClassID (m_Data);
        if (category == mxINT16_CLASS)
        {
            value = *reinterpret_cast <int16_t *> (mxGetData (m_Data));
        }
        else
        {
            mexErrMsgTxt ("GetMatlabData: Expected mxINT16_CLASS!");
        }
    }
    void GetMatlabData (const mxArray *m_Data, int32_t &value)
    {
        if (m_Data == 0)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data == 0");
        }
        size_t len = mxGetNumberOfElements (m_Data);
        if (len != 1)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data != 1");
        }
        mxClassID category = mxGetClassID (m_Data);
        if (category == mxINT32_CLASS)
        {
            value = *reinterpret_cast <int32_t *> (mxGetData (m_Data));
        }
        else
        {
            mexErrMsgTxt ("GetMatlabData: Expected mxINT32_CLASS!");
        }
    }
    void GetMatlabData (const mxArray *m_Data, uint16_t &value)
    {
        if (m_Data == 0)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data == 0");
        }
        size_t len = mxGetNumberOfElements (m_Data);
        if (len != 1)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data != 1");
        }
        mxClassID category = mxGetClassID (m_Data);
        if (category == mxUINT16_CLASS)
        {
            value = *reinterpret_cast <uint16_t *> (mxGetData (m_Data));
        }
        else
        {
            mexErrMsgTxt ("GetMatlabData: Expected mxUINT16_CLASS!");
        }
    }
    void GetMatlabData (const mxArray *m_Data, uint32_t &value)
    {
        if (m_Data == 0)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data == 0");
        }
        size_t len = mxGetNumberOfElements (m_Data);
        if (len != 1)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data != 1");
        }
        mxClassID category = mxGetClassID (m_Data);
        if (category == mxUINT32_CLASS)
        {
            value = *reinterpret_cast <uint32_t *> (mxGetData (m_Data));
        }
        else
        {
            mexErrMsgTxt ("GetMatlabData: Expected mxUINT32_CLASS!");
        }
    }
    void GetMatlabData (const mxArray *m_Data, uint64_t &value)
    {
        if (m_Data == 0)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data == 0");
        }
        size_t len = mxGetNumberOfElements (m_Data);
        if (len != 1)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data != 1");
        }
        mxClassID category = mxGetClassID (m_Data);
        if (category == mxUINT64_CLASS)
        {
            value = *reinterpret_cast <uint64_t *> (mxGetData (m_Data));
        }
        else
        {
            mexErrMsgTxt ("GetMatlabData: Expected mxUINT64_CLASS!");
        }
    }
    void GetMatlabData (const mxArray *m_Data, bool &value)
    {
        if (m_Data == 0)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data == 0");
        }
        size_t len = mxGetNumberOfElements (m_Data);
        if (len != 1)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data != 1");
        }
        mxClassID category = mxGetClassID (m_Data);
        if (category == mxLOGICAL_CLASS)
        {
            value = *reinterpret_cast <bool *> (mxGetData (m_Data));
        }
        else
        {
            mexErrMsgTxt ("GetMatlabData: Expected mxLOGICAL_CLASS!");
        }
    }
    void GetMatlabData (const mxArray *m_Data, float &value)
    {
        if (m_Data == 0)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data == 0");
        }
        size_t len = mxGetNumberOfElements (m_Data);
        if (len != 1)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data != 1");
        }
        mxClassID category = mxGetClassID (m_Data);
        if (category == mxSINGLE_CLASS)
        {
            value = *reinterpret_cast <float *> (mxGetData (m_Data));
        }
        else
        {
            mexErrMsgTxt ("GetMatlabData: Expected mxSINGLE_CLASS!");
        }
    }

    void GetMatlabData (const mxArray *m_Data, double &value)
    {
        if (m_Data == 0)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data == 0");
        }
        size_t len = mxGetNumberOfElements (m_Data);
        if (len != 1)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data != 1");
        }
        mxClassID category = mxGetClassID (m_Data);
        if (category == mxDOUBLE_CLASS)
        {
            value = *reinterpret_cast <double *> (mxGetData (m_Data));
        }
        else
        {
            mexErrMsgTxt ("GetMatlabData: Expected mxDOUBLE_CLASS!");
        }
    }

    std::string GetStringFromMatlab (const mxArray *Matlab_String)
    {
        if (Matlab_String == 0)
        {
            mexErrMsgTxt ("GetStringFromMatlab: Matlab_String == 0");
        }
        if (mxGetClassID (Matlab_String) != mxCHAR_CLASS)
        {
            if ( (Matlab_String != 0) && (mxGetNumberOfElements (Matlab_String) == 0))
            {
                return std::string ();
            }
            mexErrMsgTxt ("GetStringFromMatlab: Expected mxCHAR_CLASS!");
        }
        size_t len = mxGetNumberOfElements (Matlab_String) + 1;

        char *Dst = new char[len];
        if (mxGetString (Matlab_String, Dst, (mwSize) len) != 0)
        {
            delete[] Dst;
            Dst = NULL;
            mexErrMsgTxt ("GetStringFromMatlab: Error reading from mxCHAR_CLASS");
        }
        std::string RetVal = std::string (Dst);
        delete[] Dst;
        return RetVal;
    }
    void GetMatlabData (const mxArray *m_Data, std::string &value)
    {
        value = GetStringFromMatlab (m_Data);
    }

    void GetMatlabData (const mxArray *m_Data, std::vector <uint32_t> &Data)
    {
        if (m_Data == 0)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data == 0");
        }
        size_t len = mxGetNumberOfElements (m_Data);
        if (len == 0)
        {
            Data.clear ();
        }
        Data.resize (len);
        mxClassID category = mxGetClassID (m_Data);
        if (category == mxUINT32_CLASS)
        {
            uint32_t *data = reinterpret_cast <uint32_t *> (mxGetData (m_Data));
            std::copy (data, data + len, Data.begin ());
        }
        else
        {
            mexErrMsgTxt ("Expected mxUINT32_CLASS!");
        }
    }

    void GetMatlabData (const mxArray *m_Data, std::vector <uint8_t> &Data)
    {
        if (m_Data == 0)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data == 0");
        }
        size_t len = mxGetNumberOfElements (m_Data);
        if (len == 0)
        {
            Data.clear();
        }
        Data.resize (len);
        mxClassID category = mxGetClassID (m_Data);
        if (category == mxUINT8_CLASS)
        {
            uint8_t *data = reinterpret_cast <uint8_t *> (mxGetData (m_Data));
            std::copy (data, data + len, Data.begin());
        }
        else
        {
            mexErrMsgTxt ("Expected mxUINT8_CLASS!");
        }
    }

    void GetMatlabData (const mxArray *m_Data, std::vector <double> &Data)
    {
        if (m_Data == 0)
        {
            mexErrMsgTxt ("GetMatlabData: m_Data == 0");
        }
        size_t len = mxGetNumberOfElements (m_Data);
        if (len == 0)
        {
            Data.clear();
        }
        Data.resize (len);
        mxClassID category = mxGetClassID (m_Data);
        if (category == mxSINGLE_CLASS)
        {
            float *data = reinterpret_cast <float *> (mxGetData (m_Data));
            std::vector <float> temp_data (len);
            std::copy (data, data + len, temp_data.begin());
            Data.assign (temp_data.begin(), temp_data.end());
        }
        else if (category == mxDOUBLE_CLASS)
        {
            double *data = reinterpret_cast <double *> (mxGetData (m_Data));
            std::copy (data, data + len, Data.begin());
        }
        else if (category == mxUINT32_CLASS)
        {
            std::vector <uint32_t> temp_data (len);
            uint32_t *data = reinterpret_cast <uint32_t *> (mxGetData (m_Data));
            std::copy (data, data + len, temp_data.begin());
            Data.assign (temp_data.begin(), temp_data.end());
        }
        else
        {
            mexErrMsgTxt ("GetFloatsFromMatlab: Expected mxSINGLE_CLASS or mxDOUBLE_CLASS!");
        }
    }

    mxArray *CreateMatlabData (const float &value)
    {
        mxArray *m_value = mxCreateNumericMatrix (1, 1, mxSINGLE_CLASS, mxREAL);
        *reinterpret_cast <float *> (mxGetData (m_value)) = value;
        return m_value;
    }
    mxArray *CreateMatlabData (const uint64_t &value)
    {
        mxArray *m_value = mxCreateNumericMatrix (1, 1, mxUINT64_CLASS, mxREAL);
        *reinterpret_cast <uint64_t *> (mxGetData (m_value)) = value;
        return m_value;
    }
    mxArray *CreateMatlabData (const uint32_t &value)
    {
        mxArray *m_value = mxCreateNumericMatrix (1, 1, mxUINT32_CLASS, mxREAL);
        *reinterpret_cast <uint32_t *> (mxGetData (m_value)) = value;
        return m_value;
    }
    mxArray *CreateMatlabData (const int32_t &value)
    {
        mxArray *m_value = mxCreateNumericMatrix (1, 1, mxINT32_CLASS, mxREAL);
        *reinterpret_cast <int32_t *> (mxGetData (m_value)) = value;
        return m_value;
    }
    mxArray *CreateMatlabData (const uint16_t &value)
    {
        mxArray *m_value = mxCreateNumericMatrix (1, 1, mxUINT16_CLASS, mxREAL);
        *reinterpret_cast <uint16_t *> (mxGetData (m_value)) = value;
        return m_value;
    }
    mxArray *CreateMatlabData (const bool &value)
    {
        mxArray *m_value = mxCreateNumericMatrix (1, 1, mxLOGICAL_CLASS, mxREAL);
        *reinterpret_cast <bool *> (mxGetData (m_value)) = value;
        return m_value;
    }
    mxArray *CreateMatlabData (const royale::String &value)
    {
        mxArray *m_value = mxCreateString (value.c_str());
        return m_value;
    }
    mxArray *CreateMatlabData (const royale::Vector<royale::String> &value)
    {
        mxArray *m_value = mxCreateCellMatrix (value.size (), 1);
        size_t c = 0;
        for (auto &s : value)
        {
            mxSetCell (m_value, c++, mxCreateString (s.c_str ()));
        }
        return m_value;
    }
    mxArray *CreateMatlabData (const royale::Vector<royale::Pair<royale::String, royale::String>> &value)
    {
        mxArray *m_value = mxCreateCellMatrix (value.size (), 2);
        size_t c = 0;
        for (auto &p : value)
        {
            mxSetCell (m_value, c++, mxCreateString (p.first.c_str ()));
            mxSetCell (m_value, c++, mxCreateString (p.second.c_str ()));
        }
        return m_value;
    }
    mxArray *CreateMatlabData (const royale::Pair<uint32_t, uint32_t> &value)
    {
        mxArray *m_values = mxCreateNumericMatrix (1, 2, mxUINT32_CLASS, mxREAL);
        uint32_t *dst = reinterpret_cast <uint32_t *> (mxGetData (m_values));
        std::memcpy (&dst[0], &value.first, sizeof (uint32_t));
        std::memcpy (&dst[1], &value.second, sizeof (uint32_t));
        return m_values;
    }
    mxArray *CreateMatlabData (const royale::Vector <uint8_t> &values)
    {
        mxArray *m_values = mxCreateNumericMatrix (1, values.size (), mxUINT8_CLASS, mxREAL);
        uint8_t *dst = reinterpret_cast <uint8_t *> (mxGetData (m_values));
        std::memcpy (dst, values.data(), sizeof (uint8_t) * values.size());
        return m_values;
    }
    mxArray *CreateMatlabData (const royale::Vector <uint16_t> &values)
    {
        mxArray *m_values = mxCreateNumericMatrix (1, values.size (), mxUINT16_CLASS, mxREAL);
        uint16_t *dst = reinterpret_cast <uint16_t *> (mxGetData (m_values));
        std::memcpy (dst, values.data(), sizeof (uint16_t) * values.size());
        return m_values;
    }
    mxArray *CreateMatlabData (const royale::Vector <uint32_t> &values)
    {
        mxArray *m_values = mxCreateNumericMatrix (1, values.size(), mxUINT32_CLASS, mxREAL);
        uint32_t *dst = reinterpret_cast <uint32_t *> (mxGetData (m_values));
        std::memcpy (dst, values.data(), sizeof (uint32_t) * values.size());
        return m_values;
    }
    mxArray *CreateMatlabData (const royale::Vector <uint64_t> &values)
    {
        mxArray *m_values = mxCreateNumericMatrix (1, values.size(), mxUINT64_CLASS, mxREAL);
        uint64_t *dst = reinterpret_cast <uint64_t *> (mxGetData (m_values));
        std::memcpy (dst, values.data (), sizeof (uint64_t) * values.size ());
        return m_values;
    }

    void UsageErr (const std::string &msg)
    {
        std::ostringstream sbuffer;
        sbuffer << "Usage error: " << msg << std::endl;
        mexErrMsgTxt (sbuffer.str().c_str());
    }

    mxArray *CreateMatlabData (const royale::DepthData *buf)
    {
        static const char *fieldnames[] = { "version", "timeStamp", "streamId", "x", "y", "z", "grayValue", "noise", "depthConfidence", "exposureTimes" };
        static const int nfields = static_cast <int> (std::distance (std::begin (fieldnames), std::end (fieldnames)));
        mxArray *m_data;
        m_data = mxCreateStructMatrix (1, 1, nfields, fieldnames);

        mxSetField (m_data, 0, "version", CreateMatlabData (static_cast <uint32_t> (buf->version)));
        mxSetField (m_data, 0, "timeStamp", CreateMatlabData (static_cast <uint64_t> (buf->timeStamp.count())));
        mxSetField (m_data, 0, "streamId", CreateMatlabData (static_cast<uint16_t> (buf->streamId)));

        if (buf->points.size() != static_cast<size_t> (buf->height * buf->width))
        {
            mexErrMsgTxt ("unexpected number of points!");
        }

        mxArray *m_x = mxCreateNumericMatrix (buf->height, buf->width, mxSINGLE_CLASS, mxREAL);
        mxArray *m_y = mxCreateNumericMatrix (buf->height, buf->width, mxSINGLE_CLASS, mxREAL);
        mxArray *m_z = mxCreateNumericMatrix (buf->height, buf->width, mxSINGLE_CLASS, mxREAL);
        mxArray *m_grayValue = mxCreateNumericMatrix (buf->height, buf->width, mxUINT16_CLASS, mxREAL);
        mxArray *m_noise = mxCreateNumericMatrix (buf->height, buf->width, mxSINGLE_CLASS, mxREAL);
        mxArray *m_depthConfidence = mxCreateNumericMatrix (buf->height, buf->width, mxUINT8_CLASS, mxREAL);


        float *x_ptr = reinterpret_cast <float *> (mxGetData (m_x));
        float *y_ptr = reinterpret_cast <float *> (mxGetData (m_y));
        float *z_ptr = reinterpret_cast <float *> (mxGetData (m_z));
        uint16_t *grayValue_ptr = reinterpret_cast <uint16_t *> (mxGetData (m_grayValue));
        float *noise_ptr = reinterpret_cast <float *> (mxGetData (m_noise));
        uint8_t *depthConfidence_ptr = reinterpret_cast <uint8_t *> (mxGetData (m_depthConfidence));

        // Matlab vs C++: row major vs. col major
        for (uint16_t col = 0; col < buf->width; col++)
        {
            for (uint16_t row = 0; row < buf->height; row++)
            {
                const royale::DepthPoint &p = buf->points[buf->width * row + col];
                *x_ptr++ = p.x;
                *y_ptr++ = p.y;
                *z_ptr++ = p.z;
                *grayValue_ptr++ = p.grayValue;
                *noise_ptr++ = p.noise;
                *depthConfidence_ptr++ = p.depthConfidence;
            }
        }
        mxSetField (m_data, 0, "x", m_x);
        mxSetField (m_data, 0, "y", m_y);
        mxSetField (m_data, 0, "z", m_z);
        mxSetField (m_data, 0, "grayValue", m_grayValue);
        mxSetField (m_data, 0, "noise", m_noise);
        mxSetField (m_data, 0, "depthConfidence", m_depthConfidence);
        mxSetField (m_data, 0, "exposureTimes", CreateMatlabData (buf->exposureTimes));

        return m_data;
    }
    mxArray *CreateMatlabData (const royale::IntermediateData *buf)
    {
        static const char *fieldnames[] = { "version", "timeStamp", "streamId", "distance", "amplitude", "intensity", "flags", "modulationFrequencies", "exposureTimes", "numFrequencies" };
        static const int nfields = static_cast <int> (std::distance (std::begin (fieldnames), std::end (fieldnames)));
        mxArray *m_data;
        m_data = mxCreateStructMatrix (1, 1, nfields, fieldnames);
        mxSetField (m_data, 0, "version", CreateMatlabData (static_cast <uint32_t> (buf->version)));
        mxSetField (m_data, 0, "timeStamp", CreateMatlabData (static_cast <uint64_t> (buf->timeStamp.count())));
        mxSetField (m_data, 0, "streamId", CreateMatlabData (static_cast<uint64_t> (buf->streamId)));

        mxArray *m_distanceCell = mxCreateCellMatrix (1, 1);
        mxArray *m_amplitudeCell = mxCreateCellMatrix (1, 1);
        mxArray *m_intensityCell = mxCreateCellMatrix (1, 1);
        mxArray *m_flagsCell = mxCreateCellMatrix (1, 1);
        mxSetField (m_data, 0, "distance", m_distanceCell);
        mxSetField (m_data, 0, "amplitude", m_amplitudeCell);
        mxSetField (m_data, 0, "intensity", m_intensityCell);
        mxSetField (m_data, 0, "flags", m_flagsCell);

        const royale::Vector<royale::IntermediatePoint> &Seq = buf->points;
        if (Seq.size() != static_cast<size_t> (buf->height * buf->width))
        {
            mexErrMsgTxt ("unexpected number of points!");
        }
        mxArray *m_distance = mxCreateNumericMatrix (buf->height, buf->width, mxSINGLE_CLASS, mxREAL);
        mxArray *m_amplitude = mxCreateNumericMatrix (buf->height, buf->width, mxSINGLE_CLASS, mxREAL);
        mxArray *m_intensity = mxCreateNumericMatrix (buf->height, buf->width, mxSINGLE_CLASS, mxREAL);
        mxArray *m_flags = mxCreateNumericMatrix (buf->height, buf->width, mxUINT32_CLASS, mxREAL);
        mxSetCell (m_distanceCell, 0, m_distance);
        mxSetCell (m_amplitudeCell, 0, m_amplitude);
        mxSetCell (m_intensityCell, 0, m_intensity);
        mxSetCell (m_flagsCell, 0, m_flags);

        float *distance_ptr = reinterpret_cast <float *> (mxGetData (m_distance));
        float *amplitude_ptr = reinterpret_cast <float *> (mxGetData (m_amplitude));
        float *intensity_ptr = reinterpret_cast <float *> (mxGetData (m_intensity));
        uint32_t *flags_ptr = reinterpret_cast <uint32_t *> (mxGetData (m_flags));
        // Matlab vs C++: row major vs. col major
        for (uint16_t col = 0; col < buf->width; col++)
        {
            for (uint16_t row = 0; row < buf->height; row++)
            {
                const royale::IntermediatePoint &p = Seq[buf->width * row + col];
                *distance_ptr++ = p.distance;
                *amplitude_ptr++ = p.amplitude;
                *intensity_ptr++ = p.intensity;
                *flags_ptr++ = p.flags;
            }
        }

        mxSetField (m_data, 0, "modulationFrequencies", CreateMatlabData (buf->modulationFrequencies));
        mxSetField (m_data, 0, "exposureTimes", CreateMatlabData (buf->exposureTimes));
        mxSetField (m_data, 0, "numFrequencies", CreateMatlabData (buf->numFrequencies));
        return m_data;
    }
    mxArray *CreateMatlabData (const royale::RawData *buf, const std::vector<std::vector<uint16_t>> &actualRawData)
    {
        static const char *fieldnames[] = { "streamId", "timeStamp", "rawData", "exposureGroupNames", "rawFrameCount", "modulationFrequencies", "exposureTimes",
                                            "illuminationTemperature", "phaseAngles", "illuminationEnabled"
                                          };
        static const int nfields = static_cast <int> (std::distance (std::begin (fieldnames), std::end (fieldnames)));
        mxArray *m_data;
        m_data = mxCreateStructMatrix (1, 1, nfields, fieldnames);

        mxSetField (m_data, 0, "streamId", CreateMatlabData (static_cast<uint64_t> (buf->streamId)));
        mxSetField (m_data, 0, "timeStamp", CreateMatlabData (static_cast <uint64_t> (buf->timeStamp.count())));

        mxArray *m_rawData = mxCreateCellMatrix (1, buf->rawData.size());
        size_t i = 0;
        //for (auto& rawFrame : buf->rawData)
        for (auto &rawFrame : actualRawData)
        {
            mxArray *m_rawFrame = mxCreateNumericMatrix (buf->height, buf->width, mxUINT16_CLASS, mxREAL);
            uint16_t *dst = reinterpret_cast <uint16_t *> (mxGetData (m_rawFrame));
            mxSetCell (m_rawData, i++, m_rawFrame);

            // Matlab vs C++: row major vs. col major
            for (uint16_t col = 0; col < buf->width; col++)
            {
                for (uint16_t row = 0; row < buf->height; row++)
                {
                    *dst++ = rawFrame[buf->width * row + col];
                }
            }
        }
        mxSetField (m_data, 0, "rawData", m_rawData);
        mxSetField (m_data, 0, "exposureGroupNames", CreateMatlabData (buf->exposureGroupNames));
        mxSetField (m_data, 0, "rawFrameCount", CreateMatlabData (buf->rawFrameCount));
        mxSetField (m_data, 0, "modulationFrequencies", CreateMatlabData (buf->modulationFrequencies));
        mxSetField (m_data, 0, "exposureTimes", CreateMatlabData (buf->exposureTimes));
        mxSetField (m_data, 0, "illuminationTemperature", CreateMatlabData (buf->illuminationTemperature));
        mxSetField (m_data, 0, "phaseAngles", CreateMatlabData (buf->phaseAngles));
        mxSetField (m_data, 0, "illuminationEnabled", CreateMatlabData (buf->illuminationEnabled));
        return m_data;
    }


    std::string CameraStatusName (royale::CameraStatus status)
    {
        switch (status)
        {
            case royale::CameraStatus::SUCCESS:
                return std::string ("SUCCESS"); //!< Indicates that there isn't an error

            case royale::CameraStatus::RUNTIME_ERROR:
                return std::string ("RUNTIME_ERROR"); //!< Something unexpected happened
            case royale::CameraStatus::DISCONNECTED:
                return std::string ("DISCONNECTED"); //!< Camera device is disconnected
            case royale::CameraStatus::INVALID_VALUE:
                return std::string ("INVALID_VALUE"); //!< The value provided is invalid
            case royale::CameraStatus::TIMEOUT:
                return std::string ("TIMEOUT"); //!< The connection got a timeout

            case royale::CameraStatus::LOGIC_ERROR:
                return std::string ("LOGIC_ERROR"); //!< This does not make any sense here
            case royale::CameraStatus::NOT_IMPLEMENTED:
                return std::string ("NOT_IMPLEMENTED"); //!< This feature is not implemented yet
            case royale::CameraStatus::OUT_OF_BOUNDS:
                return std::string ("OUT_OF_BOUNDS"); //!< Setting/parameter is out of specified range

            case royale::CameraStatus::RESOURCE_ERROR:
                return std::string ("RESOURCE_ERROR"); //!< Cannot access resource
            case royale::CameraStatus::FILE_NOT_FOUND:
                return std::string ("FILE_NOT_FOUND"); //!< Specified file was not found
            case royale::CameraStatus::COULD_NOT_OPEN:
                return std::string ("COULD_NOT_OPEN"); //!< Cannot open file
            case royale::CameraStatus::DATA_NOT_FOUND:
                return std::string ("DATA_NOT_FOUND"); //!< No data available where expected
            case royale::CameraStatus::DEVICE_IS_BUSY:
                return std::string ("DEVICE_IS_BUSY"); //!< Another action is in progress
            case royale::CameraStatus::WRONG_DATA_FORMAT_FOUND:
                return std::string ("WRONG_DATA_FORMAT_FOUND"); //!< A resource was expected to be in one data format, but was in a different (recognisable) format

            case royale::CameraStatus::USECASE_NOT_SUPPORTED:
                return std::string ("USECASE_NOT_SUPPORTED"); //!< This use case mode is not supported
            case royale::CameraStatus::FRAMERATE_NOT_SUPPORTED:
                return std::string ("FRAMERATE_NOT_SUPPORTED"); //!< The specified frame rate is not supported
            case royale::CameraStatus::EXPOSURE_TIME_NOT_SUPPORTED:
                return std::string ("EXPOSURE_TIME_NOT_SUPPORTED"); //!< The exposure time is not supported
            case royale::CameraStatus::DEVICE_NOT_INITIALIZED:
                return std::string ("DEVICE_NOT_INITIALIZED"); //!< The device seems to be uninitialized
            case royale::CameraStatus::CALIBRATION_DATA_ERROR:
                return std::string ("CALIBRATION_DATA_ERROR"); //!< The calibration data is not readable
            case royale::CameraStatus::INSUFFICIENT_PRIVILEGES:
                return std::string ("INSUFFICIENT_PRIVILEGES"); //!< The camera access level does not allow to call this operation
            case royale::CameraStatus::DEVICE_ALREADY_INITIALIZED:
                return std::string ("DEVICE_ALREADY_INITIALIZED"); //!< The camera was already initialized
            case royale::CameraStatus::EXPOSURE_MODE_INVALID:
                return std::string ("EXPOSURE_MODE_INVALID"); //!< The current set exposure mode does not support this operation
            case royale::CameraStatus::NO_CALIBRATION_DATA:
                return std::string ("NO_CALIBRATION_DATA"); //!< The method cannot be called since no calibration data is available
            case royale::CameraStatus::INSUFFICIENT_BANDWIDTH:
                return std::string ("INSUFFICIENT_BANDWIDTH"); //!< The interface to the camera module does not provide a sufficient bandwidth
            case royale::CameraStatus::DUTYCYCLE_NOT_SUPPORTED:
                return std::string ("DUTYCYCLE_NOT_SUPPORTED"); //!< The duty cycle is not supported
            case royale::CameraStatus::SPECTRE_NOT_INITIALIZED:
                return std::string ("SPECTRE_NOT_INITIALIZED"); //!< Spectre was not initialized properly


            case royale::CameraStatus::FSM_INVALID_TRANSITION:
                return std::string ("FSM_INVALID_TRANSITION"); //!< Camera module state machine does not support current transition

            case royale::CameraStatus::UNKNOWN:
                return std::string ("UNKNOWN"); //!< Catch-all failure
            default:
                return std::string ("???");
        }
    }
    void verifyCameraStatus (const royale::CameraStatus &status, const std::string &msg)
    {
        if (status != royale::CameraStatus::SUCCESS)
        {
            std::ostringstream sbuffer;
            sbuffer << "Camera error: \"" << msg << "\". " << "Error code: " << static_cast<int> (status)
                    << " (" << CameraStatusName (status) << ")";
            mexErrMsgTxt (sbuffer.str().c_str());
        }
    }

    class myExtendedData
    {
    public:
        bool m_hasDepthData, m_hasIntermediateData, m_hasRawData;
        royale::DepthData m_depthData;
        royale::IntermediateData m_intermediateData;
        royale::RawData m_rawData;
        std::vector<std::vector <uint16_t>> m_actualRawData;
        myExtendedData()
            : m_hasDepthData (false),
              m_hasIntermediateData (false),
              m_hasRawData (false),
              m_depthData(),
              m_intermediateData(),
              m_rawData()
        {   }
    };

    class MyExtendedDataListener : public royale::IExtendedDataListener
    {
    private:
        std::atomic<bool> m_dataRequest, m_dataAvailable, m_abort;
        std::condition_variable m_cv;
        std::mutex m_mutex;
        myExtendedData m_buf;
    public:
        std::atomic<bool> m_freerun;
        MyExtendedDataListener() : m_dataRequest (false),
            m_dataAvailable (false),
            m_abort (false),
            m_freerun (true)
        {
        };
        void abort (bool value)
        {
            m_abort = value;
            m_cv.notify_all ();
        }
        void onNewData (const royale::IExtendedData *data) override
        {
            if (!m_freerun)
            {
                std::unique_lock<std::mutex> lk (m_mutex);
                m_cv.wait (lk, [&] {return (m_dataRequest || m_abort); });
            }
            if (!m_dataRequest)
            {
                return;
            }

            if (data->hasDepthData())
            {
                m_buf.m_hasDepthData = true;
                m_buf.m_depthData = *data->getDepthData();
            }
            else
            {
                m_buf.m_hasDepthData = false;
            }
            if (data->hasIntermediateData())
            {
                m_buf.m_hasIntermediateData = true;
                m_buf.m_intermediateData = *data->getIntermediateData();
            }
            else
            {
                m_buf.m_hasIntermediateData = false;
            }
            if (data->hasRawData())
            {
                m_buf.m_hasRawData = true;
                m_buf.m_rawData = *data->getRawData();

                size_t N_Pixel = m_buf.m_rawData.height * m_buf.m_rawData.width;
                m_buf.m_actualRawData.resize (m_buf.m_rawData.rawData.size());
                for (size_t i = 0; i < m_buf.m_rawData.rawData.size(); ++i)
                {
                    auto &srcFrame = m_buf.m_rawData.rawData.at (i);
                    m_buf.m_actualRawData[i].assign (srcFrame, srcFrame + N_Pixel);
                }
            }
            else
            {
                m_buf.m_hasRawData = false;
            }

            {
                std::lock_guard<std::mutex> lk (m_mutex);
                m_dataAvailable = true;
                m_dataRequest = false;
            }

            m_cv.notify_one ();
        }

        void get (int &nlhs, mxArray *plhs[])
        {
            {
                // request data
                std::lock_guard<std::mutex> lk (m_mutex);
                m_dataRequest = true;
            }

            m_cv.notify_one ();

            {
                // wait for available data
                std::unique_lock<std::mutex> lk (m_mutex);
                // set a time out
                auto TimeOut = (std::chrono::system_clock::now() + std::chrono::milliseconds (g_matlabTimeout));
                if (!m_cv.wait_until (lk, TimeOut, [&] {return (m_dataAvailable || m_abort); }))
                {
                    mexErrMsgTxt ("TimeOut in IExtendedDataListener");
                }
            }
            if (!m_dataAvailable)
            {
                if (m_abort)
                {
                    mexErrMsgTxt ("Stopped");
                }
                else
                {
                    mexErrMsgTxt ("No data available?");
                }
            }

            {
                std::lock_guard<std::mutex> lk (m_mutex);
                m_dataAvailable = false;
            }

            if (nlhs < 1)
            {
                return;
            }

            if (m_buf.m_hasDepthData)
            {
                plhs[0] = CreateMatlabData (&m_buf.m_depthData);
            }
            else
            {
                plhs[0] = mxCreateNumericMatrix (0, 0, mxDOUBLE_CLASS, mxREAL);
            }

            if (nlhs < 2)
            {
                return;
            }

            if (m_buf.m_hasIntermediateData)
            {
                plhs[1] = CreateMatlabData (&m_buf.m_intermediateData);
            }
            else
            {
                plhs[1] = mxCreateNumericMatrix (0, 0, mxDOUBLE_CLASS, mxREAL);
            }

            if (nlhs < 3)
            {
                return;
            }

            if (m_buf.m_hasRawData)
            {
                plhs[2] = CreateMatlabData (&m_buf.m_rawData, m_buf.m_actualRawData);
            }
            else
            {
                plhs[2] = mxCreateNumericMatrix (0, 0, mxDOUBLE_CLASS, mxREAL);
            }

            if (nlhs > 3)
            {
                mexErrMsgTxt ("Too many output arguments.");
            }
        }
    };

    class MyDepthDataListener : public royale::IDepthDataListener
    {
    private:
        std::atomic<bool> m_dataRequest, m_dataAvailable, m_abort;
        std::condition_variable m_cv;
        std::mutex m_mutex;
        royale::DepthData m_buf;
    public:
        std::atomic<bool> m_freerun;
        MyDepthDataListener()
            : m_dataRequest (false),
              m_dataAvailable (false),
              m_abort (false),
              m_buf(),
              m_freerun (true)
        {
        };

        void onNewData (const royale::DepthData *data) override
        {
            if (!m_freerun)
            {
                std::unique_lock<std::mutex> lk (m_mutex);
                m_cv.wait (lk, [&] {return (m_dataRequest || m_abort); });
            }
            if (!m_dataRequest)
            {
                return;
            }

            m_buf = *data;
            std::lock_guard<std::mutex> lk (m_mutex);
            m_dataAvailable = true;
            m_dataRequest = false;
            m_cv.notify_one();
        }
        void abort (bool value)
        {
            m_abort = value;
            m_cv.notify_all ();
        }
        mxArray *get ()
        {
            {
                // request data
                std::lock_guard<std::mutex> lk (m_mutex);
                m_dataRequest = true;
            }
            m_cv.notify_one ();
            {
                // wait for available data
                std::unique_lock<std::mutex> lk (m_mutex);
                // set a time out
                auto TimeOut = (std::chrono::system_clock::now() + std::chrono::milliseconds (g_matlabTimeout));
                if (!m_cv.wait_until (lk, TimeOut, [&] {return (m_dataAvailable || m_abort); }))
                {
                    mexErrMsgTxt ("TimeOut in IDepthDataListener");
                }
            }
            if (!m_dataAvailable)
            {
                if (m_abort)
                {
                    mexErrMsgTxt ("Stopped");
                }
                else
                {
                    mexErrMsgTxt ("No data available?");
                }
            }

            {
                std::lock_guard<std::mutex> lk (m_mutex);
                m_dataAvailable = false;
            }
            return CreateMatlabData (&m_buf);
        }
    };

    class MyRecordStopListener : public royale::IRecordStopListener
    {
    private:
        std::mutex m_mutex;
        std::atomic<bool> m_isRecordingValue;
    public:
        uint32_t m_framesRecorded;
        MyRecordStopListener()
            : m_isRecordingValue (false), m_framesRecorded (0) {};
        void onRecordingStopped (const uint32_t numFrames) override
        {
            std::lock_guard <std::mutex> lock (m_mutex);
            m_framesRecorded = numFrames;
            m_isRecordingValue = false;
        }
        const bool isRecording()
        {
            return m_isRecordingValue;
        }
        void start()
        {
            std::lock_guard <std::mutex> lock (m_mutex);
            m_isRecordingValue = true;
            m_framesRecorded = 0;
        }
        void abort()
        {
            std::lock_guard <std::mutex> lock (m_mutex);
            m_isRecordingValue = false;
        }
    };

    class MyPlaybackStopListener : public royale::IPlaybackStopListener
    {
    public:
        std::atomic<bool> m_isPlaying;
        MyPlaybackStopListener ()
            : m_isPlaying (false) {};
        void onPlaybackStopped () override
        {
            m_isPlaying = false;
        }
    };

    class MyCameraDevice
    {
    public:
        std::unique_ptr <royale::ICameraDevice> m_cameraDevice;
        royale::CameraAccessLevel m_accessLevel;
        MyDepthDataListener m_depthDataListener;
        MyExtendedDataListener m_extendedDataListener;
        MyRecordStopListener m_recordStopListener;
        MyPlaybackStopListener m_playbackStopListener;

        MyCameraDevice (std::unique_ptr <royale::ICameraDevice> &cameraDevice)
            : m_cameraDevice (std::move (cameraDevice)),
              m_accessLevel (royale::CameraAccessLevel::L1)
        {
            auto replay = dynamic_cast <royale::IReplay *> (m_cameraDevice.get ());
            m_isPlayBackDevice = (replay != nullptr) ? true : false;
            if (m_isPlayBackDevice)
            {
                m_depthDataListener.m_freerun = false;
                m_extendedDataListener.m_freerun = false;
            }
        }
        ~MyCameraDevice()
        {
            // make sure, everything has stopped properly
            stopCapture();
        }
        bool m_isPlayBackDevice;
        royale::IReplay *replay()
        {
            auto replay_ptr = dynamic_cast <royale::IReplay *> (this->m_cameraDevice.get());
            if (replay_ptr == nullptr)
            {
                mexErrMsgTxt ("Replay device access not available.");
            }
            return replay_ptr;
        }
        void initialize()
        {
            auto ret = m_cameraDevice->initialize();
            verifyCameraStatus (ret, "Cannot initialize the camera device.");

            // store accesslevel to know which listener is active
            ret = m_cameraDevice->getAccessLevel (m_accessLevel);
            verifyCameraStatus (ret, "Cannot retrieve access level.");
            switch (m_accessLevel)
            {
                case royale::CameraAccessLevel::L1:
                    // register a data listener
                    m_cameraDevice->registerDataListener (&m_depthDataListener);
                    break;
                case royale::CameraAccessLevel::L2:
                case royale::CameraAccessLevel::L3:
                case royale::CameraAccessLevel::L4:
                default:
                    // register an extended data listener
                    m_cameraDevice->registerDataListenerExtended (&m_extendedDataListener);
            }
            m_cameraDevice->registerRecordListener (&m_recordStopListener);
            if (m_isPlayBackDevice)
            {
                replay()->registerStopListener (&m_playbackStopListener);
            }
        }
        void startCapture()
        {
            m_depthDataListener.abort (false);
            m_extendedDataListener.abort (false);
            if (m_isPlayBackDevice)
            {
                m_playbackStopListener.m_isPlaying = true;
            }
            auto ret = m_cameraDevice->startCapture();
            if (ret != royale::CameraStatus::SUCCESS)
            {
                if (m_isPlayBackDevice)
                {
                    m_playbackStopListener.m_isPlaying = false;
                }
                verifyCameraStatus (ret, "Cannot execute startCapture");
            }
        }
        void stopCapture()
        {
            m_depthDataListener.abort (true);
            m_extendedDataListener.abort (true);
            // make sure, everything has stopped properly
            royale::CameraStatus ret;
            if (!m_isPlayBackDevice)
            {
                ret = m_cameraDevice->stopRecording();
                verifyCameraStatus (ret, "Cannot stop recording");
            }
            ret = m_cameraDevice->stopCapture();
            verifyCameraStatus (ret, "Cannot stop capturing");
        }
        void startRecording (const std::string &fileName, uint32_t numberOfFrames = 0,
                             uint32_t frameSkip = 0, uint32_t msSkip = 0)
        {
            if (m_recordStopListener.isRecording())
            {
                mexErrMsgTxt ("Recording already started.");
            }

            m_recordStopListener.start();
            auto ret = m_cameraDevice->startRecording (fileName, numberOfFrames,
                       frameSkip, msSkip);
            if (ret != royale::CameraStatus::SUCCESS)
            {
                m_recordStopListener.abort();
                verifyCameraStatus (ret, "Cannot start recording.");
            }
        }
        void stopRecording()
        {
            m_cameraDevice->stopRecording();
        }
    };

    void checkLock();
    // myhandle class: manages handles and keys for Matlab <-> C++ usage
    template <class T>
    class myhandle
    {
    private:
        std::unordered_map <uint32_t, std::unique_ptr <T> > table;
    public:
        uint32_t add (std::unique_ptr <T> &CM)
        {
            uint32_t new_key = 0;
            while (table.find (new_key) != table.end())
            {
                new_key++;
            }
            table.insert (
                std::make_pair (new_key, std::move (CM)));
            checkLock();
            return new_key;
        }
        void add (std::unique_ptr <T> &CM, mxArray *&handle)
        {
            handle = CreateMatlabData (add (CM));
        }
        uint32_t getHandle (const mxArray *obj)
        {
            mxArray *m_h = mxGetProperty (obj, 0, "h");
            if (m_h == 0)
            {
                mexErrMsgTxt ("cannot retrieve handle");
            }
            uint32_t h; // internal camera manager handle
            GetMatlabData (m_h, h);
            return h;
        }
        std::unique_ptr <T> &at (const uint32_t &h)
        {
            return table.at (h);
        }
        std::unique_ptr <T> &at (const mxArray *obj)
        {
            return table.at (getHandle (obj));
        }
        void erase (const uint32_t &h)
        {
            table.erase (h);
            checkLock();
        }
        void erase (const mxArray *obj)
        {
            erase (getHandle (obj));
        }
        const size_t size()
        {
            return table.size();
        }
        void clear()
        {
            table.clear();
            checkLock();
        }
    };

    myhandle <royale::CameraManager> CameraManagerList;
    myhandle <MyCameraDevice> CameraDeviceList;
    void clearAllHandles()
    {
        CameraManagerList.clear();
        CameraDeviceList.clear();
    }
    void checkLock()
    {
        // locked and unlocks the mex function
        // depending on active handles
        size_t handle_count = (CameraManagerList.size() + CameraDeviceList.size());
        if (handle_count == 0)
        {
            if (mexIsLocked())
            {
                mexUnlock();
            }
        }
        else
        {
            if (!mexIsLocked())
            {
                mexLock();
            }
        }
    }

    void CameraManager (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
    {
        if (nrhs < 2)
        {
            UsageErr ("wrong parameter count");
        }

        std::string command = GetStringFromMatlab (prhs[1]);
        if (!command.compare ("new"))
        {
            std::unique_ptr <royale::CameraManager> cameraManager;
            if (nrhs < 3)
            {
                cameraManager = std::unique_ptr <royale::CameraManager> (new royale::CameraManager());
            }
            else
            {
                std::string activationCode = GetStringFromMatlab (prhs[2]);
                cameraManager = std::unique_ptr <royale::CameraManager> (new royale::CameraManager (activationCode));
            }
            CameraManagerList.add (cameraManager, plhs[0]);
        }
        else if (!command.compare ("delete"))
        {
            CameraManagerList.erase (prhs[0]);
        }
        else
        {
            auto &cameraManager = CameraManagerList.at (prhs[0]);

            if (!command.compare ("createCamera"))
            {
                if (nrhs < 3 || nrhs > 4)
                {
                    UsageErr ("wrong parameter count");
                }
                std::string cameraId = GetStringFromMatlab (prhs[2]);
                std::unique_ptr <royale::ICameraDevice> cameraDevice;
                if (nrhs == 3)
                {
                    cameraDevice = cameraManager->createCamera (cameraId);
                }
                else
                {
                    uint16_t triggerInput;
                    GetMatlabData (prhs[3], triggerInput);
                    auto triggerMode = static_cast <royale::TriggerMode> (triggerInput);
                    cameraDevice = cameraManager->createCamera (cameraId, triggerMode);
                }

                if (cameraDevice == nullptr)
                {
                    mexErrMsgTxt ("fail");
                }
                auto camera = std::unique_ptr <MyCameraDevice> (new MyCameraDevice (cameraDevice));
                CameraDeviceList.add (camera, plhs[0]);
            }
            else if (!command.compare ("getConnectedCameraList"))
            {
                royale::Vector<royale::String> ConnectedCameraList = cameraManager->getConnectedCameraList();
                plhs[0] = CreateMatlabData (ConnectedCameraList);
            }
            else
            {
                UsageErr ("Command not found: " + command);
            }
        }
    }

    // has_suffix: helper function to deduce data type from processing flag name
    bool has_suffix (const std::string &str, const std::string &suffix)
    {
        return str.size() >= suffix.size() &&
               str.compare (str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    void ICameraDevice (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
    {
        if (nrhs < 2)
        {
            UsageErr ("wrong parameter count");
        }

        std::string command = GetStringFromMatlab (prhs[1]);
        if (!command.compare ("new"))
        {
            mexErrMsgTxt ("No constructor call implemented.");
        }
        else if (!command.compare ("delete"))
        {
            CameraDeviceList.erase (prhs[0]);
        }
        else
        {
            auto &camera = CameraDeviceList.at (prhs[0]);

            if (!command.compare ("initialize"))
            {
                camera->initialize();
            }
            else if (!command.compare ("getUseCases"))
            {
                royale::Vector<royale::String> modes;
                auto ret = camera->m_cameraDevice->getUseCases (modes);
                verifyCameraStatus (ret, "Cannot retrieve use cases.");
                plhs[0] = CreateMatlabData (modes);
            }
            else if (!command.compare ("getStreams"))
            {
                royale::Vector<royale::StreamId> streams;
                auto ret = camera->m_cameraDevice->getStreams (streams);

                verifyCameraStatus (ret, "Cannot retrieve streams.");

                plhs[0] = mxCreateCellMatrix (streams.size(), 1);
                for (size_t i = 0; i < streams.size(); ++i)
                {
                    auto &stream = streams.at (i);
                    mxSetCell (plhs[0], i, CreateMatlabData (stream));
                }
            }
            else if (!command.compare ("setUseCase"))
            {
                if (nrhs != 3)
                {
                    UsageErr ("wrong parameter count");
                }
                std::string m_mode;
                GetMatlabData (prhs[2], m_mode);
                auto ret = camera->m_cameraDevice->setUseCase (m_mode);
                verifyCameraStatus (ret, "Cannot set use case.");
            }
            else if (!command.compare ("getCurrentUseCase"))
            {
                royale::String mode;
                auto ret = camera->m_cameraDevice->getCurrentUseCase (mode);
                verifyCameraStatus (ret, "Cannot retrieve current use case.");
                plhs[0] = CreateMatlabData (mode);
            }
            else if (!command.compare ("getId"))
            {
                royale::String cameraId;
                auto ret = camera->m_cameraDevice->getId (cameraId);
                verifyCameraStatus (ret, "Cannot retrieve camera ID.");
                plhs[0] = mxCreateString (cameraId.c_str());
            }
            else if (!command.compare ("getCameraName"))
            {
                royale::String cameraName;
                auto ret = camera->m_cameraDevice->getCameraName (cameraName);
                verifyCameraStatus (ret, "Cannot retrieve camera name.");
                plhs[0] = CreateMatlabData (cameraName);
            }
            else if (!command.compare ("getCameraInfo"))
            {
                royale::Vector<royale::Pair<royale::String, royale::String>>  cameraInfo;
                auto ret = camera->m_cameraDevice->getCameraInfo (cameraInfo);
                verifyCameraStatus (ret, "Cannot retrieve camera info.");
                plhs[0] = CreateMatlabData (cameraInfo);
            }
            else if (!command.compare ("getMaxSensorWidth"))
            {
                uint16_t maxSensorWidth;
                auto ret = camera->m_cameraDevice->getMaxSensorWidth (maxSensorWidth);
                verifyCameraStatus (ret, "Cannot retrieve max sensor width.");
                plhs[0] = CreateMatlabData (maxSensorWidth);
            }
            else if (!command.compare ("getMaxSensorHeight"))
            {
                uint16_t maxSensorHeight;
                auto ret = camera->m_cameraDevice->getMaxSensorHeight (maxSensorHeight);
                verifyCameraStatus (ret, "Cannot retrieve max sensor height.");
                plhs[0] = CreateMatlabData (maxSensorHeight);
            }
            else if (!command.compare ("getLensParameters"))
            {
                royale::LensParameters lp;
                auto ret = camera->m_cameraDevice->getLensParameters (lp);
                verifyCameraStatus (ret, "Cannot retrieve lens parameters.");

                // initialize LensParameter structure for Matlab
                static const char *fieldnames[] = { "principalPoint", "focalLength", "distortionTangential", "distortionRadial" };
                static const char *principalPoint_fieldnames[] = { "cx", "cy" };
                static const char *focalLength_fieldnames[] = { "fx", "fy" };
                static const char *distortionTangential_fieldnames[] = { "p1", "p2" };
                static const char *distortionRadial_fieldnames[] = { "k1", "k2", "k3" };

                plhs[0] = mxCreateStructMatrix (1, 1, 4, fieldnames);
                mxArray *m_principalPoint = mxCreateStructMatrix (1, 1, 2, principalPoint_fieldnames);
                mxArray *m_focalLength = mxCreateStructMatrix (1, 1, 2, focalLength_fieldnames);
                mxArray *m_distortionTangential = mxCreateStructMatrix (1, 1, 2, distortionTangential_fieldnames);
                mxArray *m_distortionRadial = mxCreateStructMatrix (1, 1, 3, distortionRadial_fieldnames);
                mxSetField (plhs[0], 0, "principalPoint", m_principalPoint);
                mxSetField (plhs[0], 0, "focalLength", m_focalLength);
                mxSetField (plhs[0], 0, "distortionTangential", m_distortionTangential);
                mxSetField (plhs[0], 0, "distortionRadial", m_distortionRadial);

                // fill values into structure
                // cx, cy
                mxSetField (m_principalPoint, 0, "cx", CreateMatlabData (lp.principalPoint.first));
                mxSetField (m_principalPoint, 0, "cy", CreateMatlabData (lp.principalPoint.second));
                // fx, fy
                mxSetField (m_focalLength, 0, "fx", CreateMatlabData (lp.focalLength.first));
                mxSetField (m_focalLength, 0, "fy", CreateMatlabData (lp.focalLength.second));
                // p1,p2
                mxSetField (m_distortionTangential, 0, "p1", CreateMatlabData (lp.distortionTangential.first));
                mxSetField (m_distortionTangential, 0, "p2", CreateMatlabData (lp.distortionTangential.second));
                // k1,k2,k3
                mxSetField (m_distortionRadial, 0, "k1", CreateMatlabData (lp.distortionRadial.at (0)));
                mxSetField (m_distortionRadial, 0, "k2", CreateMatlabData (lp.distortionRadial.at (1)));
                mxSetField (m_distortionRadial, 0, "k3", CreateMatlabData (lp.distortionRadial.at (2)));
            }
            else if (!command.compare ("startCapture"))
            {
                camera->startCapture();
            }
            else if (!command.compare ("stopCapture"))
            {
                camera->stopCapture();
            }
            else if (!command.compare ("startRecording"))
            {
                if (nrhs < 3)
                {
                    UsageErr ("wrong parameter count");
                }

                auto fileName = GetStringFromMatlab (prhs[2]);
                if (nrhs == 3)
                {
                    camera->startRecording (fileName);
                }
                else if (nrhs == 4)
                {
                    uint32_t numberOfFrames;
                    GetMatlabData (prhs[3], numberOfFrames);
                    camera->startRecording (fileName, numberOfFrames);
                }
                else if (nrhs == 5)
                {
                    uint32_t numberOfFrames;
                    GetMatlabData (prhs[3], numberOfFrames);
                    uint32_t frameSkip;
                    GetMatlabData (prhs[4], frameSkip);
                    camera->startRecording (fileName, numberOfFrames, frameSkip);
                }
                else if (nrhs == 6)
                {
                    uint32_t numberOfFrames;
                    GetMatlabData (prhs[3], numberOfFrames);
                    uint32_t frameSkip;
                    GetMatlabData (prhs[4], frameSkip);
                    uint32_t msSkip;
                    GetMatlabData (prhs[5], msSkip);
                    camera->startRecording (fileName, numberOfFrames, frameSkip, msSkip);
                }
            }
            else if (!command.compare ("isRecording"))
            {
                plhs[0] = CreateMatlabData (camera->m_recordStopListener.isRecording());
            }
            else if (!command.compare ("stopRecording"))
            {
                camera->stopRecording();
            }
            else if (!command.compare ("isCapturing"))
            {
                bool capturing;
                auto ret = camera->m_cameraDevice->isCapturing (capturing);
                verifyCameraStatus (ret, "Error calling isCapturing.");
                plhs[0] = CreateMatlabData (capturing);
            }
            else if (!command.compare ("isConnected"))
            {
                bool connected;
                auto ret = camera->m_cameraDevice->isConnected (connected);
                verifyCameraStatus (ret, "Error calling isConnected.");
                plhs[0] = CreateMatlabData (connected);
            }
            else if (!command.compare ("getExposureGroups"))
            {
                royale::Vector< royale::String > exposureGroups;
                auto ret = camera->m_cameraDevice->getExposureGroups (exposureGroups);
                verifyCameraStatus (ret, "Error calling getExposureGroups.");
                plhs[0] = CreateMatlabData (exposureGroups);
            }
            else if (!command.compare ("setExposureTime"))
            {
                if (nrhs < 3)
                {
                    UsageErr ("wrong parameter count");
                }

                uint32_t exposureTime;
                if (mxIsChar (prhs[2]))
                {
                    if (nrhs != 4)
                    {
                        UsageErr ("wrong parameter count");
                    }
                    std::string exposureGroup;
                    GetMatlabData (prhs[2], exposureGroup);
                    GetMatlabData (prhs[3], exposureTime);
                    auto ret = camera->m_cameraDevice->setExposureTime (exposureGroup, exposureTime);
                    verifyCameraStatus (ret, "Cannot set exposure time.");
                }
                else
                {
                    GetMatlabData (prhs[2], exposureTime);
                    if (nrhs == 3)
                    {
                        auto ret = camera->m_cameraDevice->setExposureTime (exposureTime);
                        verifyCameraStatus (ret, "Cannot set exposure time.");
                    }
                    else
                    {
                        royale::StreamId streamId;
                        GetMatlabData (prhs[3], streamId);

                        auto ret = camera->m_cameraDevice->setExposureTime (exposureTime, streamId);
                        verifyCameraStatus (ret, "Cannot set exposure time for streamId.");
                    }
                }
            }
            else if (!command.compare ("setExposureTimes"))
            {
                if (nrhs < 3)
                {
                    UsageErr ("wrong parameter count");
                }

                std::vector <uint32_t> exposureTimes;
                GetMatlabData (prhs[2], exposureTimes);

                if (nrhs == 3)
                {
                    auto ret = camera->m_cameraDevice->setExposureTimes (exposureTimes);
                    verifyCameraStatus (ret, "Cannot set exposure times.");
                }
                else
                {
                    royale::StreamId streamId;
                    GetMatlabData (prhs[3], streamId);

                    auto ret = camera->m_cameraDevice->setExposureTimes (exposureTimes, streamId);
                    verifyCameraStatus (ret, "Cannot set exposure times for streamId.");
                }
            }
            else if (!command.compare ("setExposureForGroups"))
            {
                if (nrhs != 3)
                {
                    UsageErr ("wrong parameter count");
                }

                std::vector <uint32_t> exposureTimes;
                GetMatlabData (prhs[2], exposureTimes);

                auto ret = camera->m_cameraDevice->setExposureForGroups (exposureTimes);
                verifyCameraStatus (ret, "Cannot set exposure times for groups.");
            }
            else if (!command.compare ("setExposureMode"))
            {
                if (nrhs < 3)
                {
                    UsageErr ("wrong parameter count");
                }
                uint16_t exposureMode_u;
                GetMatlabData (prhs[2], exposureMode_u);
                auto exposureMode = static_cast <royale::ExposureMode> (exposureMode_u);

                if (nrhs == 3)
                {
                    auto ret = camera->m_cameraDevice->setExposureMode (exposureMode);
                    verifyCameraStatus (ret, "Cannot set exposure mode.");
                }
                else
                {
                    royale::StreamId streamId;
                    GetMatlabData (prhs[3], streamId);

                    auto ret = camera->m_cameraDevice->setExposureMode (exposureMode, streamId);
                    verifyCameraStatus (ret, "Cannot set exposure mode for streamId.");
                }
            }
            else if (!command.compare ("getExposureMode"))
            {
                if (nrhs < 2)
                {
                    UsageErr ("wrong parameter count");
                }

                royale::ExposureMode exposureMode;
                if (nrhs == 3)
                {
                    royale::StreamId streamId;
                    GetMatlabData (prhs[2], streamId);

                    auto ret = camera->m_cameraDevice->getExposureMode (exposureMode, streamId);
                    verifyCameraStatus (ret, "Cannot get exposure mode for streamId.");
                }
                else
                {
                    auto ret = camera->m_cameraDevice->getExposureMode (exposureMode);
                    verifyCameraStatus (ret, "Cannot get exposure mode.");
                }

                plhs[0] = CreateMatlabData (static_cast <uint16_t> (exposureMode));
            }
            else if (!command.compare ("getData"))
            {
                if (camera->m_isPlayBackDevice)
                {
                    if (!camera->m_playbackStopListener.m_isPlaying)
                    {
                        camera->m_depthDataListener.abort (true);
                        camera->m_extendedDataListener.abort (true);
                    }
                }
                switch (camera->m_accessLevel)
                {
                    case royale::CameraAccessLevel::L1:
                        plhs[0] = camera->m_depthDataListener.get();
                        break;
                    case royale::CameraAccessLevel::L2:
                    case royale::CameraAccessLevel::L3:
                    default:
                        camera->m_extendedDataListener.get (nlhs, plhs);
                }
            }
            else if (!command.compare ("getAccessLevel"))
            {
                royale::CameraAccessLevel accessLevel;
                auto ret = camera->m_cameraDevice->getAccessLevel (accessLevel);
                verifyCameraStatus (ret, "Cannot retrieve access level.");
                plhs[0] = CreateMatlabData (static_cast <uint32_t> (accessLevel));
            }
            else if (!command.compare ("setProcessingParameters"))
            {
                // validate input
                if (nrhs < 3)
                {
                    UsageErr ("wrong parameter count");
                }
                const mxArray *&m_parameters = prhs[2];
                if (!mxIsStruct (m_parameters))
                {
                    UsageErr ("wrong parameter type.");
                }
                size_t len = mxGetNumberOfElements (m_parameters);
                if (len != 1)
                {
                    UsageErr ("wrong parameter dimensions.");
                }

                // retrieve current processing parameters
                royale::ProcessingParameterVector parameters;

                if (nrhs == 3)
                {
                    auto ret = camera->m_cameraDevice->getProcessingParameters (parameters);
                    verifyCameraStatus (ret, "Cannot retrieve processing parameters.");
                }
                else
                {
                    royale::StreamId streamId;
                    GetMatlabData (prhs[3], streamId);

                    auto ret = camera->m_cameraDevice->getProcessingParameters (parameters, streamId);
                    verifyCameraStatus (ret, "Cannot retrieve processing parameter for streamId.");
                }

                // retrieve processing flag names
                std::vector<std::string> flags;
                flags.reserve (parameters.size());
                for (size_t i = 0; i < parameters.size(); ++i)
                {
                    auto &par = parameters.at (i);
                    const auto &flag_name = royale::getProcessingFlagName (par.first).toStdString();
                    flags.push_back (flag_name);
                }

                int parameter_count = mxGetNumberOfFields (m_parameters);
                for (int i = 0; i < parameter_count; i++)
                {
                    // find processing flag by name
                    const char *fieldname = mxGetFieldNameByNumber (m_parameters, i);
                    if (fieldname == 0)
                    {
                        mexErrMsgTxt ("Could not retrieve field name.");
                    }
                    auto it = std::find (flags.begin(), flags.end(), std::string (fieldname));
                    if (it == flags.end())
                    {
                        std::ostringstream sbuffer;
                        sbuffer << "Unknown processing flag: " << fieldname;
                        mexWarnMsgTxt (sbuffer.str().c_str());
                        continue;
                    }
                    auto iflag = std::distance (flags.begin(), it);

                    mxArray *m_value = mxGetField (m_parameters, 0, fieldname);
                    royale::Variant v_value;
                    if (has_suffix (*it, "_Bool"))
                    {
                        bool value;
                        GetMatlabData (m_value, value);
                        v_value.setBool (value);
                    }
                    else if (has_suffix (*it, "_Int"))
                    {
                        int32_t value;
                        GetMatlabData (m_value, value);
                        v_value.setInt (static_cast <int> (value));
                    }
                    else if (has_suffix (*it, "_Float"))
                    {
                        float value;
                        GetMatlabData (m_value, value);
                        v_value.setFloat (value);
                    }
                    else
                    {
                        mexWarnMsgTxt ("Processing type unknown. This should not be possible.");
                        continue;
                    }
                    parameters.at (iflag) = royale::Pair<royale::ProcessingFlag, royale::Variant> (parameters.at (iflag).first, v_value);
                }

                if (nrhs == 3)
                {
                    auto ret = camera->m_cameraDevice->setProcessingParameters (parameters);
                    verifyCameraStatus (ret, "Cannot set processing parameters.");
                }
                else
                {
                    royale::StreamId streamId;
                    GetMatlabData (prhs[3], streamId);

                    auto ret = camera->m_cameraDevice->setProcessingParameters (parameters, streamId);
                    verifyCameraStatus (ret, "Cannot set processing parameters for streamId.");
                }
            }
            else if (!command.compare ("getProcessingParameters"))
            {
                // validate input
                if (nrhs < 2)
                {
                    UsageErr ("wrong parameter count");
                }

                royale::ProcessingParameterVector parameters;
                if (nrhs == 2)
                {
                    auto ret = camera->m_cameraDevice->getProcessingParameters (parameters);
                    verifyCameraStatus (ret, "Cannot retrieve processing parameters.");
                }
                else
                {
                    royale::StreamId streamId;
                    GetMatlabData (prhs[2], streamId);

                    auto ret = camera->m_cameraDevice->getProcessingParameters (parameters, streamId);
                    verifyCameraStatus (ret, "Cannot retrieve processing parameter for streamId.");
                }

                // retrieve flag names
                std::vector<std::string> flags;
                flags.reserve (parameters.size());
                for (size_t i = 0; i < parameters.size(); ++i)
                {
                    auto &par = parameters.at (i);
                    const auto &flag_name = royale::getProcessingFlagName (par.first).toStdString();
                    flags.push_back (flag_name);
                }

                // prepare matlab structure
                std::vector<const char *> cflags;
                cflags.reserve (flags.size());
                for (size_t i = 0; i < flags.size(); ++i)
                {
                    cflags.push_back (flags.at (i).c_str());
                }
                mxArray *m_parameters = mxCreateStructMatrix (1, 1, static_cast <int> (cflags.size()), &cflags[0]);

                // fill matlab structure
                for (size_t i = 0; i < parameters.size(); ++i)
                {
                    auto &par = parameters.at (i);
                    auto &flag_name = flags.at (i);
                    mxArray *m_value;
                    if (has_suffix (flag_name, "_Bool"))
                    {
                        if (par.second.variantType() != royale::VariantType::Bool)
                        {
                            std::ostringstream sbuffer;
                            sbuffer << "Processing flag \"" << flag_name << "\" type mismatch.";
                            sbuffer << " Expected Bool.";
                            mexErrMsgTxt (sbuffer.str().c_str());
                        }
                        m_value = CreateMatlabData (par.second.getBool());
                    }
                    else if (has_suffix (flag_name, "_Int"))
                    {
                        if (par.second.variantType() != royale::VariantType::Int)
                        {
                            std::ostringstream sbuffer;
                            sbuffer << "Processing flag \"" << flag_name << "\" type mismatch.";
                            sbuffer << " Expected Int.";
                            mexErrMsgTxt (sbuffer.str().c_str());
                        }
                        m_value = CreateMatlabData (par.second.getInt());
                    }
                    else if (has_suffix (flag_name, "_Float"))
                    {
                        if (par.second.variantType() != royale::VariantType::Float)
                        {
                            std::ostringstream sbuffer;
                            sbuffer << "Processing flag \"" << flag_name << "\" type mismatch.";
                            sbuffer << " Expected Float.";
                            mexErrMsgTxt (sbuffer.str().c_str());
                        }
                        m_value = CreateMatlabData (par.second.getFloat());
                    }
                    else
                    {
                        std::ostringstream sbuffer;
                        sbuffer << "Processing flag \"" << flag_name << "\" type unknown.";
                        mexWarnMsgTxt (sbuffer.str().c_str());
                        continue;
                    }
                    mxSetField (m_parameters, 0, flag_name.c_str(), m_value);
                }

                plhs[0] = m_parameters;
            }
            else if (!command.compare ("setCallbackData"))
            {
                if (nrhs < 3)
                {
                    UsageErr ("wrong parameter count");
                }

                uint16_t cbData_u;
                GetMatlabData (prhs[2], cbData_u);
                auto cbData = static_cast <royale::CallbackData> (cbData_u);

                auto ret = camera->m_cameraDevice->setCallbackData (cbData);
                verifyCameraStatus (ret, "Cannot set callback data.");
            }
            else if (!command.compare ("setCalibrationData"))
            {
                if (nrhs != 3)
                {
                    UsageErr ("wrong parameter count");
                }
                royale::String filename (GetStringFromMatlab (prhs[2]));
                auto ret = camera->m_cameraDevice->setCalibrationData (filename);
                verifyCameraStatus (ret, "Cannot execute setCalibrationData");
            }
            else if (!command.compare ("getCalibrationData"))
            {
                royale::Vector<uint8_t> calibData;
                auto ret = camera->m_cameraDevice->getCalibrationData (calibData);
                verifyCameraStatus (ret, "Cannot retrieve calibration data.");
                plhs[0] = CreateMatlabData (calibData);
            }
            else if (!command.compare ("writeCalibrationToFlash"))
            {
                auto ret = camera->m_cameraDevice->writeCalibrationToFlash();
                verifyCameraStatus (ret, "Cannot write calibration to flash.");
            }
            else if (!command.compare ("writeRegister"))
            {
                if (nrhs != 4)
                {
                    UsageErr ("wrong parameter count");
                }
                royale::String RegisterName (GetStringFromMatlab (prhs[2]));
                uint64_t RegisterValue;
                GetMatlabData (prhs[3], RegisterValue);
                auto ret = camera->m_cameraDevice->writeRegisters ({ { RegisterName, RegisterValue} });
                verifyCameraStatus (ret, "Cannot write register");
            }
            else if (!command.compare ("readRegister"))
            {
                if (nrhs != 3)
                {
                    UsageErr ("wrong parameter count");
                }
                royale::String RegisterName (GetStringFromMatlab (prhs[2]));
                royale::Vector<royale::Pair<royale::String, uint64_t>> registers;
                registers.push_back (royale::Pair<royale::String, uint64_t> (RegisterName, 0));
                auto ret = camera->m_cameraDevice->readRegisters (registers);

                verifyCameraStatus (ret, "Cannot read register");
                plhs[0] = CreateMatlabData (registers.at (0).second);
            }
            // playback commands
            else if (!command.compare ("frameCount"))
            {
                plhs[0] = CreateMatlabData (camera->replay()->frameCount());
            }
            else if (!command.compare ("seek"))
            {
                if (nrhs != 3)
                {
                    UsageErr ("wrong parameter count");
                }
                uint32_t frameNumber;
                GetMatlabData (prhs[2], frameNumber);
                auto ret = camera->replay()->seek (frameNumber);
                verifyCameraStatus (ret, "Seek failed");
            }
            else if (!command.compare ("loop"))
            {
                if (nrhs != 3)
                {
                    UsageErr ("wrong parameter count");
                }
                bool onoff;
                GetMatlabData (prhs[2], onoff);
                camera->replay()->loop (onoff);
            }
            else if (!command.compare ("useTimestamps"))
            {
                if (nrhs != 3)
                {
                    UsageErr ("wrong parameter count");
                }
                bool truefalse;
                GetMatlabData (prhs[2], truefalse);
                camera->replay()->useTimestamps (truefalse);
            }
            else if (!command.compare ("freerun"))
            {
                if (nrhs == 2)
                {
                    plhs[0] = CreateMatlabData (camera->m_depthDataListener.m_freerun);
                    return;
                }
                else if (nrhs != 3)
                {
                    UsageErr ("wrong parameter count");
                }
                bool truefalse;
                GetMatlabData (prhs[2], truefalse);
                camera->m_depthDataListener.m_freerun = truefalse;
                camera->m_extendedDataListener.m_freerun = truefalse;
            }
            else if (!command.compare ("setDutyCycle"))
            {
                if (nrhs != 4)
                {
                    UsageErr ("wrong parameter count");
                }

                double dutycycle;
                GetMatlabData (prhs[2], dutycycle);

                int16_t index;
                GetMatlabData (prhs[3], index);

                royale::CameraStatus ret = camera->m_cameraDevice->setDutyCycle (dutycycle, index);

                verifyCameraStatus (ret, "Cannot set duty cycle time.");
            }
            else if (!command.compare ("shiftLensCenter"))
            {
                if (nrhs != 4)
                {
                    UsageErr ("wrong parameter count");
                }

                int16_t tx, ty;
                GetMatlabData (prhs[2], tx);
                GetMatlabData (prhs[3], ty);

                royale::CameraStatus ret = camera->m_cameraDevice->shiftLensCenter (tx, ty);

                verifyCameraStatus (ret, "Cannot shift lens center.");
            }
            else if (!command.compare ("getLensCenter"))
            {
                if (nrhs != 2)
                {
                    UsageErr ("wrong parameter count");
                }

                uint16_t x, y;

                royale::CameraStatus ret = camera->m_cameraDevice->getLensCenter (x, y);

                verifyCameraStatus (ret, "Cannot retrieve the lens center.");

                plhs[0] = CreateMatlabData (x);
                plhs[1] = CreateMatlabData (y);
            }
            else if (!command.compare ("getNumberOfStreams"))
            {
                if (nrhs != 3)
                {
                    UsageErr ("wrong parameter count");
                }

                uint32_t nr;
                std::string useCase;
                GetMatlabData (prhs[2], useCase);

                auto ret = camera->m_cameraDevice->getNumberOfStreams (useCase, nr);
                verifyCameraStatus (ret, "Cannot get number of streams for use case.");

                plhs[0] = CreateMatlabData (nr);
            }
            else if (!command.compare ("setExternalTrigger"))
            {
                if (nrhs < 3)
                {
                    UsageErr ("wrong parameter count");
                }

                bool useExternalTrigger;
                GetMatlabData (prhs[2], useExternalTrigger);

                auto ret = camera->m_cameraDevice->setExternalTrigger (useExternalTrigger);
                verifyCameraStatus (ret, "Cannot set external trigger.");
            }
            else if (!command.compare ("setMatlabTimeout"))
            {
                if (nrhs < 3)
                {
                    UsageErr ("wrong parameter count");
                }

                uint32_t matlabTimeout;
                GetMatlabData (prhs[2], matlabTimeout);

                g_matlabTimeout = matlabTimeout;
            }
            else if (!command.compare ("writeDataToFlash"))
            {
                if (nrhs != 3)
                {
                    UsageErr ("wrong parameter count");
                }
                royale::String filename (GetStringFromMatlab (prhs[2]));
                auto ret = camera->m_cameraDevice->writeDataToFlash (filename);
                verifyCameraStatus (ret, "Error executing writeDataToFlash");
            }
            else if (!command.compare ("setFrameRate"))
            {
                if (nrhs != 3)
                {
                    UsageErr ("wrong parameter count");
                }
                uint16_t frameRate;
                GetMatlabData (prhs[2], frameRate);

                auto ret = camera->m_cameraDevice->setFrameRate (frameRate);
                verifyCameraStatus (ret, "Error executing setFrameRate");
            }
            else if (!command.compare ("setFilterLevel"))
            {
                if (nrhs < 3)
                {
                    UsageErr ("wrong parameter count");
                }
                uint16_t filterLevelU;
                GetMatlabData (prhs[2], filterLevelU);
                auto filterLevel = static_cast <royale::FilterLevel> (filterLevelU);

                if (nrhs == 3)
                {
                    auto ret = camera->m_cameraDevice->setFilterLevel (filterLevel);
                    verifyCameraStatus (ret, "Cannot set filter level.");
                }
                else
                {
                    royale::StreamId streamId;
                    GetMatlabData (prhs[3], streamId);

                    auto ret = camera->m_cameraDevice->setFilterLevel (filterLevel, streamId);
                    verifyCameraStatus (ret, "Cannot set filter level for streamId.");
                }
            }
            else if (!command.compare ("getFilterLevel"))
            {
                if (nrhs < 2)
                {
                    UsageErr ("wrong parameter count");
                }

                royale::FilterLevel filterLevel;
                if (nrhs == 3)
                {
                    royale::StreamId streamId;
                    GetMatlabData (prhs[2], streamId);

                    auto ret = camera->m_cameraDevice->getFilterLevel (filterLevel, streamId);
                    verifyCameraStatus (ret, "Cannot get filter level for streamId.");
                }
                else
                {
                    auto ret = camera->m_cameraDevice->getFilterLevel (filterLevel);
                    verifyCameraStatus (ret, "Cannot get filter level.");
                }

                plhs[0] = CreateMatlabData (static_cast <uint16_t> (filterLevel));
            }
            else if (!command.compare ("getExposureLimits"))
            {
                if (nrhs < 2)
                {
                    UsageErr ("wrong parameter count");
                }

                royale::Pair<uint32_t, uint32_t> exposureLimits;
                if (nrhs == 3)
                {
                    royale::StreamId streamId;
                    GetMatlabData (prhs[2], streamId);

                    auto ret = camera->m_cameraDevice->getExposureLimits (exposureLimits, streamId);
                    verifyCameraStatus (ret, "Cannot get exposure limits for streamId.");
                }
                else
                {
                    auto ret = camera->m_cameraDevice->getExposureLimits (exposureLimits);
                    verifyCameraStatus (ret, "Cannot get exposure limits.");
                }

                plhs[0] = CreateMatlabData (exposureLimits);
            }
            else
            {
                UsageErr ("Command not found: " + command);
            }
        }
    }

    void getVersion (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
    {
        if ( (nlhs != 1) && (nlhs != 4) && (nlhs != 5))
        {
            mexErrMsgTxt ("Unexpected number of output arguments!");
        }
        unsigned major, minor, patch, build;
        royale::String scmRevision;
        royale::getVersion (major, minor, patch, build, scmRevision);
        switch (nlhs)
        {
            case 1:
                {
                    std::ostringstream sbuffer;
                    sbuffer << major << "."
                            << minor << "."
                            << patch << "."
                            << build << " ("
                            << scmRevision << ")";
                    plhs[0] = mxCreateString (sbuffer.str ().c_str ());
                }
                break;
            case 5:
                plhs[4] = CreateMatlabData (scmRevision);
            case 4:
                plhs[0] = CreateMatlabData (static_cast <uint32_t> (major));
                plhs[1] = CreateMatlabData (static_cast <uint32_t> (minor));
                plhs[2] = CreateMatlabData (static_cast <uint32_t> (patch));
                plhs[3] = CreateMatlabData (static_cast <uint32_t> (build));
                break;
        }
    }

    void GetExposureLimits (const mxArray *m_exposureLimits, std::pair <uint32_t, uint32_t> &exposureLimits)
    {
        // validate input
        if (!mxIsStruct (m_exposureLimits))
        {
            UsageErr ("wrong constructor parameter syntax: exposureLimits.");
        }
        if (mxGetFieldNumber (m_exposureLimits, "minExposure") < 0)
        {
            mexErrMsgTxt ("exposureLimits: missing field \"minExposure\"");
        }
        if (mxGetFieldNumber (m_exposureLimits, "maxExposure") < 0)
        {
            mexErrMsgTxt ("exposureLimits: missing field \"maxExposure\"");
        }
        if (mxGetNumberOfElements (m_exposureLimits) != 1)
        {
            mexErrMsgTxt ("exposureLimits: wrong parameter length");
        }

        GetMatlabData (mxGetField (m_exposureLimits, 0, "minExposure"), exposureLimits.first);
        GetMatlabData (mxGetField (m_exposureLimits, 0, "maxExposure"), exposureLimits.second);
    }

    void GetSequenceInformation (const mxArray *m_sequenceInformation, royale::Vector<royale::Pair<uint32_t, uint32_t>> &sequenceInformation)
    {
        if (!mxIsStruct (m_sequenceInformation))
        {
            UsageErr ("wrong constructor parameter syntax: sequenceInformation.");
        }
        if (mxGetFieldNumber (m_sequenceInformation, "modulationFrequency") < 0)
        {
            mexErrMsgTxt ("sequenceInformation: missing field \"modulationFrequency\"");
        }
        if (mxGetFieldNumber (m_sequenceInformation, "exposureTime") < 0)
        {
            mexErrMsgTxt ("sequenceInformation: missing field \"exposureTime\"");
        }

        size_t len = mxGetNumberOfElements (m_sequenceInformation);
        sequenceInformation.resize (len);
        for (size_t i = 0; i < len; i++)
        {
            uint32_t modulationFrequency, exposureTime;
            GetMatlabData (mxGetField (m_sequenceInformation, i, "modulationFrequency"), modulationFrequency);
            GetMatlabData (mxGetField (m_sequenceInformation, i, "exposureTime"), exposureTime);
            sequenceInformation.at (i).first = modulationFrequency;
            sequenceInformation.at (i).second = exposureTime;
        }
    }

    void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
    {
        if (nrhs < 1)
        {
            UsageErr ("wrong parameter count");
        }

        std::string ClassName (mxGetClassName (prhs[0]));
        if (!ClassName.compare ("char"))
        {
            std::string command = GetStringFromMatlab (prhs[0]);
            if (!command.compare ("getVersion"))
            {
                getVersion (nlhs, plhs, nrhs, prhs);
            }
            else if (!command.compare ("clear"))
            {
                // force an unlock and clears all handles.
                mexUnlock();
                clearAllHandles();
            }
#if !defined(NDEBUG) && defined(ROYALE_USE_LOGGING)
            else if (!command.compare ("setLogFile"))
            {
                if (nrhs < 2)
                {
                    UsageErr ("wrong parameter count");
                }
                std::string logfilePath = GetStringFromMatlab (prhs[1]);
                royale::common::LogSettings::getInstance ()->setLogFile (logfilePath);
                //uint16_t logSettings = royale::common::LogSettings::getInstance ()->logLevel ();
                //logSettings |= (uint16_t)RoyaleLoggerLevels::INFO_;
                //logSettings |= (uint16_t)RoyaleLoggerLevels::DEBUG_;
                //logSettings |= (uint16_t)RoyaleLoggerLevels::WARN_;
                //logSettings |= (uint16_t)RoyaleLoggerLevels::ERROR_;
                //royale::common::LogSettings::getInstance ()->setLogLevel (logSettings);
            }
#endif
            else
            {
                UsageErr ("Command not found: " + command);
            }
        }
        else if (!ClassName.compare ("royale.CameraManager"))
        {
            CameraManager (nlhs, plhs, nrhs, prhs);
        }
        else if (!ClassName.compare ("royale.ICameraDevice"))
        {
            ICameraDevice (nlhs, plhs, nrhs, prhs);
        }
        else
        {
            UsageErr ("Unexpected parameter type: " + ClassName);
        }
    }

}

void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    royale_matlab::mexFunction (nlhs, plhs, nrhs, prhs);
}
