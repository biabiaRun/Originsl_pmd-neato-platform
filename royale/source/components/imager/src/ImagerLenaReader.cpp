/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <imager/ImagerLenaReader.hpp>

#include <common/NarrowCast.hpp>

#include <stdexcept>
#include <fstream>
#include <sstream>

using namespace std;
using namespace royale::common;
using namespace royale::imager;

namespace
{
    const string MAGIC_TAG = "ROYALE-IMAGER-LENA-FILE";
    const string VERSION_TAG = "VERSION";
    const string INIT_MAP_TAG = "INIT-MAP";
    const string INIT_MAP_END_TAG = "INIT-MAP-END";
    const string FW_PAGE_1_TAG = "FW-PAGE-1";
    const string FW_PAGE_1_END_TAG = "FW-PAGE-1-END";
    const string FW_PAGE_2_TAG = "FW-PAGE-2";
    const string FW_PAGE_2_END_TAG = "FW-PAGE-2-END";
    const string FW_START_MAP_TAG = "FW-START-MAP";
    const string FW_START_MAP_END_TAG = "FW-START-MAP-END";
    const string START_MAP_TAG = "START-MAP";
    const string START_MAP_END_TAG = "START-MAP-END";
    const string STOP_MAP_TAG = "STOP-MAP";
    const string STOP_MAP_END_TAG = "STOP-MAP-END";
    const string USE_CASE_START_TAG = "USECASE-START";
    const string USE_CASE_END_TAG = "USECASE-END";
    const string GUID_TAG = "GUID";
    const string NAME_TAG = "NAME";
    const string BLOCKS_TAG = "BLOCKS";
    const string MODFREQS_TAG = "MODFREQS";

    const char csvDelimiter = ';';

    const string reviewMessage = "Consider reviewing the input file format at line ";

    const auto startTags = map<const FileSection, const string>
    {
        { FileSection::VERSION, VERSION_TAG },
        { FileSection::INIT_MAP, INIT_MAP_TAG },
        { FileSection::FW_PAGE_1, FW_PAGE_1_TAG },
        { FileSection::FW_PAGE_2, FW_PAGE_2_TAG },
        { FileSection::FW_START_MAP, FW_START_MAP_TAG },
        { FileSection::START_MAP, START_MAP_TAG },
        { FileSection::STOP_MAP, STOP_MAP_TAG },
        { FileSection::USE_CASE_LIST, USE_CASE_START_TAG }
    };

    FileSection &operator++ (FileSection &f)
    {
        return f = (FileSection::USE_CASE_LIST == f) ? f : static_cast<FileSection> ( (size_t) f + 1);
    }

    template <typename T>
    std::string to_string (T value)
    {
        std::ostringstream os;
        os << value;
        return os.str();
    }

    int stoi_dec (const std::string &s)
    {
        auto result = int{0};
        if (! (istringstream (s) >> result))
        {
            throw std::invalid_argument ("Cannot convert to an integer");
        }
        return result;
    }

    int stoi_hex (const std::string &s)
    {
        auto result = int{0};
        if (! (istringstream (s) >> std::hex >> result))
        {
            throw std::invalid_argument ("Cannot convert to an integer");
        }
        return result;
    }

    inline void trimComment (std::string &input)
    {
        if (input.size() >= 2u)
        {
            auto pos = input.find ("//");
            if (pos != std::string::npos)
            {
                // We found a comment starting at pos
                input = input.substr (0, pos);
            }
        }
        if (input.size() >= 1u)
        {
            auto pos = input.find ("#");
            if (pos != std::string::npos)
            {
                // We found a python comment starting at pos
                input = input.substr (0, pos);
            }
        }

        // Trim the string
        std::stringstream trimmedStr;
        trimmedStr << input;
        input.clear();
        trimmedStr >> input;
    }
}

