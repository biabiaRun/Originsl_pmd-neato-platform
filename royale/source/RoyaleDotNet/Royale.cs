/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

using System;
using System.Runtime.InteropServices;

namespace RoyaleDotNet
{
    /// <summary>
    /// Accessor functions to get the version of the Royale library.
    /// </summary>
    public sealed class Royale
    {
        #region Interop DLL Stub
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_free_string (IntPtr dest);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_get_version_with_build_v220 (out UInt32 major, out UInt32 minor, out UInt32 patch, out UInt32 build);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_get_version_with_build_and_scm_revision_v320 (out UInt32 major, out UInt32 minor, out UInt32 patch, out UInt32 build, out IntPtr scm_rev);
        #endregion

        /// <summary>
        /// Returns the version number and build number of the Royale library.
        /// <param name="major">The major component of the version number</param>
        /// <param name="minor">The minor component of the version number</param>
        /// <param name="patch">The patch component of the version number</param>
        /// <param name="build">The build number, if built on the release build server</param>
        /// <remarks>This function always succeeds, no error status needs to be returned</remarks>
        /// </summary>
        public static void GetVersion (out UInt32 major, out UInt32 minor, out UInt32 patch, out UInt32 build)
        {
            royale_get_version_with_build_v220 (out major, out minor, out patch, out build);
        }

        /// <summary>
        /// Returns the version number of the Royale library, including SCM revision number.
        /// <param name="major">The major component of the version number</param>
        /// <param name="minor">The minor component of the version number</param>
        /// <param name="patch">The patch component of the version number</param>
        /// <param name="build">The build number, if built on the release build server</param>
        /// <param name="scm">The source control revision that the library was built from.  May be
        /// empty if the SCM revision wasn't available, and may have the postfix "-dirty" if there
        /// were local changes that weren't checked in to source control.</param>
        /// <returns>CameraStatus.SUCCESS, or an error code if allocating memory for the SCM revision string failed.</returns>
        /// <remarks>If the SCM number is not needed, this can also be read from the AssemblyInfo: typeof(RoyaleDotNet.CameraDevice).Assembly.GetName().Version</remarks>
        /// </summary>
        public static CameraStatus GetVersion (out UInt32 major, out UInt32 minor, out UInt32 patch, out UInt32 build, out String scm)
        {
            IntPtr scmPtr;
            CameraStatus status = (CameraStatus) royale_get_version_with_build_and_scm_revision_v320 (out major, out minor, out patch, out build, out scmPtr);
            if (CameraStatus.SUCCESS == status)
            {
                scm = Marshal.PtrToStringAnsi (scmPtr);
                royale_free_string (scmPtr);
            }
            else
            {
                scm = null;
            }
            return status;
        }
    }
}
