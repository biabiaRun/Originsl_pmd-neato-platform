/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#include "ProbedDeviceInfoList.hpp"

namespace royale
{
    namespace device
    {
        /**
         * This class contains all available information about the results from a device probe. An
         * object of this type can be given as an argument to the method enumerateDevicesWithInfo()
         * of selected classes derived from royale::usb::enumerator::IBusEnumerator.
         */
        class ProbeResultInfo
        {
        public:
            /**
             * Add device information to this probe result info. Do this for every probed device.
             * @param probedDeviceInfo This is the device information which is to be added.
             */
            void addDeviceInfo (const ProbedDeviceInfo &probedDeviceInfo);

            /**
             * Were devices found during the probe?
             * @return whether devices were found.
             */
            bool devicesWereFound() const;

            /**
             * Get a list of infos about the probed devices.
             * @return the probed device info list.
             */
            const ProbedDeviceInfoList &getDeviceInfoList() const;
        private:
            ProbedDeviceInfoList m_probedDeviceInfos;
        };
    }
}
