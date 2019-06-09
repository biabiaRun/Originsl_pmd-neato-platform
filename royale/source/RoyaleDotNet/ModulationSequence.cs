/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

using System;
using System.Collections.Generic;

namespace RoyaleDotNet
{
    /// <summary>
    /// Represents the upper and lower exposure time limit values for a CameraDevice.
    /// </summary>
    public sealed class ModulationSequence
    {
        private UInt32 m_modulationFrequency;
        private UInt32 m_exposureTime;

        public ModulationSequence (UInt32 modulationFrequency, UInt32 exposureTime)
        {
            m_modulationFrequency = modulationFrequency;
            m_exposureTime = exposureTime;
        }

        /// <summary>
        /// Lower exposure time limit.
        /// </summary>
        public UInt32 ModulationFrequency
        {
            get
            {
                return m_modulationFrequency;
            }
        }

        /// <summary>
        /// Upper exposure time limit.
        /// </summary>
        public UInt32 ExposureTime
        {
            get
            {
                return m_exposureTime;
            }
        }

        /// <summary>
        /// String representation of the exposure time limits.
        /// </summary>
        public override string ToString()
        {
            return "ModulationSequence\n{"
                   + "\n  ModulationFrequency: " + m_modulationFrequency
                   + "\n  ExposureTime: " + m_exposureTime
                   + "\n}";
        }
    }
}
