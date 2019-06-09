/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <common/exceptions/InvalidValue.hpp>
#include <imager/IImagerComponent.hpp>
#include <imager/ImagerParameters.hpp>
#include <imager/ImagerUseCaseIdentifier.hpp>

#include <chrono>
#include <iomanip>
#include <string>
#include <vector>

#include <royale/Vector.hpp>
#include <royale/Pair.hpp>

namespace royale
{
    namespace imager
    {
        class ImagerSimpleHexSerialNumber;
        inline std::ostream &operator<< (std::ostream &os, const ImagerSimpleHexSerialNumber &serial);

        /**
        * For some imagers, the serial number is simply four e-fuse registers converted to hex.
        * Contrast this with the ImagerM2450Serial, which uses both bit-shifts and decimal.
        *
        * M2453 and MiraDonna use this.
        */
        class ImagerSimpleHexSerialNumber
        {
        public:
            ImagerSimpleHexSerialNumber (std::vector <uint16_t> serialRegisters)
            {
                if (serialRegisters.size() != 4)
                {
                    throw common::InvalidValue ("Unexpected number of serial registers");
                }

                m_serial.push_back (serialRegisters.at (0));
                m_serial.push_back (serialRegisters.at (1));
                m_serial.push_back (serialRegisters.at (2));
                m_serial.push_back (serialRegisters.at (3));
            }

            std::vector<uint16_t> getSerial() const
            {
                return m_serial;
            }

            std::string toString() const
            {
                std::ostringstream  serialNr;
                serialNr << *this;
                return serialNr.str();
            }

        private:
            std::vector <uint16_t> m_serial;
        };

        inline std::ostream &operator<< (std::ostream &os, const ImagerSimpleHexSerialNumber &serial)
        {
            auto data = serial.getSerial();
            os << std::setfill ('0') << std::setw (4) << std::uppercase << std::hex << data.at (0);
            os << "-";
            os << std::setfill ('0') << std::setw (4) << std::uppercase << std::hex << data.at (1);
            os << "-";
            os << std::setfill ('0') << std::setw (4) << std::uppercase << std::hex << data.at (2);
            os << "-";
            os << std::setfill ('0') << std::setw (4) << std::uppercase << std::hex << data.at (3);
            return os;
        }
    }
}
