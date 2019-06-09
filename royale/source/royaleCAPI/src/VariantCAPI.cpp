/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <royale/Variant.hpp>
#include <VariantCAPI.h>
#include <private/VariantManagedCAPI.hpp>
#include <private/InstanceManagerCAPI.hpp>

#include <math.h>
#include <limits>

using namespace royale;

InstanceManager<Variant *> m_instanceManager (::InstanceManagerType::VARIANT);

ROYALE_CAPI_LINKAGE_TOP

ROYALE_CAPI royale_variant royale_variant_create_float (float f)
{
    return royale_variant_create_float_with_limits (f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
}

ROYALE_CAPI royale_variant royale_variant_create_int (int i)
{
    return royale_variant_create_int_with_limits (i, std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
}

ROYALE_CAPI royale_variant royale_variant_create_bool (bool b)
{
    royale_variant var;
    royale_variant_set_bool (&var, b);

    return var;
}

ROYALE_CAPI royale_variant royale_variant_create_float_with_limits (float f, float min, float max)
{
    royale_variant var;
    royale_variant_set_float (&var, f);
    var.float_min = min;
    var.float_max = max;
    var.int_min = std::numeric_limits<int>::lowest();
    var.int_max = std::numeric_limits<int>::max();

    return var;
}

ROYALE_CAPI royale_variant royale_variant_create_int_with_limits (int i, int min, int max)
{
    royale_variant var;
    royale_variant_set_int (&var, i);
    var.int_min = min;
    var.int_max = max;
    var.float_min = std::numeric_limits<float>::lowest();
    var.float_max = std::numeric_limits<float>::max();

    return var;
}

ROYALE_CAPI void royale_variant_set_float (royale_variant *v, float f)
{
    v->type = ROYALE_VARIANT_TYPE_FLOAT;
    v->data.f = f;
}

ROYALE_CAPI void royale_variant_set_int (royale_variant *v, int i)
{
    v->type = ROYALE_VARIANT_TYPE_INT;
    v->data.i = i;
}

ROYALE_CAPI void royale_variant_set_bool (royale_variant *v, bool b)
{
    v->type = ROYALE_VARIANT_TYPE_BOOL;
    v->data.b = b;
}

ROYALE_CAPI royale_variant_managed_hnd royale_variant_managed_create()
{
    auto v = new (std::nothrow) Variant();
    if (v == nullptr)
    {
        return ROYALE_NO_VARIANT_INSTANCE_CREATED;
    }

    royale_variant_managed_hnd h = m_instanceManager.AddInstance (v);
    return h;
}

ROYALE_CAPI royale_variant_managed_hnd royale_variant_managed_create_float (float f, float min, float max)
{
    auto v = new (std::nothrow) Variant (f, min, max);
    if (v == nullptr)
    {
        return ROYALE_NO_VARIANT_INSTANCE_CREATED;
    }

    royale_variant_managed_hnd h = m_instanceManager.AddInstance (v);
    return h;
}

ROYALE_CAPI royale_variant_managed_hnd royale_variant_managed_create_int (int i, int min, int max)
{
    auto v = new (std::nothrow) Variant (i, min, max);
    if (v == nullptr)
    {
        return ROYALE_NO_VARIANT_INSTANCE_CREATED;
    }

    royale_variant_managed_hnd h = m_instanceManager.AddInstance (v);
    return h;
}

ROYALE_CAPI royale_variant_managed_hnd royale_variant_managed_create_bool (bool b)
{
    auto v = new (std::nothrow) Variant (b);
    if (v == nullptr)
    {
        return ROYALE_NO_VARIANT_INSTANCE_CREATED;
    }

    royale_variant_managed_hnd h = m_instanceManager.AddInstance (v);
    return h;
}

ROYALE_CAPI royale_variant_managed_hnd royale_variant_managed_create_type (royale_variant_type type, uint32_t value)
{
    auto v = new (std::nothrow) Variant ( (VariantType) type, value);
    if (v == nullptr)
    {
        return ROYALE_NO_VARIANT_INSTANCE_CREATED;
    }

    royale_variant_managed_hnd h = m_instanceManager.AddInstance (v);
    return h;
}

ROYALE_CAPI void royale_variant_managed_delete (royale_variant_managed_hnd handle)
{
    auto v = m_instanceManager.DeleteInstance (handle);
    if (v != nullptr)
    {
        delete v;
    }
}

ROYALE_CAPI void royale_variant_managed_set_float (royale_variant_managed_hnd handle, float f)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL ( (void) 0);
    instance->setFloat (f);
}

ROYALE_CAPI float royale_variant_managed_get_float (royale_variant_managed_hnd handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (NAN);
    return instance->getFloat();
}

ROYALE_CAPI float royale_variant_managed_get_float_min (royale_variant_managed_hnd handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (NAN);
    return instance->getFloatMin();
}

ROYALE_CAPI float royale_variant_managed_get_float_max (royale_variant_managed_hnd handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (NAN);
    return instance->getFloatMax();
}

ROYALE_CAPI void royale_variant_managed_set_int (royale_variant_managed_hnd handle, int i)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL ( (void) 0);
    instance->setInt (i);
}

