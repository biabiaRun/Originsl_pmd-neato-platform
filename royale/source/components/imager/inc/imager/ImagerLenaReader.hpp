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

#include <imager/ImagerCommon.hpp>
#include <config/IImagerExternalConfig.hpp>

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <functional>

namespace royale
{
    namespace imager
    {
        enum class FileSection : size_t
        {
            VERSION = IMG_ENUM,
            INIT_MAP,
            FW_PAGE_1,
            FW_PAGE_2,
            FW_START_MAP,
            START_MAP,
            STOP_MAP,
            USE_CASE_LIST
        };

        /**
        * This is a helper class for the M2453 imager bring-up.
        * It allows to read a text file that contains all
        * register configurations needed to initialize and run
        * a M2453 imager.
        * The file format is called Lena and is defined by this example:
        *
        *   ROYALE-IMAGER-LENA-FILE
        *   VERSION;1003
        *   INIT-MAP
        *   adr;val;delay
        *   adr;val;delay
        *   adr;val;delay
        *   INIT-MAP-END
        *   FW-PAGE-1
        *   adr;val;delay
        *   FW-PAGE-1-END
        *   FW-PAGE-2
        *   adr;val;delay
        *   FW-PAGE-2-END
        *   FW-START-MAP
        *   adr;val;delay
        *   FW-START-MAP-END
        *   START-MAP
        *   adr;val;delay
        *   STOP-MAP
        *   adr;val;delay
        *   START-MAP-END
        *   USECASE-START
        *   GUID;{69F0E9EA-0DFF-47C9-8311-E236F35AB6FF}
        *   NAME;MyUseCase1
        *   BLOCKS;9
        *   MODFREQS;60240000;80320000;80320000;80320000;80320000;60240000;60240000;60240000;60240000
        *   adr;val;delay
        *   adr;val;delay
        *   adr;val;delay
        *   adr;val;delay
        *   USECASE-END
        *   USECASE-START
        *   GUID;{020B6301-30B5-4E39-9F5F-92F7B4DF4A7D}
        *   NAME;MyUseCase2
        *   BLOCKS;4;4;1
        *   adr;val;delay
        *   adr;val;delay
        *   USECASE-END
        *
        * No comments are supported, but it is allowed to have empty lines.
        * The identifiers adr and val denotes a 16-bit hex number
        * (e.g. 0xAB12) and delay is an 32-bit unsigned integer value,
        * denoting a time in units of microseconds.
        *
        * Loading data from connected flash is not supported, the UseCaseData
        * members flashConfigAddress and flashConfigSize will always be zero.
        *
        * Disclaimer: This class is not fully unit tested!
        */
        class ImagerLenaReader : public IImagerExternalConfig
        {
            using ParserFunctional = std::function < void (std::istream &, const std::string &) >;

        public:
            /**
            * Creates an ImagerLenaReader object by reading the provided source string.
            *
            * \param   sourceString   A string containing Lena data.
            * \return  An ImagerLenaReader object containing all information parsed from the string.
            */
            IMAGER_EXPORT static std::unique_ptr<IImagerExternalConfig> fromString (const std::string &sourceString);

            /**
            * Creates an ImagerLenaReader object by reading the full content of the file.
            * The file is only needed for construction of the object,
            * no lock is hold beyond that.
            *
            * \param   sourceFile   Full path to the file containing Lena data.
            * \return  An ImagerLenaReader object containing all information parsed from the file.
            */
            IMAGER_EXPORT static std::unique_ptr<IImagerExternalConfig> fromFile (const std::string &sourceFile);

            IMAGER_EXPORT ~ImagerLenaReader() = default;

            /**
            * Gets the Lena file format version number. Currently this getter is only used
            * for unit testing, but a similar parser for a flash-blob will most likely also
            * publish this information.
            */
            IMAGER_EXPORT const uint32_t getVersion() const;

            /**
            * Returns the TimedRegisterMap that was defined at the INIT-MAP section of the file.
            */
            IMAGER_EXPORT const TimedRegisterList &getInitializationMap() const override;

            /** For Lena, currently unused (always returns a empty header) */
            IMAGER_EXPORT SequentialRegisterHeader getFirmwareHeader1() const override;
            /** For Lena, currently unused (always returns a empty header) */
            IMAGER_EXPORT SequentialRegisterHeader getFirmwareHeader2() const override;

            /**
            * Returns the TimedRegisterMap that was defined at the FW-PAGE-1 section of the file.
            */
            IMAGER_EXPORT const TimedRegisterList &getFirmwarePage1() const override;

            /**
            * Returns the TimedRegisterMap that was defined at the FW-PAGE-2 section of the file.
            */
            IMAGER_EXPORT const TimedRegisterList &getFirmwarePage2() const override;

            /**
            * Returns the TimedRegisterMap that was defined at the FW-START-MAP section of the file.
            */
            IMAGER_EXPORT const TimedRegisterList &getFirmwareStartMap() const override;

            /**
            * Returns the TimedRegisterMap that was defined at the START-MAP section of the file.
            */
            IMAGER_EXPORT const TimedRegisterList &getStartMap() const override;

            /**
            * Returns the TimedRegisterMap that was defined at the STOP-MAP section of the file.
            */
            IMAGER_EXPORT const TimedRegisterList &getStopMap() const override;

            /**
            * Returns the list of use cases, each use case was defined using a USECASE-START
            * section from the file. Each list item consists of a pair of the use case
            * information (UseCaseData) as well as the TimedRegisterMap that belongs to that use case.
            */
            IMAGER_EXPORT const std::vector<UseCaseData> &getUseCaseList() const override;

        private:
            /**
            * The constructor will read the full content of the source stream and
            * stores all extracted information in memory. The source stream is only needed
            * for construction of the object.
            *
            * \param   source   A stream containing Lena data.
            */
            explicit IMAGER_EXPORT ImagerLenaReader (std::istream &source);

            /**
            * A map that defines for each top-level non-terminal symbol
            * a function that is able to parse the sub-tree of such symbol.
            */
            std::map <FileSection, ParserFunctional> m_parseTable;

            /**
            * Initializes the m_initializationMap member by adding for each
            * top-level non-terminal symbol defined in FileSection a parser
            * function that is capable of parsing the sub-block for this
            * specific non-terminal symbol.
            */
            void createParseTable();

            /**
            * Helper method that runs the parser on an istream object.
            */
            void parse (std::istream &stream);

            /**
            * Helper method that parses a register map (adr;val;delay list) and
            * adds the parsed items the outputMap.
            */
            void parseMap (TimedRegisterList &outputMap, std::istream &stream, const std::string &endTag);

            /**
            * Helper method for signalling a parse error.
            * This throws exceptions.
            */
            void raiseParseError();

            //the following members are used to store the parsed information,
            //for each of them a public getter is defined to provide readonly access

            FileSection m_section;
            size_t m_lineNr;
            uint32_t m_version;
            TimedRegisterList m_initializationMap;
            TimedRegisterList m_fwPage1;
            TimedRegisterList m_fwPage2;
            TimedRegisterList m_fwStartMap;
            TimedRegisterList m_startMap;
            TimedRegisterList m_stopMap;
            std::vector<UseCaseData> m_useCases;
        };
    }
}
