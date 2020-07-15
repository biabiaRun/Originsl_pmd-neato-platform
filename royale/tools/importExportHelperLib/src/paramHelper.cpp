/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <importExportHelperLib/paramHelper.hpp>

#include <regex>
#include <iostream>
#include <fstream>


using namespace royale;
using namespace royale::importExportHelperLib;

inline bool endsWith (const royale::String &value, const royale::String &ending)
{
    if (ending.size() > value.size())
    {
        return false;
    }
    return value.compare (value.length() - ending.length(), ending.length(), ending) == 0;
}

royale::CameraStatus royale::importExportHelperLib::loadCfg (const std::string &cfgFile, std::map<royale::StreamId, royale::ProcessingParameterVector> &outMap)
{
    std::ifstream ifs (cfgFile);
    std::string configString ( (std::istreambuf_iterator<char> (ifs)),
                               (std::istreambuf_iterator<char>()));

    std::regex streamRegex ("^(?!#)\\s*\\[([0-9]+)\\]\\s*([^\\[]*)");
    std::regex keyValueRegex ("^(?!#)\\s*(\\S+)\\s*=\\s*(\\S+)\\s*");


    std::smatch streamMatcher;
    while (std::regex_search (configString, streamMatcher, streamRegex))
    {
        std::string streamIDString = streamMatcher[1];
        std::string streamConfigString = streamMatcher[2];

        royale::ProcessingParameterVector params;
        auto streamID = static_cast<uint16_t> (atoi (streamIDString.c_str()));

        std::smatch keyValueMatcher;
        while (std::regex_search (streamConfigString, keyValueMatcher, keyValueRegex))
        {
            std::string keyString = keyValueMatcher[1];
            std::string valueString = keyValueMatcher[2];

            royale::String key = keyString;

            try
            {
                royale::ProcessingFlag processingFlag;

                if (royale::parseProcessingFlagName (key, processingFlag))
                {

                    if (endsWith (key.toStdString(), "_Bool"))
                    {
                        bool variantValue = valueString.compare ("True") == 0;
                        royale::Variant variant (variantValue);

                        std::pair<royale::ProcessingFlag, royale::Variant> processingParameter (processingFlag, variant);
                        params.push_back (processingParameter);
                    }

                    else if (endsWith (key.toStdString(), "_Int"))
                    {
                        int variantValue = atoi (valueString.c_str());
                        royale::Variant variant (variantValue);

                        std::pair<royale::ProcessingFlag, royale::Variant> processingParameter (processingFlag, variant);
                        params.push_back (processingParameter);
                    }

                    else if (endsWith (key.toStdString(), "_Float"))
                    {
                        auto variantValue = static_cast<float> (atof (valueString.c_str()));
                        royale::Variant variant (variantValue);

                        std::pair<royale::ProcessingFlag, royale::Variant> processingParameter (processingFlag, variant);
                        params.push_back (processingParameter);
                    }
                }
            }
            catch (...)
            {
                return royale::CameraStatus::INVALID_VALUE;
            }

            streamConfigString = keyValueMatcher.suffix().str();
        }

        outMap[streamID] = params;

        configString = streamMatcher.suffix().str();
    }

    return royale::CameraStatus::SUCCESS;
}
