/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <royale/Variant.hpp>

#include <string.h>

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/OutOfBounds.hpp>

using namespace royale;
using namespace royale::common;

#define CHECK_VARIANT_TYPE(vtype) \
{ \
    if (m_type != vtype) \
            { \
        throw InvalidType(); \
            } \
}

Variant::Variant() :
    m_type (VariantType::Int),
    i (0),
    intMin (std::numeric_limits<int>::lowest()),
    intMax (std::numeric_limits<int>::max()),
    floatMin (std::numeric_limits<float>::lowest()),
    floatMax (std::numeric_limits<float>::max())
{

}

Variant::Variant (int n, int min, int max)
{
    setIntMinMax (min, max);
    setInt (n);
}

Variant::Variant (float n, float min, float max)
{
    setFloatMinMax (min, max);
    setFloat (n);
}

Variant::Variant (bool n)
{
    setBool (n);
}

Variant::Variant (VariantType type, uint32_t value)
{
    m_type = type;
    setData (type, value);
}

Variant::~Variant()
{

}

void Variant::setFloat (float n)
{
    m_type = VariantType::Float;
    if (n < floatMin || n > floatMax)
    {
        throw OutOfBounds ("Variant out of bounds");
    }
    f = n;
}

void Variant::setFloatMinMax (float min, float max)
{
    floatMin = min;
    floatMax = max;
}

float Variant::getFloat() const
{
    CHECK_VARIANT_TYPE (VariantType::Float)
    return f;
}

float Variant::getFloatMin() const
{
    CHECK_VARIANT_TYPE (VariantType::Float)
    return floatMin;
}

float Variant::getFloatMax() const
{
    CHECK_VARIANT_TYPE (VariantType::Float)
    return floatMax;
}

void Variant::setInt (int n)
{
    m_type = VariantType::Int;
    if (n < intMin || n > intMax)
    {
        throw OutOfBounds ("Variant out of bounds");
    }
    i = n;
}

void Variant::setIntMinMax (int min, int max)
{
    intMin = min;
    intMax = max;
}

int Variant::getInt() const
{
    CHECK_VARIANT_TYPE (VariantType::Int)
    return i;
}

int Variant::getIntMin() const
{
    CHECK_VARIANT_TYPE (VariantType::Int)
    return intMin;
}

int Variant::getIntMax() const
{
    CHECK_VARIANT_TYPE (VariantType::Int)
    return intMax;
}

void Variant::setBool (bool n)
{
    m_type = VariantType::Bool;
    b = n;
}

bool Variant::getBool() const
{
    CHECK_VARIANT_TYPE (VariantType::Bool)
    return b;
}

void Variant::setData (VariantType type, uint32_t value)
{
    switch (type)
    {
        case VariantType::Int:
            {
                setIntMinMax (std::numeric_limits<int>::lowest(),
                              std::numeric_limits<int>::max());
                int tmp;
                memcpy (&tmp, &value, sizeof (uint32_t));
                setInt (tmp);
                break;
            }
        case VariantType::Float:
            {
                setFloatMinMax (std::numeric_limits<float>::lowest(),
                                std::numeric_limits<float>::max());
                float tmp;
                memcpy (&tmp, &value, sizeof (uint32_t));
                setFloat (tmp);
                break;
            }
        case VariantType::Bool:
            {
                if (value)
                {
                    setBool (true);
                }
                else
                {
                    setBool (false);
                }
                break;
            }
        default:
            {
                throw LogicError ("VariantType not supported");
            }
    }

}

uint32_t Variant::getData() const
{
    switch (m_type)
    {
        case VariantType::Int:
            {
                uint32_t ret;
                memcpy (&ret, &i, sizeof (uint32_t));
                return ret;
            }
        case VariantType::Float:
            {
                uint32_t ret;
                memcpy (&ret, &f, sizeof (uint32_t));
                return ret;
            }
        case VariantType::Bool:
            {
                if (b)
                {
                    return 1;
                }
                return 0;
            }
        default:
            {
                throw LogicError ("VariantType not supported");
            }
    }
}

VariantType Variant::variantType() const
{
    return m_type;
}

bool Variant::operator== (const Variant &v) const
{
    if (v.variantType() == m_type)
    {
        switch (m_type)
        {
            case VariantType::Int:
                {
                    if (v.getInt() == getInt())
                    {
                        return true;
                    }
                    break;
                }
            case VariantType::Float:
                {
                    if (v.getFloat() == getFloat())
                    {
                        return true;
                    }
                    break;
                }
            case VariantType::Bool:
                {
                    if (v.getBool() == getBool())
                    {
                        return true;
                    }
                    break;
                }
            default:
                {
                    return false;
                }
        }
    }
    return false;
}

bool Variant::operator!= (const Variant &v) const
{
    return ! (*this == v);
}

bool Variant::operator< (const Variant &v) const
{
    if (v.variantType() == m_type)
    {
        switch (m_type)
        {
            case VariantType::Int:
                {
                    if (getInt() < v.getInt())
                    {
                        return true;
                    }
                    break;
                }
            case VariantType::Float:
                {
                    if (getFloat() < v.getFloat())
                    {
                        return true;
                    }
                    break;
                }
            case VariantType::Bool:
                {
                    if (getBool() < v.getBool())
                    {
                        return true;
                    }
                    break;
                }
            default:
                {
                    return false;
                }
        }
    }
    return false;
}
