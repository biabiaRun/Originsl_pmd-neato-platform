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
    /// Encapsulates a 3D point in object space [m]. In addition to the X/Y/Z coordinate
    /// each point also includes a gray value, a noise standard deviation, and a depth confidence value.
    /// </summary>
    [StructLayout (LayoutKind.Sequential)]
    public struct DepthPoint
    {
        public float x;
        public float y;
        public float z;
        public float noise;
        public UInt16 grayValue;
        public byte depthConfidence;
    }

    /// <summary>
    /// This structure defines the depth data which is delivered through the callback.
    /// This data comprises a dense 3D point cloud with the size of the depth image (width, height).
    /// The points vector encodes an array (row-based) with the size of (width x height). Based
    /// on the `depthConfidence`, the user can decide to use the 3D point or not.
    /// </summary>
    [StructLayout (LayoutKind.Sequential)]
    public struct DepthData
    {
        public int version;
        public long timeStamp;
        public UInt16 streamId;
        public UInt16 width;
        public UInt16 height;
        public UInt32[] exposureTimes;
        public DepthPoint[] points;
    }

    // e.g. "warning CS0649: Field 'RoyaleDotNet.RoyaleDepthData.NativeDepthData.version' is never
    // assigned to, and will always have its default value 0"
    // These fields are only assigned through Marshalling from native data structures

    //\cond HIDDEN_SYMBOLS
#pragma warning disable 649
    [StructLayout (LayoutKind.Sequential)]
    internal struct NativeDepthData
    {
        public int version;
        public long timeStamp;
        public UInt16 streamId;
        public UInt16 width;
        public UInt16 height;
        public IntPtr exposureTimes;
        public UInt32 numExposureTimes;
        public IntPtr points;
        public UInt32 numPoints;
    }
#pragma warning restore 649

    internal class DepthDataConverter
    {
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void copyDataBlocked (IntPtr dest, IntPtr src, UInt32 length);

        internal static DepthData FromNativeData (IntPtr data)
        {
            DepthData depthData = new DepthData();
            NativeDepthData nativeData = (NativeDepthData) Marshal.PtrToStructure (
                                             data, typeof (NativeDepthData));

            int pointStructSize = Marshal.SizeOf (typeof (DepthPoint));

            depthData.version = nativeData.version;
            depthData.timeStamp = nativeData.timeStamp;
            depthData.streamId = nativeData.streamId;
            depthData.width = nativeData.width;
            depthData.height = nativeData.height;
            depthData.exposureTimes = new UInt32[nativeData.numExposureTimes];
            depthData.points = new DepthPoint[nativeData.numPoints];

            GCHandle gcExpTimes = GCHandle.Alloc (depthData.exposureTimes, GCHandleType.Pinned);
            copyDataBlocked (gcExpTimes.AddrOfPinnedObject(), nativeData.exposureTimes,
                             (UInt32) (sizeof (UInt32) * nativeData.numExposureTimes));
            gcExpTimes.Free();

            GCHandle gcPoints = GCHandle.Alloc (depthData.points, GCHandleType.Pinned);
            copyDataBlocked (gcPoints.AddrOfPinnedObject(), nativeData.points,
                             (UInt32) (nativeData.numPoints * pointStructSize));
            gcPoints.Free();

            return depthData;
        }
    }
    //\endcond
}
