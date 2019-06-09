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
    public struct RawData
    {
        public long timeStamp;                              //!< timestamp in milliseconds precision (time since epoch 1970)
        public UInt16 streamId;                             //!< stream which produced the data
        public UInt16 width;                                //!< width of raw frame
        public UInt16 height;                               //!< height of raw frame
        public List<UInt16[]> rawData;                      //!< pointer to each raw frame
        public UInt32[] modulationFrequencies;              //!< modulation frequencies for each sequence
        public UInt32[] exposureTimes;                      //!< integration times for each sequence
        public float illuminationTemperature;               //!< temperature of illumination
        public UInt16[] phaseAngles;                        //!< phase angles for each raw frame
        public Byte[] illuminationEnabled;                  //!< status of the illumination for each raw frame (1-enabled/0-disabled)
    }

    // e.g. "warning CS0649: Field 'RoyaleDotNet.RoyaleDepthData.NativeDepthData.version' is never
    // assigned to, and will always have its default value 0"
    // These fields are only assigned through Marshalling from native data structures

    //\cond HIDDEN_SYMBOLS
#pragma warning disable 649
    [StructLayout (LayoutKind.Sequential)]
    internal struct NativeRawData
    {
        public long timeStamp;
        public UInt16 streamId;
        public UInt16 width;
        public UInt16 height;
        public IntPtr rawData;
        public UInt16 numFrames;
        public UInt32 numDataPerFrame;
        public IntPtr modulationFrequencies;
        public UInt32 numModulationFrequencies;
        public IntPtr exposureTimes;
        public UInt32 numExposureTimes;
        public float illuminationTemperature;
        public IntPtr phaseAngles;
        public UInt32 numPhaseAngles;
        public IntPtr illuminationEnabled;
        public UInt32 numIlluminationEnabled;

    }
#pragma warning restore 649
    internal class RawDataConverter
    {
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void copyDataBlocked (IntPtr dest, IntPtr src, UInt32 length);

        internal static RawData FromNativeData (IntPtr data)
        {
            NativeRawData nativeRawData = (NativeRawData) Marshal.PtrToStructure (
                                              data, typeof (NativeRawData));

            RawData rawData = new RawData();

            rawData.timeStamp = nativeRawData.timeStamp;
            rawData.streamId = nativeRawData.streamId;
            rawData.width = nativeRawData.width;
            rawData.height = nativeRawData.height;
            rawData.illuminationTemperature = nativeRawData.illuminationTemperature;

            //modulation frequencies
            rawData.modulationFrequencies = new UInt32[nativeRawData.numModulationFrequencies];
            GCHandle gcMemory = GCHandle.Alloc (rawData.modulationFrequencies, GCHandleType.Pinned);
            copyDataBlocked (gcMemory.AddrOfPinnedObject(), nativeRawData.modulationFrequencies, (UInt32) (nativeRawData.numModulationFrequencies * Marshal.SizeOf (typeof (UInt32))));
            gcMemory.Free();

            //exposure times
            rawData.exposureTimes = new UInt32[nativeRawData.numExposureTimes];
            gcMemory = GCHandle.Alloc (rawData.exposureTimes, GCHandleType.Pinned);
            copyDataBlocked (gcMemory.AddrOfPinnedObject(), nativeRawData.exposureTimes, (UInt32) (nativeRawData.numExposureTimes * Marshal.SizeOf (typeof (UInt32))));
            gcMemory.Free();

            //phase angles
            rawData.phaseAngles = new UInt16[nativeRawData.numPhaseAngles];
            gcMemory = GCHandle.Alloc(rawData.phaseAngles, GCHandleType.Pinned);
            copyDataBlocked(gcMemory.AddrOfPinnedObject(), nativeRawData.phaseAngles, (UInt32)(nativeRawData.numPhaseAngles * Marshal.SizeOf(typeof(UInt16))));
            gcMemory.Free();

            //illumination enabled
            rawData.illuminationEnabled = new Byte[nativeRawData.numIlluminationEnabled];
            gcMemory = GCHandle.Alloc(rawData.illuminationEnabled, GCHandleType.Pinned);
            copyDataBlocked(gcMemory.AddrOfPinnedObject(), nativeRawData.illuminationEnabled, (UInt32)(nativeRawData.numIlluminationEnabled * Marshal.SizeOf(typeof(Byte))));
            gcMemory.Free();

            //raw data frames
            rawData.rawData = new List<UInt16[]>();

            Int32 dataSize = (Int32) (nativeRawData.numDataPerFrame * Marshal.SizeOf (typeof (UInt16)));

            IntPtr[] nativaDataPtrArray = new IntPtr[nativeRawData.numFrames];
            Marshal.Copy (nativeRawData.rawData, nativaDataPtrArray, 0, (int) nativeRawData.numFrames);

            for (UInt16 i = 0; i < nativeRawData.numFrames; i++)
            {
                UInt16[] currentFrame = new UInt16[nativeRawData.numDataPerFrame];

                gcMemory = GCHandle.Alloc (currentFrame, GCHandleType.Pinned);
                copyDataBlocked (gcMemory.AddrOfPinnedObject(), nativaDataPtrArray[i], (UInt32) (dataSize));
                gcMemory.Free();

                rawData.rawData.Add (currentFrame);
            }

            return rawData;
        }
    }
    //\endcond
}
