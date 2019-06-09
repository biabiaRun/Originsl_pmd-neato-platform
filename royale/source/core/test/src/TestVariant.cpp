#include <royale/Variant.hpp>
#include <gtest/gtest.h>
#include <map>

#include <common/exceptions/OutOfBounds.hpp>

using namespace royale;
using namespace royale::common;

TEST (TestVariant, testBool)
{
    Variant v;
    v.setBool (true);
    EXPECT_EQ (v.getBool(), true);
    EXPECT_THROW (v.getInt(), Variant::InvalidType);

    Variant v2 (VariantType::Bool, false);
    EXPECT_EQ (v2, false);
    EXPECT_NE (v2, true);
}

TEST (TestVariant, testInt)
{
    Variant v;
    v.setInt (-3);
    EXPECT_EQ (v.getInt(), -3);
    EXPECT_THROW (v.getFloat(), Variant::InvalidType);

    Variant v2 (VariantType::Int, -10);
    EXPECT_EQ (v2, -10);
    EXPECT_NE (v2, 5);
}

TEST (TestVariant, testFloat)
{
    Variant v;
    v.setFloat (3.4456f);
    EXPECT_EQ (v.getFloat(), 3.4456f);
    EXPECT_THROW (v.getBool(), Variant::InvalidType);

    Variant v2 (3.4456f);
    EXPECT_EQ (v2, 3.4456f);
    EXPECT_NE (v2, 3.5456f);
}

TEST (TestVariant, testVector)
{
    std::vector<Variant> vec;

    vec.push_back (Variant (2));
    vec.push_back (Variant (3.4f));
    vec.push_back (Variant (true));

    EXPECT_EQ (vec.size(), 3u);
    EXPECT_EQ (vec[0].getInt(), 2);
    EXPECT_EQ (vec[1].getFloat(), 3.4f);
    EXPECT_EQ (vec[2].getBool(), true);
}

TEST (TestVariant, testMap)
{
    enum Flags
    {
        PARAM1,
        PARAM2,
        PARAM3
    };

    std::map<Flags, Variant> params;

    params[PARAM1] = Variant (2);
    params[PARAM2] = Variant (3.4f);
    params[PARAM3] = Variant (true);

    EXPECT_EQ (params.size(), 3u);
    EXPECT_EQ (params[PARAM1].getInt(), 2);
    EXPECT_EQ (params[PARAM2].getFloat(), 3.4f);
    EXPECT_EQ (params[PARAM3].getBool(), true);
}

TEST (TestVariant, edgeCases)
{
    EXPECT_NE (Variant (2), false);

    Variant royVariant;
    royVariant.setData (VariantType::Bool, true);
    EXPECT_EQ (royVariant.getData() > 0, true);

    royVariant.setData (VariantType::Bool, false);
    EXPECT_EQ (royVariant.getData() > 0, false);

    //royVariant.setData(VariantType::Float, static_cast<uint32_t>(3.141592f));
    //EXPECT_EQ (static_cast<float>(royVariant.getData()), 3.141592f);

    royVariant.setData (VariantType::Int, -152);
    EXPECT_EQ (static_cast<int> (royVariant.getData()), -152);

    EXPECT_EQ (royVariant.variantType(), VariantType::Int);

    EXPECT_NE (Variant (2), false);

    EXPECT_EQ (Variant (2) < Variant (false), false);
    EXPECT_EQ (Variant (2) < Variant (2.0f), false);
    EXPECT_EQ (Variant (2) < Variant (1), false);
    EXPECT_EQ (Variant (2) < Variant (2.5f), false);
    EXPECT_EQ (Variant (2) < Variant (3), true);

    EXPECT_EQ (Variant (2.0f) < Variant (3), false);
    EXPECT_EQ (Variant (2.23f) < Variant (2.235f), true);
    EXPECT_EQ (Variant (true) < Variant (true), false);
    EXPECT_EQ (Variant (false) < Variant (true), true);
}

TEST (TestVariant, outOfBounds)
{
    {
        Variant v (1, 0, 2);

        EXPECT_THROW (v.setInt (3), OutOfBounds);
        EXPECT_NO_THROW (v.setInt (0));
        EXPECT_NO_THROW (v.setInt (2));
        EXPECT_THROW (Variant v2 (0, 1, 2), OutOfBounds);
    }
    {
        Variant v (1.0f, 0.0f, 2.0f);

        EXPECT_THROW (v.setFloat (3.0f), OutOfBounds);
        EXPECT_NO_THROW (v.setFloat (0.0f));
        EXPECT_NO_THROW (v.setFloat (2.0f));
        EXPECT_THROW (Variant v2 (0.0f, 1.0f, 2.0f), OutOfBounds);
    }
}