ImagerLenaReader::ImagerLenaReader (std::istream &source) :
    m_section{ FileSection::VERSION },
    m_lineNr{ 0 }
{
    createParseTable();

    string magicTag{ "" };

    while (source.good() && !magicTag.length())
    {
        getline (source, magicTag);
    }

    //the magic tag must match prior to parsing the Lena content
    if (!magicTag.compare (MAGIC_TAG))
    {
        parse (source);
    }
    else
    {
        throw invalid_argument ("Specified source does not contain Lena data");
    }
}

std::unique_ptr<IImagerExternalConfig> ImagerLenaReader::fromString (const string &sourceString)
{
    stringstream istream {sourceString};
    // this can't use common::makeUnique, because ImagerLenaReader::ImagerLenaReader is private
    return std::unique_ptr<ImagerLenaReader> (new ImagerLenaReader (istream));
}

std::unique_ptr<IImagerExternalConfig> ImagerLenaReader::fromFile (const string &sourceFile)
{
    ifstream fileStream { sourceFile };
    if (!fileStream.good())
    {
        throw invalid_argument ("Source file does not exist");
    }
    // this can't use common::makeUnique, because ImagerLenaReader::ImagerLenaReader is private
    return std::unique_ptr<ImagerLenaReader> (new ImagerLenaReader (fileStream));
}

void ImagerLenaReader::parse (std::istream &stream)
{
    for (std::string line;
            std::getline (stream, line) && ++m_lineNr && !stream.eof();)
    {
        trimComment (line);

        if (line.length())
        {
            if (line.find (startTags.at (m_section)))
            {
                raiseParseError();
            }

            try
            {
                m_parseTable.at (m_section) (stream, line);
            }
            catch (const std::out_of_range &)
            {
                raiseParseError();
            }

            ++m_section;
        }
    }

    if (stream.bad())
    {
        throw std::invalid_argument ("Error while reading Lena source");
    }
}

void ImagerLenaReader::parseMap (TimedRegisterList &outputMap, std::istream &stream, const string &endTag)
{
    for (string line; getline (stream, line) && ++m_lineNr && line.find (endTag);)
    {
        trimComment (line);
        std::istringstream s{ line };
        string adr{ "" };
        string val{ "" };
        string delay{ "" };
        getline (s, adr, csvDelimiter);
        getline (s, val, csvDelimiter);
        getline (s, delay, csvDelimiter);

        outputMap.push_back (
        {
            narrow_cast<uint16_t> (stoi_hex (adr.substr (2))),
            narrow_cast<uint16_t> (stoi_hex (val.substr (2))),
            narrow_cast<uint32_t> (stoi_dec (delay))
        });
    }
}

void ImagerLenaReader::raiseParseError()
{
    throw std::logic_error (reviewMessage + to_string (m_lineNr));
}

