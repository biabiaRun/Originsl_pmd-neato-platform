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
    [StructLayout (LayoutKind.Sequential)]
    public struct IntermediatePoint
    {
        public float distance;                      //!< distance value of the current sequence
        public float amplitude;                     //!< amplitude value of the current sequence
        public float intensity;                     //!< intensity value of the current sequence
        public UInt32 flags;                        //!< flags of the current sequence
    }

    [StructLayout (LayoutKind.Sequential)]
    public struct IntermediateData
    {
        public int version;                         //!< version number of the data format
        public long timeStamp;                      //!< timestamp in milliseconds precision (time since epoch 1970)
        public UInt16 streamId;                     //!< stream which produced the data
        public UInt16 width;                        //!< width of distance image
        public UInt16 height;                       //!< height of distance image
        public IntermediatePoint[] points;          //!< array of intermediate points
        public UInt32[] modulationFrequencies;      //!< modulation frequencies for each sequence
        public UInt32[] exposureTimes;              //!< integration times for each sequence
        public UInt32 numFrequencies;               //!< number of processed frequencies
    }

    // e.g. "warning CS0649: Field 'RoyaleDotNet.RoyaleDepthData.NativeDepthData.version' is never
    // assigned to, and will always have its default value 0"
    // These fields are only assigned through Marshalling from native data structures

    //\cond HIDDEN_SYMBOLS
#pragma warning disable 649
    [StructLayout (LayoutKind.Sequential)]
    internal struct NativeIntermediateData
    {
        public int version;
        public long timeStamp;
        public UInt16 streamId;
        public UInt16 width;
        public UInt16 height;
        public IntPtr points;
        public UInt32 numPoints;
        public IntPtr modFrequencies;
        public UInt32 numModFrequencies;
        public IntPtr exposureTimes;
        public UInt32 numExposureTimes;
        public UInt32 numFrequencies;
    }
#pragma warning restore 649
    internal class IntermediateDataConverter
    {
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void copyDataBlocked (IntPtr dest, IntPtr src, UInt32 length);

        internal static IntermediateData FromNativeData (IntPtr data)
        {
            NativeIntermediateData nativeIntermediateData = (NativeIntermediateData) Marshal.PtrToStructure (
                        data, typeof (NativeIntermediateData));

            IntermediateData intermediateData = new IntermediateData();

            intermediateData.version = nativeIntermediateData.version;
            intermediateData.timeStamp = nativeIntermediateData.timeStamp;
            intermediateData.streamId = nativeIntermediateData.streamId;
            intermediateData.width = nativeIntermediateData.width;
            intermediateData.height = nativeIntermediateData.height;
            intermediateData.numFrequencies = nativeIntermediateData.numFrequencies;
            intermediateData.modulationFrequencies = new UInt32[nativeIntermediateData.numModFrequencies];
            intermediateData.exposureTimes = new UInt32[nativeIntermediateData.numExposureTimes];

            UInt32 dataSize = (UInt32) (nativeIntermediateData.numPoints * Marshal.SizeOf (typeof (IntermediatePoint)));

            GCHandle gcMemory;
            intermediateData.points = new IntermediatePoint[nativeIntermediateData.numPoints];

            gcMemory = GCHandle.Alloc(intermediateData.points, GCHandleType.Pinned);
            copyDataBlocked (gcMemory.AddrOfPinnedObject(), nativeIntermediateData.points, (UInt32) (dataSize));
            gcMemory.Free();

            dataSize = (UInt32) (Marshal.SizeOf (typeof (UInt32)) * nativeIntermediateData.numModFrequencies);

            gcMemory = GCHandle.Alloc (intermediateData.modulationFrequencies, GCHandleType.Pinned);
            copyDataBlocked (gcMemory.AddrOfPinnedObject(), nativeIntermediateData.modFrequencies, (UInt32) (dataSize));
            gcMemory.Free();

            dataSize = (UInt32) (Marshal.SizeOf (typeof (UInt32)) * nativeIntermediateData.numExposureTimes);

            gcMemory = GCHandle.Alloc (intermediateData.exposureTimes, GCHandleType.Pinned);
            copyDataBlocked (gcMemory.AddrOfPinnedObject(), nativeIntermediateData.exposureTimes, (UInt32) (dataSize));
            gcMemory.Free();

            return intermediateData;
        }
    }
    //\endcond
}
