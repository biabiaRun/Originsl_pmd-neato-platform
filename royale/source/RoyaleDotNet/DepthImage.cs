/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
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
    /// The depth image represents the depth and confidence for every pixel.
    /// The least significant 13 bits are the depth (z value along the optical axis) in
    /// millimeters. 0 stands for invalid measurement / no data.
    ///
    /// The most significant 3 bits correspond to a confidence value. 0 is the highest confidence, 7
    /// the second highest, and 1 the lowest.
    ///
    /// *note* The meaning of the confidence bits changed between Royale v3.14.0 and v3.15.0. Before
    /// v3.15.0, zero was lowest and 7 was highest. Because of this, the member was renamed from
    /// "data" to "cdData".
    /// </summary>
    [StructLayout (LayoutKind.Sequential)]
    public struct DepthImage
    {
        public long timeStamp;  //!< timestamp for the frame
        public UInt16 streamId; //!< stream which produced the data
        public UInt16 width;    //!< width of depth image
        public UInt16 height;   //!< height of depth image
        public UInt16[] cdData; //!< depth and confidence for the pixel
    }

    // e.g. "warning CS0649: Field 'RoyaleDotNet.RoyaleDepthData.NativeDepthData.version' is never
    // assigned to, and will always have its default value 0"
    // These fields are only assigned through Marshalling from native data structures

    //\cond HIDDEN_SYMBOLS
#pragma warning disable 649
    [StructLayout (LayoutKind.Sequential)]
    internal struct NativeDepthImage
    {
        public long timeStamp;
        public UInt16 streamId;
        public UInt16 width;
        public UInt16 height;
        public IntPtr cdData;
        public UInt32 nr_data_entries;
    }
#pragma warning restore 649

    internal class DepthImageConverter
    {
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void copyDataBlocked (IntPtr dest, IntPtr src, UInt32 length);

        internal static DepthImage FromNativeData (IntPtr cdData)
        {
            DepthImage depthImage = new DepthImage();
            NativeDepthImage nativeImage = (NativeDepthImage) Marshal.PtrToStructure (
                                               cdData, typeof (NativeDepthImage));

            depthImage.timeStamp = nativeImage.timeStamp;
            depthImage.streamId = nativeImage.streamId;
            depthImage.width = nativeImage.width;
            depthImage.height = nativeImage.height;
            depthImage.cdData = new UInt16[nativeImage.nr_data_entries];

            GCHandle gcPoints = GCHandle.Alloc (depthImage.cdData, GCHandleType.Pinned);
            copyDataBlocked (gcPoints.AddrOfPinnedObject(), nativeImage.cdData,
                             (UInt32) (nativeImage.nr_data_entries * sizeof (UInt16)));
            gcPoints.Free();

            return depthImage;
        }
    }
    //\endcond
}
