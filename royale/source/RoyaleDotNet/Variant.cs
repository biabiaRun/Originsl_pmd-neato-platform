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
using HND_TYPE = System.UInt64;

namespace RoyaleDotNet
{
    /// <summary>
    /// Implements a variant type which can take different basic data types, the default
    /// type is int and the value is set to zero
    /// </summary>
    public sealed class Variant
    {
        #region Interop DLL Stub
        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern HND_TYPE royale_variant_managed_create();

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern HND_TYPE royale_variant_managed_create_int (int n, int min, int max);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern HND_TYPE royale_variant_managed_create_float (float n, float min, float max);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern HND_TYPE royale_variant_managed_create_bool (bool n);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern HND_TYPE royale_variant_managed_create_type (Int32 type, UInt32 value);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_variant_managed_delete (HND_TYPE variantHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_variant_managed_set_float (HND_TYPE variantHnd, float n);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern float royale_variant_managed_get_float (HND_TYPE variantHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_variant_managed_set_int (HND_TYPE variantHnd, int n);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern int royale_variant_managed_get_int (HND_TYPE variantHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_variant_managed_set_bool (HND_TYPE variantHnd, bool n);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        [return: MarshalAs (UnmanagedType.I1)]
        private static extern bool royale_variant_managed_get_bool (HND_TYPE variantHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern void royale_variant_managed_set_data (HND_TYPE variantHnd, Int32 type, UInt32 value);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern UInt32 royale_variant_managed_get_data (HND_TYPE variantHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        private static extern Int32 royale_variant_managed_get_type (HND_TYPE variantHnd);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        [return: MarshalAs (UnmanagedType.I1)]
        private static extern bool royale_variant_managed_is_equal (HND_TYPE variantHnd1, HND_TYPE variantHnd2);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        [return: MarshalAs (UnmanagedType.I1)]
        private static extern bool royale_variant_managed_is_not_equal (HND_TYPE variantID1, HND_TYPE variantID2);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        [return: MarshalAs (UnmanagedType.I1)]
        private static extern bool royale_variant_managed_is_less_than (HND_TYPE variantID1, HND_TYPE variantID2);

        [DllImport (RoyaleDotNET.royaleCAPI_DLL)]
        [return: MarshalAs (UnmanagedType.I1)]
        private static extern bool royale_variant_managed_is_greater_than (HND_TYPE variantID1, HND_TYPE variantID2);
        #endregion

        private HND_TYPE m_variantHnd;

        public enum VariantType
        {
            Int,
            Float,
            Bool
        };

        public Variant()
        {
            m_variantHnd = royale_variant_managed_create();
            if (m_variantHnd == 0)
            {
                throw new Exception ("Could not instantiate Variant");
            }
        }

        public Variant (int value, int min = int.MinValue, int max = int.MaxValue)
        {
            m_variantHnd = royale_variant_managed_create_int (value, min, max);
            if (m_variantHnd == 0)
            {
                throw new Exception ("Could not instantiate Variant");
            }
        }

        public Variant (float value, float min = float.MinValue, float max = float.MaxValue)
        {
            m_variantHnd = royale_variant_managed_create_float (value, min, max);
            if (m_variantHnd == 0)
            {
                throw new Exception ("Could not instantiate Variant");
            }
        }

        public Variant (bool value)
        {
            m_variantHnd = royale_variant_managed_create_bool (value);
            if (m_variantHnd == 0)
            {
                throw new Exception ("Could not instantiate Variant");
            }
        }

        public Variant (VariantType type, UInt32 value)
        {
            m_variantHnd = royale_variant_managed_create_type ( (Int32) type, value);
            if (m_variantHnd == 0)
            {
                throw new Exception ("Could not instantiate Variant");
            }
        }

        ~Variant()
        {
            royale_variant_managed_delete (m_variantHnd);
        }

        public void setInt (int value)
        {
            royale_variant_managed_set_int (m_variantHnd, value);
        }

        public int getInt()
        {
            return royale_variant_managed_get_int (m_variantHnd);
        }

        public void setFloat (float value)
        {
            royale_variant_managed_set_float (m_variantHnd, value);
        }

        public float getFloat()
        {
            return royale_variant_managed_get_float (m_variantHnd);
        }

        public void setBool (bool value)
        {
            royale_variant_managed_set_bool (m_variantHnd, value);
        }

        public bool getBool()
        {
            return royale_variant_managed_get_bool (m_variantHnd);
        }

        public void setData (VariantType type, UInt32 value)
        {
            royale_variant_managed_set_data (m_variantHnd, (Int32) type, value);
        }

        public UInt32 getData()
        {
            return royale_variant_managed_get_data (m_variantHnd);
        }

        public VariantType Type
        {
            get
            {
                return (VariantType) royale_variant_managed_get_type (m_variantHnd);
            }
        }

        public static bool operator == (Variant v1, Variant v2)
        {
            return royale_variant_managed_is_equal (v1.m_variantHnd, v2.m_variantHnd);
        }

        public static bool operator != (Variant v1, Variant v2)
        {
            return royale_variant_managed_is_not_equal (v1.m_variantHnd, v2.m_variantHnd);
        }

        public static bool operator < (Variant v1, Variant v2)
        {
            return royale_variant_managed_is_less_than (v1.m_variantHnd, v2.m_variantHnd);
        }

        public static bool operator > (Variant v1, Variant v2)
        {
            return royale_variant_managed_is_greater_than (v1.m_variantHnd, v2.m_variantHnd);
        }

        public override int GetHashCode()
        {
            return ( ( (int) this.Type) << 16) ^ ( (int) this.getData());
        }

        public override bool Equals (object obj)
        {
            Variant other;
            try
            {
                other = (Variant) obj;
            }
            catch (InvalidCastException)
            {
                return false;
            }
            if (other != null)
            {
                return this == other;
            }
            return false;
        }

        public override string ToString()
        {
            String s = base.ToString() + " : ";

            switch (Type)
            {
                case VariantType.Bool:
                    s += "Bool: " + getBool();
                    break;
                case VariantType.Float:
                    s += "Float: " + getFloat();
                    break;
                case VariantType.Int:
                    s += "Int: " + getInt();
                    break;
                default:
                    break;
            }

            return s;
        }
    }
}