ROYALE_CAPI int royale_variant_managed_get_int (royale_variant_managed_hnd handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL ( (int) NAN);
    return instance->getInt();
}

ROYALE_CAPI int royale_variant_managed_get_int_min (royale_variant_managed_hnd handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL ( (int) NAN);
    return instance->getIntMin();
}

ROYALE_CAPI int royale_variant_managed_get_int_max (royale_variant_managed_hnd handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL ( (int) NAN);
    return instance->getIntMax();
}

ROYALE_CAPI void royale_variant_managed_set_bool (royale_variant_managed_hnd handle, bool b)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL ( (void) 0);
    return instance->setBool (b);
}

ROYALE_CAPI bool royale_variant_managed_get_bool (royale_variant_managed_hnd handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL ( (bool) NAN);
    return instance->getBool();
}

ROYALE_CAPI void royale_variant_managed_set_data (royale_variant_managed_hnd handle, royale_variant_type type, uint32_t value)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL ( (void) 0);
}

ROYALE_CAPI const uint32_t royale_variant_managed_get_data (royale_variant_managed_hnd handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL (std::numeric_limits<uint32_t>::max());
    const uint32_t data = instance->getData();
    return data;
}

ROYALE_CAPI royale_variant_type royale_variant_managed_get_type (royale_variant_managed_hnd handle)
{
    GET_INSTANCE_AND_RETURN_VALUE_IF_NULL ( (royale_variant_type) std::numeric_limits<int>::max());
    return (royale_variant_type) instance->variantType();
}

ROYALE_CAPI bool royale_variant_managed_is_equal (royale_variant_managed_hnd handle1, royale_variant_managed_hnd handle2)
{
    auto instance1 = m_instanceManager.GetInstanceFromHandle (handle1);
    auto instance2 = m_instanceManager.GetInstanceFromHandle (handle2);

    if (instance1 == nullptr || instance2 == nullptr)
    {
        return false;
    }

    return instance1 == instance2;
}

ROYALE_CAPI bool royale_variant_managed_is_not_equal (royale_variant_managed_hnd handle1, royale_variant_managed_hnd handle2)
{
    auto instance1 = m_instanceManager.GetInstanceFromHandle (handle1);
    auto instance2 = m_instanceManager.GetInstanceFromHandle (handle2);

    if (instance1 == nullptr || instance2 == nullptr)
    {
        return false;
    }

    return instance1 != instance2;
}

ROYALE_CAPI bool royale_variant_managed_is_greater_than (royale_variant_managed_hnd handle1, royale_variant_managed_hnd handle2)
{
    auto instance1 = m_instanceManager.GetInstanceFromHandle (handle1);
    auto instance2 = m_instanceManager.GetInstanceFromHandle (handle2);

    if (instance1 == nullptr || instance2 == nullptr)
    {
        return false;
    }

    return instance1 > instance2;
}

ROYALE_CAPI bool royale_variant_managed_is_less_than (royale_variant_managed_hnd handle1, royale_variant_managed_hnd handle2)
{
    auto instance1 = m_instanceManager.GetInstanceFromHandle (handle1);
    auto instance2 = m_instanceManager.GetInstanceFromHandle (handle2);

    if (instance1 == nullptr || instance2 == nullptr)
    {
        return false;
    }

    return instance1 < instance2;
}


ROYALE_CAPI_LINKAGE_BOTTOM
