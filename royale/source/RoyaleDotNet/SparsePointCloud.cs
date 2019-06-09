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
    /// The sparse point cloud gives XYZ and confidence for every valid
    /// point.
    /// It is given as an array of packed coordinate quadruplets (x,y,z,c)
    /// as floating point values. The x, y and z coordinates are in meters.
    /// The confidence (c) has a floating point value in [0.0, 1.0], where 1
    /// corresponds to full confidence.
    /// </summary>
    [StructLayout (LayoutKind.Sequential)]
    public struct SparsePointCloud
    {
        public long timeStamp;
        public UInt16 streamId;
        public UInt32 numPoints;
        public float[] xyzcPoints;
    }

    // e.g. "warning CS0649: Field 'RoyaleDotNet.RoyaleDepthData.NativeDepthData.version' is never
    // assigned to, and will always have its default value 0"
    // These fields are only assigned through Marshalling from native data structures

    //\cond HIDDEN_SYMBOLS
#pragma warning disable 649
    [StructLayout (LayoutKind.Sequential)]
    internal struct NativeSparsePointCloud
    {
        public long timeStamp;    //!< timestamp for the frame
        public UInt16 streamId;   //!< stream which produced the data
        public UInt32 numPoints;  //!< the number of valid points
        public IntPtr xyzcPoints; //!< XYZ and confidence for every valid point
    }
#pragma warning restore 649

    internal class SparsePointCloudConverter
    {
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void copyDataBlocked (IntPtr dest, IntPtr src, UInt32 length);

        internal static SparsePointCloud FromNativeData (IntPtr data)
        {
            SparsePointCloud sparsePointCloud = new SparsePointCloud();
            NativeSparsePointCloud nativeSPC = (NativeSparsePointCloud) Marshal.PtrToStructure (
                                                   data, typeof (NativeSparsePointCloud));

            sparsePointCloud.timeStamp = nativeSPC.timeStamp;
            sparsePointCloud.streamId = nativeSPC.streamId;
            sparsePointCloud.numPoints = nativeSPC.numPoints;
            sparsePointCloud.xyzcPoints = new float[nativeSPC.numPoints * 4];

            GCHandle gcPoints = GCHandle.Alloc (sparsePointCloud.xyzcPoints, GCHandleType.Pinned);
            copyDataBlocked (gcPoints.AddrOfPinnedObject(), nativeSPC.xyzcPoints,
                             (UInt32) (nativeSPC.numPoints * sizeof (float) * 4));
            gcPoints.Free();

            return sparsePointCloud;
        }
    }
    //\endcond
}
