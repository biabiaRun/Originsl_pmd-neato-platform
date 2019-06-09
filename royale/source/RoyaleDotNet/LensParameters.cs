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
    /// Intrinsics of the camera module.
    /// </summary>
    public sealed class LensParameters
    {
        private RoyalePrincipalPoint m_principalPoint;
        private RoyaleFocalLength m_focalLength;
        private RoyaleDistortionTangential m_distortionTangential;
        private RoyaleDistortionRadial m_distortionRadial;

        [StructLayout (LayoutKind.Sequential)]
        internal struct NativePairFloatFloat
        {
            public float first;
            public float second;
        }

        [StructLayout (LayoutKind.Sequential)]
        internal struct NativeVectorFloat
        {
            public IntPtr values;
            public UInt32 numValues;
        }

        [StructLayout (LayoutKind.Sequential)]
        internal struct NativeLensParameters
        {
            public NativePairFloatFloat principalPoint;
            public NativePairFloatFloat focalLength;
            public NativePairFloatFloat distortionTangential;
            public NativeVectorFloat distortionRadial;
        }

        internal LensParameters (NativeLensParameters nativeParams)
        {
            m_principalPoint = new RoyalePrincipalPoint (nativeParams.principalPoint.first, nativeParams.principalPoint.second);
            m_focalLength = new RoyaleFocalLength (nativeParams.focalLength.first, nativeParams.focalLength.second);
            m_distortionTangential = new RoyaleDistortionTangential (nativeParams.distortionTangential.first, nativeParams.distortionTangential.second);

            float[] dRadial = new float[3];
            Marshal.Copy (nativeParams.distortionRadial.values, dRadial, 0, 3);
            m_distortionRadial = new RoyaleDistortionRadial (dRadial[0], dRadial[1], dRadial[2]);
        }

        /// <summary>
        /// Principal point of the lens
        /// </summary>
        public RoyalePrincipalPoint PrincipalPoint
        {
            get
            {
                return m_principalPoint;
            }
        }

        /// <summary>
        /// Focal length of the lens
        /// </summary>
        public RoyaleFocalLength FocalLength
        {
            get
            {
                return m_focalLength;
            }
        }

        /// <summary>
        /// Tangential distortion of the lens
        /// </summary>
        public RoyaleDistortionTangential DistortionTangential
        {
            get
            {
                return m_distortionTangential;
            }
        }

        /// <summary>
        /// Radial distortion of the lens
        /// </summary>
        public RoyaleDistortionRadial DistortionRadial
        {
            get
            {
                return m_distortionRadial;
            }
        }

        /// <summary>
        /// String representation of the camera module intrinsics.
        /// </summary>
        public override string ToString()
        {
            return "LensParameters\n{\n"
                   + "  " + m_principalPoint + "\n"
                   + "  " + m_focalLength + "\n"
                   + "  " + m_distortionTangential + "\n"
                   + "  " + m_distortionRadial + "\n}";
        }

        /// <summary>
        /// Principal point of the lens
        /// cX, cY
        /// </summary>
        public class RoyalePrincipalPoint
        {
            private float m_cX;
            private float m_cY;

            internal RoyalePrincipalPoint (float cX, float cY)
            {
                m_cX = cX;
                m_cY = cY;
            }

            public float CX
            {
                get
                {
                    return m_cX;
                }
            }

            public float CY
            {
                get
                {
                    return m_cY;
                }
            }

            public override string ToString()
            {
                return "PrincipalPoint : {" + m_cX + ", " + m_cY + "}";
            }
        }

        /// <summary>
        /// Focal length of the lens
        /// fX, fY
        /// </summary>
        public class RoyaleFocalLength
        {
            private float m_fX;
            private float m_fY;

            internal RoyaleFocalLength (float fX, float fY)
            {
                m_fX = fX;
                m_fY = fY;
            }

            public float FX
            {
                get
                {
                    return m_fX;
                }
            }

            public float FY
            {
                get
                {
                    return m_fY;
                }
            }

            public override string ToString()
            {
                return "FocalLength : {" + m_fX + ", " + m_fY + "}";
            }
        }

        /// <summary>
        /// Tangential distortion of the lens
        /// p1, p2
        /// </summary>
        public class RoyaleDistortionTangential
        {
            private float m_p1;
            private float m_p2;

            internal RoyaleDistortionTangential (float p1, float p2)
            {
                m_p1 = p1;
                m_p2 = p2;
            }

            public float P1
            {
                get
                {
                    return m_p1;
                }
            }

            public float P2
            {
                get
                {
                    return m_p2;
                }
            }

            public override string ToString()
            {
                return "DistortionTangential : {" + m_p1 + ", " + m_p2 + "}";
            }
        }

        /// <summary>
        /// Radial distortion of the lens
        /// k1, k2, k3
        /// </summary>
        public class RoyaleDistortionRadial
        {
            private float m_k1;
            private float m_k2;
            private float m_k3;

            internal RoyaleDistortionRadial (float k1, float k2, float k3)
            {
                m_k1 = k1;
                m_k2 = k2;
                m_k3 = k3;
            }

            public float K1
            {
                get
                {
                    return m_k1;
                }
            }

            public float K2
            {
                get
                {
                    return m_k2;
                }
            }

            public float K3
            {
                get
                {
                    return m_k3;
                }
            }

            public override string ToString()
            {
                return "DistortionRadial : {" + m_k1 + ", " + m_k2 + ", " + m_k3 + "}";
            }
        }
    }
}
