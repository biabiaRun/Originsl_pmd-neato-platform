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
    public sealed class ExposureLimits
    {
        private UInt32 m_lowerLimit;
        private UInt32 m_upperLimit;

        public ExposureLimits (UInt32 lowerLimit, UInt32 upperLimit)
        {
            m_lowerLimit = lowerLimit;
            m_upperLimit = upperLimit;
        }

        /// <summary>
        /// Lower exposure time limit.
        /// </summary>
        public UInt32 LowerLimit
        {
            get
            {
                return m_lowerLimit;
            }
        }

        /// <summary>
        /// Upper exposure time limit.
        /// </summary>
        public UInt32 UpperLimit
        {
            get
            {
                return m_upperLimit;
            }
        }

        /// <summary>
        /// String representation of the exposure time limits.
        /// </summary>
        public override string ToString()
        {
            return "ExposureLimits\n{"
                   + "\n  LowerLimit: " + m_lowerLimit
                   + "\n  UpperLimit: " + m_upperLimit
                   + "\n}";
        }
    }
}