void ImagerLenaReader::createParseTable()
{
    m_parseTable = std::map <FileSection, ParserFunctional>
    {
        {
            FileSection::VERSION,
            [&] (std::istream & stream, const string & line)
            {
                string lineCopy (line);
                trimComment (lineCopy);
                stringstream s{ lineCopy };
                string version{ "" };
                getline (s, version, csvDelimiter);
                getline (s, version, csvDelimiter);
                m_version = narrow_cast<uint32_t> (stoi_dec (version));
            }
        },
        {
            FileSection::INIT_MAP,
            [&] (std::istream & stream, const string &)
            {
                parseMap (m_initializationMap, stream, INIT_MAP_END_TAG);
            }
        },
        {
            FileSection::FW_PAGE_1,
            [&] (std::istream & stream, const string &)
            {
                parseMap (m_fwPage1, stream, FW_PAGE_1_END_TAG);
            }
        },
        {
            FileSection::FW_PAGE_2,
            [&] (std::istream & stream, const string &)
            {
                parseMap (m_fwPage2, stream, FW_PAGE_2_END_TAG);
            }
        },
        {
            FileSection::FW_START_MAP,
            [&] (std::istream & stream, const string &)
            {
                parseMap (m_fwStartMap, stream, FW_START_MAP_END_TAG);
            }
        },
        {
            FileSection::START_MAP,
            [&] (std::istream & stream, const string &)
            {
                parseMap (m_startMap, stream, START_MAP_END_TAG);
            }
        },
        {
            FileSection::STOP_MAP,
            [&] (std::istream & stream, const string &)
            {
                parseMap (m_stopMap, stream, STOP_MAP_END_TAG);
            }
        },
        {
            FileSection::USE_CASE_LIST,
            [ &, this] (std::istream & stream, const string &)
            {
                //get the meta info from the stream...
                string line;
                UseCaseData pr;

                getline (stream, line) &&++m_lineNr;
                trimComment (line);
                istringstream guid{ line };
                std::string guidAsString;
                getline (guid, guidAsString, csvDelimiter);
                if (guidAsString.find (GUID_TAG) != 0)
                {
                    raiseParseError();
                }
                getline (guid, guidAsString, csvDelimiter);
                pr.guid = parseImagerUseCaseIdentifierGuidString (guidAsString);

                getline (stream, line) &&++m_lineNr;
                trimComment (line);
                istringstream name{ line };
                getline (name, pr.name, csvDelimiter);
                if (pr.name.find (NAME_TAG) != 0)
                {
                    raiseParseError();
                }
                getline (name, pr.name, csvDelimiter);

                getline (stream, line) &&++m_lineNr;
                trimComment (line);
                istringstream blocks{ line };
                string blockSize{ "" };
                getline (blocks, blockSize, csvDelimiter);
                if (blockSize.find (BLOCKS_TAG) != 0)
                {
                    raiseParseError();
                }

                while (getline (blocks, blockSize, csvDelimiter))
                {
                    pr.imageStreamBlockSizes.push_back (narrow_cast<uint32_t> (stoi_dec (blockSize)));
                }

                getline (stream, line) &&++m_lineNr;
                istringstream modFreqs{ line };
                string modFreq{ "" };
                getline (modFreqs, modFreq, csvDelimiter);
                if (modFreq.find (MODFREQS_TAG) != 0)
                {
                    raiseParseError();
                }

                while (getline (modFreqs, modFreq, csvDelimiter))
                {
                    pr.modulationFrequencies.push_back (narrow_cast<uint32_t> (stoi_dec (modFreq)));
                }

                pr.sequentialRegisterHeader.flashConfigAddress = 0u;
                pr.sequentialRegisterHeader.flashConfigSize = 0u;

                //parse the register list
                parseMap (pr.registerMap, stream, USE_CASE_END_TAG);

                m_useCases.push_back (pr);
            }
        }
    };
}


const uint32_t ImagerLenaReader::getVersion() const
{
    return m_version;
}

const TimedRegisterList &ImagerLenaReader::getInitializationMap() const
{
    return m_initializationMap;
}

SequentialRegisterHeader ImagerLenaReader::getFirmwareHeader1() const
{
    return {0, 0, 0};
}

SequentialRegisterHeader ImagerLenaReader::getFirmwareHeader2() const
{
    return {0, 0, 0};
}

const TimedRegisterList &ImagerLenaReader::getFirmwarePage1() const
{
    return m_fwPage1;
}

const TimedRegisterList &ImagerLenaReader::getFirmwarePage2() const
{
    return m_fwPage2;
}

const TimedRegisterList &ImagerLenaReader::getFirmwareStartMap() const
{
    return m_fwStartMap;
}

const TimedRegisterList &ImagerLenaReader::getStartMap() const
{
    return m_startMap;
}

const TimedRegisterList &ImagerLenaReader::getStopMap() const
{
    return m_stopMap;
}

const std::vector<ImagerLenaReader::UseCaseData> &ImagerLenaReader::getUseCaseList() const
{
    return m_useCases;
}
