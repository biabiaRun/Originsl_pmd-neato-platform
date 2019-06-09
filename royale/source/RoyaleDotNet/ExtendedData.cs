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
using System.Runtime.InteropServices;

namespace RoyaleDotNet
{
    /// <summary>
    /// Class for getting additional data to the standard depth data. The
    /// retrieval of this data requires L2 access. Please be aware that not
    /// all data is filled. Therefore, use the has* calls to check if data is provided.
    /// </summary>
    public sealed class ExtendedData
    {
        [StructLayout (LayoutKind.Sequential)]
        private struct NativeExtendedData
        {
            [MarshalAs (UnmanagedType.I1)] public bool hasDepthData;
            [MarshalAs (UnmanagedType.I1)] public bool hasIntermediateData;
            [MarshalAs (UnmanagedType.I1)] public bool hasRawData;
            public IntPtr depthData;
            public IntPtr intermediateData;
            public IntPtr rawData;
        }

        private DepthData m_depthData;
        private IntermediateData m_intermediateData;
        private RawData m_rawData;
        private bool m_hasDepthData;
        private bool m_hasIntermediateData;
        private bool m_hasRawData;

        internal ExtendedData (IntPtr data)
        {
            NativeExtendedData nativeExtendedData = (NativeExtendedData) Marshal.PtrToStructure (
                    data, typeof (NativeExtendedData));

            m_hasDepthData = nativeExtendedData.hasDepthData;
            m_hasIntermediateData = nativeExtendedData.hasIntermediateData;
            m_hasRawData = nativeExtendedData.hasRawData;

            if (m_hasDepthData)
            {
                m_depthData = DepthDataConverter.FromNativeData (nativeExtendedData.depthData);
            }

            if (m_hasIntermediateData)
            {
                m_intermediateData = IntermediateDataConverter.FromNativeData (nativeExtendedData.intermediateData);
            }

            if (m_hasRawData)
            {
                m_rawData = RawDataConverter.FromNativeData (nativeExtendedData.rawData);
            }
        }

        public DepthData DepthData
        {
            get
            {
                return m_depthData;
            }
        }

        public IntermediateData IntermediateData
        {
            get
            {
                return m_intermediateData;
            }
        }

        public RawData RawData
        {
            get
            {
                return m_rawData;
            }
        }

        public bool HasDepthData
        {
            get
            {
                return m_hasDepthData;
            }
        }

        public bool HasIntermediateData
        {
            get
            {
                return m_hasIntermediateData;
            }
        }

        public bool HasRawData
        {
            get
            {
                return m_hasRawData;
            }
        }
    }
}
