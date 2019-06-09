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
    /// Infrared image with 8Bit mono information for every pixel
    /// </summary>
    [StructLayout (LayoutKind.Sequential)]
    public struct IRImage
    {
        public long timeStamp;  //!< timestamp for the frame
        public UInt16 streamId; //!< stream which produced the data
        public UInt16 width;    //!< width of depth image
        public UInt16 height;   //!< height of depth image
        public byte[] data;     //!< 8Bit mono IR image
    }

    // e.g. "warning CS0649: Field 'RoyaleDotNet.RoyaleDepthData.NativeDepthData.version' is never
    // assigned to, and will always have its default value 0"
    // These fields are only assigned through Marshalling from native data structures

    //\cond HIDDEN_SYMBOLS
#pragma warning disable 649
    [StructLayout (LayoutKind.Sequential)]
    internal struct NativeIRImage
    {
        public long timeStamp;
        public UInt16 streamId;
        public UInt16 width;
        public UInt16 height;
        public IntPtr data;
    }
#pragma warning restore 649

    internal class IRImageConverter
    {
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void copyDataBlocked (IntPtr dest, IntPtr src, UInt32 length);

        internal static IRImage FromNativeData (IntPtr data)
        {
            IRImage sparsePointCloud = new IRImage();
            NativeIRImage nativeImage = (NativeIRImage) Marshal.PtrToStructure (
                                            data, typeof (NativeIRImage));

            int dataSize = nativeImage.width * nativeImage.height;

            sparsePointCloud.timeStamp = nativeImage.timeStamp;
            sparsePointCloud.streamId = nativeImage.streamId;
            sparsePointCloud.width = nativeImage.width;
            sparsePointCloud.height = nativeImage.height;
            sparsePointCloud.data = new byte[dataSize];

            GCHandle gcPoints = GCHandle.Alloc (sparsePointCloud.data, GCHandleType.Pinned);
            copyDataBlocked (gcPoints.AddrOfPinnedObject(), nativeImage.data,
                             (UInt32) (dataSize * sizeof (byte)));
            gcPoints.Free();

            return sparsePointCloud;
        }
    }
    //\endcond
}
