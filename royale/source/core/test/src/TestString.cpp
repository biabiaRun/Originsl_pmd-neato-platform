#include <common/StringFunctions.hpp>
#include <royale/String.hpp>
#include <royale/Vector.hpp>
#include <gtest/gtest.h>
#include <cstdint>
#include <iostream>
#include <map>

using namespace royale;
using namespace royale::common;
using namespace royale::iterator;

TEST (TestString, TestPushBack)
{
    String str;
    ASSERT_EQ (str.length(), 0u);
    ASSERT_EQ (str.empty(), true);

    str.push_back ("Hello");
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 5u);
    ASSERT_EQ (str.length(), 5u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), 'o');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str == "Hello", true);

    str.push_back (' ');
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 6u);
    ASSERT_EQ (str.length(), 6u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), ' ');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str == "Hello ", true);

    str.push_back (String ("beautiful"));
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 15u);
    ASSERT_EQ (str.length(), 15u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), 'l');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str == "Hello beautiful", true);
    str.push_back (' ');

    str.push_back (std::string ("world"));
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 21u);
    ASSERT_EQ (str.length(), 21u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), 'd');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str == "Hello beautiful world", true);
    ASSERT_EQ (str != "Hello beautiful worl", true);
    ASSERT_EQ (str == "Hello beautiful", false);
    ASSERT_EQ (str == "Hello beautifur", false);
    ASSERT_EQ (str == "Hello beautiful worlt", false);

    ASSERT_EQ (str == std::string ("Hello beautiful world"), true);
    ASSERT_EQ (str != std::string ("Hello beautiful worl"), true);
    ASSERT_EQ (str == std::string ("Hello beautiful"), false);
    ASSERT_EQ (str == std::string ("Hello beautiful worlt"), false);

    String royString1;
    std::string str2 = "Writing ";
    std::string str3 = "print 10 and then 5 more";

    royString1.push_back (str2);                      // "Writing "
    ASSERT_EQ (royString1, "Writing ");
    ASSERT_EQ (royString1, str2);
    ASSERT_EQ (royString1, String ("Writing "));

    royString1.push_back (str3, 6, 3);                  // "10 "
    ASSERT_EQ (royString1, "Writing 10 ");
    ASSERT_EQ (royString1, std::string ("Writing 10 "));
    ASSERT_EQ (royString1, String ("Writing 10 "));

    royString1.push_back ("dots are cool", 5);         // "dots "
    ASSERT_EQ (royString1, "Writing 10 dots ");
    ASSERT_EQ (royString1, std::string ("Writing 10 dots "));
    ASSERT_EQ (royString1, String ("Writing 10 dots "));

    royString1.push_back ("here: ");                  // "here: "
    ASSERT_EQ (royString1, "Writing 10 dots here: ");
    ASSERT_EQ (royString1, std::string ("Writing 10 dots here: "));
    ASSERT_EQ (royString1, String ("Writing 10 dots here: "));

    royString1.push_back (10u, '.');                   // ".........."
    ASSERT_EQ (royString1, "Writing 10 dots here: ..........");
    ASSERT_EQ (royString1, std::string ("Writing 10 dots here: .........."));
    ASSERT_EQ (royString1, String ("Writing 10 dots here: .........."));

    royString1.push_back (str3, 8, str3.length());                  // " and then 5 more"

    ASSERT_EQ (royString1, "Writing 10 dots here: .......... and then 5 more");
    ASSERT_EQ (royString1, std::string ("Writing 10 dots here: .......... and then 5 more"));
    ASSERT_EQ (royString1, String ("Writing 10 dots here: .......... and then 5 more"));

    royString1.push_back (5, 0x2E);               // "....."
    ASSERT_EQ (royString1, "Writing 10 dots here: .......... and then 5 more.....");
    ASSERT_EQ (royString1, std::string ("Writing 10 dots here: .......... and then 5 more....."));
    ASSERT_EQ (royString1, String ("Writing 10 dots here: .......... and then 5 more....."));


    String royString2;
    String royStr2 = "Writing ";
    String royStr3 = "print 10 and then 5 more";

    royString2.push_back (royStr2);                      // "Writing "
    ASSERT_EQ (royString2, "Writing ");
    ASSERT_EQ (royString2, royStr2);
    ASSERT_EQ (royString2, String ("Writing "));

    royString2.push_back (royStr3, 6, 3);                  // "10 "
    ASSERT_EQ (royString2, "Writing 10 ");
    ASSERT_EQ (royString2, std::string ("Writing 10 "));
    ASSERT_EQ (royString2, String ("Writing 10 "));

    royString2.push_back ("dots are cool", 5);         // "dots "
    ASSERT_EQ (royString2, "Writing 10 dots ");
    ASSERT_EQ (royString2, std::string ("Writing 10 dots "));
    ASSERT_EQ (royString2, String ("Writing 10 dots "));

    royString2.push_back ("here: ");                  // "here: "
    ASSERT_EQ (royString2, "Writing 10 dots here: ");
    ASSERT_EQ (royString2, std::string ("Writing 10 dots here: "));
    ASSERT_EQ (royString2, String ("Writing 10 dots here: "));

    royString2.push_back (10u, '.');                   // ".........."
    ASSERT_EQ (royString2, "Writing 10 dots here: ..........");
    ASSERT_EQ (royString2, std::string ("Writing 10 dots here: .........."));
    ASSERT_EQ (royString2, String ("Writing 10 dots here: .........."));

    royString2.push_back (royStr3, 8, royStr3.length());                  // " and then 5 more"

    ASSERT_EQ (royString2, "Writing 10 dots here: .......... and then 5 more");
    ASSERT_EQ (royString2, std::string ("Writing 10 dots here: .......... and then 5 more"));
    ASSERT_EQ (royString2, String ("Writing 10 dots here: .......... and then 5 more"));

    royString2.push_back (5, 0x2E);               // "....."
    ASSERT_EQ (royString2, "Writing 10 dots here: .......... and then 5 more.....");
    ASSERT_EQ (royString2, std::string ("Writing 10 dots here: .......... and then 5 more....."));
    ASSERT_EQ (royString2, String ("Writing 10 dots here: .......... and then 5 more....."));

    ASSERT_EQ (royString1, royString2);

    ASSERT_NO_THROW (royString1.clear());
    ASSERT_EQ (royString1.length(), 0u);
    ASSERT_EQ (royString1.size(), 0u);
    ASSERT_EQ (royString1.empty(), true);
    ASSERT_EQ (royString1.capacity(), 0u);

    ASSERT_NE (royString2.length(), 0u);
    ASSERT_NE (royString2.size(), 0u);
    ASSERT_NE (royString2.empty(), true);
    ASSERT_NE (royString2.capacity(), 0u);
}

TEST (TestString, TestAppend)
{
    royale::String str;
    ASSERT_EQ (str.length(), 0u);
    ASSERT_EQ (str.empty(), true);

    str.append ("Hello");
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 5u);
    ASSERT_EQ (str.length(), 5u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), 'o');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str, "Hello");

    str.append (' ');
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 6u);
    ASSERT_EQ (str.length(), 6u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), ' ');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str, "Hello ");

    str.append (String ("beautiful"));
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 15u);
    ASSERT_EQ (str.length(), 15u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), 'l');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str, "Hello beautiful");
    str.append (' ');

    str.append (std::string ("world"));
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 21u);
    ASSERT_EQ (str.length(), 21u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), 'd');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str, "Hello beautiful world");


    String royString1;
    std::string str2 = "Writing ";
    std::string str3 = "print 10 and then 5 more";

    royString1.append (str2);                      // "Writing "
    ASSERT_EQ (royString1, "Writing ");
    ASSERT_EQ (royString1, str2);
    ASSERT_EQ (royString1, String ("Writing "));

    royString1.append (str3, 6, 3);                  // "10 "
    ASSERT_EQ (royString1, "Writing 10 ");
    ASSERT_EQ (royString1, std::string ("Writing 10 "));
    ASSERT_EQ (royString1, String ("Writing 10 "));

    royString1.append ("dots are cool", 5);         // "dots "
    ASSERT_EQ (royString1, "Writing 10 dots ");
    ASSERT_EQ (royString1, std::string ("Writing 10 dots "));
    ASSERT_EQ (royString1, String ("Writing 10 dots "));

    royString1.append ("here: ");                  // "here: "
    ASSERT_EQ (royString1, "Writing 10 dots here: ");
    ASSERT_EQ (royString1, std::string ("Writing 10 dots here: "));
    ASSERT_EQ (royString1, String ("Writing 10 dots here: "));

    royString1.append (10u, '.');                   // ".........."
    ASSERT_EQ (royString1, "Writing 10 dots here: ..........");
    ASSERT_EQ (royString1, std::string ("Writing 10 dots here: .........."));
    ASSERT_EQ (royString1, String ("Writing 10 dots here: .........."));

    royString1.append (str3, 8, str3.length());                  // " and then 5 more"

    ASSERT_EQ (royString1, "Writing 10 dots here: .......... and then 5 more");
    ASSERT_EQ (royString1, std::string ("Writing 10 dots here: .......... and then 5 more"));
    ASSERT_EQ (royString1, String ("Writing 10 dots here: .......... and then 5 more"));

    royString1.append (5, 0x2E);               // "....."
    ASSERT_EQ (royString1, "Writing 10 dots here: .......... and then 5 more.....");
    ASSERT_EQ (royString1, std::string ("Writing 10 dots here: .......... and then 5 more....."));
    ASSERT_EQ (royString1, String ("Writing 10 dots here: .......... and then 5 more....."));


    String royString2;
    String royStr2 = "Writing ";
    String royStr3 = "print 10 and then 5 more";

    royString2.append (royStr2);                      // "Writing "
    ASSERT_EQ (royString2, "Writing ");
    ASSERT_EQ (royString2, royStr2);
    ASSERT_EQ (royString2, String ("Writing "));

    royString2.append (royStr3, 6, 3);                  // "10 "
    ASSERT_EQ (royString2, "Writing 10 ");
    ASSERT_EQ (royString2, std::string ("Writing 10 "));
    ASSERT_EQ (royString2, String ("Writing 10 "));

    royString2.append ("dots are cool", 5);         // "dots "
    ASSERT_EQ (royString2, "Writing 10 dots ");
    ASSERT_EQ (royString2, std::string ("Writing 10 dots "));
    ASSERT_EQ (royString2, String ("Writing 10 dots "));

    royString2.append ("here: ");                  // "here: "
    ASSERT_EQ (royString2, "Writing 10 dots here: ");
    ASSERT_EQ (royString2, std::string ("Writing 10 dots here: "));
    ASSERT_EQ (royString2, String ("Writing 10 dots here: "));

    royString2.append (10u, '.');                   // ".........."
    ASSERT_EQ (royString2, "Writing 10 dots here: ..........");
    ASSERT_EQ (royString2, std::string ("Writing 10 dots here: .........."));
    ASSERT_EQ (royString2, String ("Writing 10 dots here: .........."));

    royString2.append (royStr3, 8, royStr3.length());                  // " and then 5 more"

    ASSERT_EQ (royString2, "Writing 10 dots here: .......... and then 5 more");
    ASSERT_EQ (royString2, std::string ("Writing 10 dots here: .......... and then 5 more"));
    ASSERT_EQ (royString2, String ("Writing 10 dots here: .......... and then 5 more"));

    royString2.append (5, 0x2E);               // "....."
    ASSERT_EQ (royString2, "Writing 10 dots here: .......... and then 5 more.....");
    ASSERT_EQ (royString2, std::string ("Writing 10 dots here: .......... and then 5 more....."));
    ASSERT_EQ (royString2, String ("Writing 10 dots here: .......... and then 5 more....."));

    ASSERT_EQ (royString1, royString2);

    String checkString2 ("Hello World #");
    String checkString (checkString2);
    checkString += String ("6");

    {
        String helloworld_ ("Hello world", 1u);
        ASSERT_EQ (helloworld_, "H");
        helloworld_.append ("");
        ASSERT_EQ (helloworld_, "H");

        String helloworld ("Hello world", 30u);
        ASSERT_EQ (helloworld, "Hello world");
        helloworld.append (" over there", 0u);
        ASSERT_EQ (helloworld, "Hello world");

        String helloWorld2 (helloworld);
        String helloWorld3 (helloworld);
        helloworld.append (" over here", 5u);
        ASSERT_EQ (helloworld, "Hello world over");
        ASSERT_NE (helloworld, helloWorld2);

        ASSERT_EQ (helloWorld2, "Hello world");
        ASSERT_EQ (helloWorld2.append (" over there", 500u), "Hello world over there");
        ASSERT_EQ (helloWorld3.append (" ").append (helloWorld2, 18u, 4u), "Hello world here");

        helloWorld3.append (", not", 100u).append (helloWorld2, 11u, 1000u);
        ASSERT_EQ (helloWorld3, "Hello world here, not over there");
    }
}

TEST (TestString, TestPlusAssignmentOperator)
{
    royale::String str;
    ASSERT_EQ (str.length(), 0u);
    ASSERT_EQ (str.empty(), true);

    str += "Hello";
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 5u);
    ASSERT_EQ (str.length(), 5u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), 'o');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str, "Hello");

    str += ' ';
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 6u);
    ASSERT_EQ (str.length(), 6u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), ' ');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str, "Hello ");

    str += String ("beautiful");
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 15u);
    ASSERT_EQ (str.length(), 15u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), 'l');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str, "Hello beautiful");
    str += ' ';

    str += std::string ("world");
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 21u);
    ASSERT_EQ (str.length(), 21u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), 'd');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str, "Hello beautiful world");

    ASSERT_EQ (str + std::string (" of mine"), "Hello beautiful world of mine");
}

TEST (TestString, TestAssignmentOperator)
{
    const String str ("123456");
    ASSERT_EQ (str.length(), 6u);
    ASSERT_EQ (str.empty(), false);

    String str2;
    str2 = str;
    ASSERT_EQ (str2.length(), 6u);
    ASSERT_EQ (str2.empty(), false);
    ASSERT_EQ (str, str2);
}

TEST (TestString, TestCtors)
{
    String str (std::string ("Hello"));
    ASSERT_EQ (str.length(), 5u);
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str, "Hello");

    String str2 (String (" world"));
    ASSERT_EQ (str2.length(), 6u);
    ASSERT_EQ (str2.empty(), false);
    ASSERT_EQ (str2, " world");

    ASSERT_EQ (str + str2, "Hello world");

    String str3 (std::move (str2));
    ASSERT_EQ (str3, " world");

    String appendedCtor (str + str3 + " EOT");
    ASSERT_EQ (appendedCtor, "Hello world EOT");

    String worldCopy (str + str3 + " EOT", 6u, 5u);
    ASSERT_EQ (worldCopy, "world");

    String helloWorld ("Hello world", 5u);
    ASSERT_EQ (helloWorld, str);

    String dots (5u, '.');
    ASSERT_EQ (dots, ".....");

    String listInitialized = { 'H', 'i', ' ', 'b', 'e', 'a', 'u', 't', 'i', 'f', 'u', 'l', ' ', 'W', 'o', 'r', 'l', 'd' };
    ASSERT_EQ (listInitialized, "Hi beautiful World");

    String empty;
    String empty2 (empty);

    std::string emptyStdString;
    empty2 = emptyStdString;
    empty2 = empty;
    empty = "";
    empty.append (0, 'A');

    String newEmpty = empty2 + empty + empty2;
    ASSERT_EQ (newEmpty, empty2);

    {
        String helloWorld ("Hello World");
        String world (helloWorld, 6u, 100u);
        ASSERT_EQ (world, "World");
    }

    {
        std::string helloWorld ("Hello World");
        String world (helloWorld, 6u, 100u);
        ASSERT_EQ (world, "World");
    }

    {
        String helloWorld ("Hello World");
        String world (helloWorld, 6u, 1u);
        ASSERT_EQ (world, "W");
    }

    {
        std::string helloWorld ("Hello World");
        String world (helloWorld, 6u, 1u);
        ASSERT_EQ (world, "W");
    }

    {
        String helloworld_ ("Hello world", 1u);
        ASSERT_EQ (helloworld_, "H");
        helloworld_.append ("");
        ASSERT_EQ (helloworld_, "H");

        String helloworld ("Hello world", 30u);
        ASSERT_EQ (helloworld, "Hello world");
        helloworld.append (" over there", 0u);
        ASSERT_EQ (helloworld, "Hello world");
    }
}

TEST (TestString, TestWideCtors)
{
    WString str (std::wstring ({'H', 'e', 'l', 'l', 'o'}));
    ASSERT_EQ (str.length(), 5u);
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str, L"Hello");

    WString str2 (WString (L" world"));
    ASSERT_EQ (str2.length(), 6u);
    ASSERT_EQ (str2.empty(), false);
    ASSERT_EQ (str2, L" world");

    ASSERT_EQ (str + str2, L"Hello world");

    WString str3 (std::move (str2));
    ASSERT_EQ (str3, L" world");

    WString appendedCtor (str + str3 + L" EOT");
    ASSERT_EQ (appendedCtor, L"Hello world EOT");

    WString worldCopy (str + str3 + L" EOT", 6u, 5u);
    ASSERT_EQ (worldCopy, L"world");

    WString helloWorld (L"Hello world", 5u);
    ASSERT_EQ (helloWorld, str);

    WString dots (5u, '.');
    ASSERT_EQ (dots, L".....");

    WString listInitialized = { 'H', 'i', ' ', 'b', 'e', 'a', 'u', 't', 'i', 'f', 'u', 'l', ' ', 'W', 'o', 'r', 'l', 'd' };
    ASSERT_EQ (listInitialized, L"Hi beautiful World");

    WString empty;
    WString empty2 (empty);

    std::wstring emptyStdWString;
    empty2 = emptyStdWString;
    empty2 = empty;
    empty = L"";
    empty.append (0, wchar_t {'A'});

    WString newEmpty = empty2 + empty + empty2;
    ASSERT_EQ (newEmpty, empty2);

    {
        WString helloWorld (L"Hello World");
        WString world (helloWorld, 6u, 100u);
        ASSERT_EQ (world, L"World");
    }

    {
        std::wstring helloWorld (L"Hello World");
        WString world (helloWorld, 6u, 100u);
        ASSERT_EQ (world, L"World");
    }

    {
        WString helloWorld (L"Hello World");
        WString world (helloWorld, 6u, 1u);
        ASSERT_EQ (world, L"W");
    }

    {
        std::wstring helloWorld (L"Hello World");
        WString world (helloWorld, 6u, 1u);
        ASSERT_EQ (world, L"W");
    }

    {
        WString helloworld_ (L"Hello world", 1u);
        ASSERT_EQ (helloworld_, L"H");
        helloworld_.append (L"");
        ASSERT_EQ (helloworld_, L"H");

        WString helloworld (L"Hello world", 30u);
        ASSERT_EQ (helloworld, L"Hello world");
        helloworld.append (L" over there", 0u);
        ASSERT_EQ (helloworld, L"Hello world");
    }
}

/**
 * royale::String's (char *, length) constructor is documented to truncate the string at the first
 * NUL character, even if given a length that is longer. This is different to std::string, which
 * creates a string with embedded NULs.
 *
 * The behavior has been unchanged since at least Royale 1.6.0, so this test checks that the
 * behavior doesn't change.
 */
TEST (TestString, TestCtorEmbeddedNul)
{
    {
        const char charPtr[] = {'a', 0, 'b', 0};
        String str (charPtr, 3);
        ASSERT_EQ (1u, str.size());
    }

    {
        const wchar_t wcharPtr[] = {'a', 0, 'b', 0};
        WString wstr (wcharPtr, 3);
        ASSERT_EQ (1u, wstr.size());
    }
}

TEST (TestString, TestOperators)
{
    String str (std::string ("Hello"));
    ASSERT_EQ (str.length(), 5u);
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str, "Hello");

    str = "Hello World, what is happening if we assign a super big value here?";
    ASSERT_EQ (str, "Hello World, what is happening if we assign a super big value here?");

    str = 'a';
    ASSERT_EQ (str, "a");
    ASSERT_EQ (str.length(), 1u);
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.capacity(), 1u);     // zero termination isn't counted (any more)

    str = std::string ("This is a test");
    ASSERT_EQ (str, "This is a test");
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.capacity(), 14u);


    str = String ("Especially for our basic strings");
    ASSERT_EQ (str, std::string ("Especially for our basic strings"));
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.capacity(), 32u);

    String str2 (String ("World"));
    ASSERT_EQ (str2.length(), 5u);
    ASSERT_EQ (str2.empty(), false);
    ASSERT_EQ (str2, "World");
    ASSERT_EQ (str2.capacity(), 5u);

    /* Move assignment */
    String str3;
    str3 = std::move (str2);
    ASSERT_EQ (str3, "World");
}

TEST (TestString, TestOperatorLess)
{
    ASSERT_EQ (String ("")   < String (""),   false);
    ASSERT_EQ (String ("a")  < String ("a"),  false);
    ASSERT_EQ (String ("")   < String ("a"),  true);
    ASSERT_EQ (String ("a")  < String (""),   false);
    ASSERT_EQ (String ("a")  < String ("ab"), true);
    ASSERT_EQ (String ("ab") < String ("a"),  false);
    ASSERT_EQ (String ("a")  < String ("b"),  true);
    ASSERT_EQ (String ("b")  < String ("a"),  false);
}


TEST (TestString, TestAccessSpecifiers)
{
    {
        String str (std::string ("Hello"));
        ASSERT_EQ (str.length(), 5u);
        ASSERT_EQ (str.empty(), false);
        ASSERT_EQ (str, "Hello");

        ASSERT_EQ (str.at (0), str.front());
        ASSERT_EQ (str.at (str.length() - 1), str.back());

        ASSERT_EQ (str.at (0), str[0]);
        ASSERT_EQ (str[str.length() - 1], str.back());

        ASSERT_EQ (str.at (0), 'H');
        ASSERT_EQ (str[0], 'H');
        ASSERT_EQ (str.front(), 'H');

        ASSERT_EQ (str.at (str.length() - 1), 'o');
        ASSERT_EQ (str[str.length() - 1], 'o');
        ASSERT_EQ (str.back(), 'o');

        ASSERT_EQ (str[str.length()], String::EOS);
        ASSERT_EQ (str.at (str.length()), String::EOS);
        ASSERT_THROW (str.at (str.length() + 1), std::out_of_range);

        str[1] = 'a';
        ASSERT_EQ (str, "Hallo");

        str.at (1) = 'e';
        ASSERT_EQ (str, "Hello");

        ASSERT_EQ (* (str.data() + 3), 'l');
    }

    {
        String strHolder (std::string ("Hello"));
        const String &str = strHolder;

        ASSERT_EQ (str.length(), 5u);
        ASSERT_EQ (str.empty(), false);
        ASSERT_EQ (str, "Hello");

        ASSERT_EQ (str.at (0), str.front());
        ASSERT_EQ (str.at (str.length() - 1), str.back());

        ASSERT_EQ (str.at (0), str[0]);
        ASSERT_EQ (str[str.length() - 1], str.back());

        ASSERT_EQ (str.at (0), 'H');
        ASSERT_EQ (str[0], 'H');
        ASSERT_EQ (str.front(), 'H');

        ASSERT_EQ (str.at (str.length() - 1), 'o');
        ASSERT_EQ (str[str.length() - 1], 'o');
        ASSERT_EQ (str.back(), 'o');

        ASSERT_EQ (str[str.length()], '\0');
        ASSERT_EQ (str.at (str.length()), '\0');
        ASSERT_THROW (str.at (str.length() + 1), std::out_of_range);

        ASSERT_EQ (* (str.data() + 3), 'l');
    }
}

TEST (TestString, TestConvertFunctions)
{
    std::string stdString;

    String royStr ("Hello world");

    stdString = royStr.toStdString();
    ASSERT_EQ (royStr, stdString);

    String royStr2;
    royStr2 = String::fromCArray ("Hi fish");
    ASSERT_EQ (royStr2, "Hi fish");

    String royStr3 (String::fromStdString (stdString));
    ASSERT_EQ (royStr3, "Hello world");

    String royStr4 (String::fromCArray (royStr3.c_str()));
    String royStr5 (royStr3.c_str());
    ASSERT_EQ (royStr4, royStr5);

    std::string stdString2 (String::toStdString (String ("My Kingdom")));
    ASSERT_EQ (stdString2, "My Kingdom");

    ASSERT_EQ (String::fromUInt (1234), "1234");
    ASSERT_EQ (String ("1234"), "1234");
    ASSERT_EQ (String::fromUInt (1234), String::fromAny<int> (1234));

    WString royWString (L"1234");
    ASSERT_EQ (royWString, WString::fromAny<int> (1234));

    std::string abc;
    ASSERT_EQ (abc, "");

    std::string stdString3 (String::toStdString (String()));
    ASSERT_EQ (stdString3.empty(), true);

    String test;
    ASSERT_EQ (test.empty(), true);
    ASSERT_EQ (test, "");

    std::string stdString4 (test.toStdString());
    ASSERT_EQ (stdString4.empty(), true);
    ASSERT_EQ (stdString4, "");
}

TEST (TestString, TestPopBack)
{
    String str;
    ASSERT_EQ (str.length(), 0u);
    ASSERT_EQ (str.empty(), true);

    str.push_back ("Hello");
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 5u);
    ASSERT_EQ (str.length(), 5u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), 'o');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str, "Hello");

    str.push_back (' ');
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 6u);
    ASSERT_EQ (str.length(), 6u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), ' ');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str, "Hello ");

    str.push_back (String ("beautiful"));
    ASSERT_EQ (str.empty(), false);
    ASSERT_EQ (str.size(), 15u);
    ASSERT_EQ (str.length(), 15u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.back(), 'l');
    ASSERT_EQ (str.front(), 'H');
    ASSERT_EQ (str, "Hello beautiful");
    str.push_back (' ');
    ASSERT_EQ (str, "Hello beautiful ");

    str.pop_back();
    ASSERT_EQ (str, "Hello beautiful");

    str.pop_back();
    ASSERT_EQ (str, "Hello beautifu");
    str.push_back ("l ");

    str.reserve (100);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.length(), 16u);

    ASSERT_EQ (str.capacity(), 100u);
    str.resize (5);

    ASSERT_EQ (str.capacity(), 5u);

    str.clear();
    ASSERT_EQ (str.capacity(), 0u);
    ASSERT_EQ (str.length(), str.size());
    ASSERT_EQ (str.length(), 0u);

    String strNew;
    std::stringstream s;
    s << strNew;
    ASSERT_NO_THROW (strNew.clear());
}

namespace royaletest
{
    /**
    * This contains the file reader class.
    */
    class StringVectorTest
    {
    public:
        StringVectorTest()
        {
            m_supportedModes.push_back (0);
            m_supportedModes.push_back (1);
            m_supportedModes.push_back (2);
            m_supportedModes.push_back (3);
            m_supportedModes.push_back (4);
            m_supportedModes.push_back (5);
        }

        Vector<int32_t> &getUseCases()
        {
            return m_supportedModes;
        }

    private:
        Vector<int32_t> m_supportedModes;
    };

    const std::map<int32_t, const String> useCaseNames =
    {
        {
            { -1, "MODE_INVALID" },
            { 0, "MODE_9_5FPS_2000" },
            { 1, "MODE_9_10FPS_1000" },
            { 2, "MODE_9_15FPS_700" },
            { 3, "MODE_9_25FPS_450" },
            { 4, "MODE_5_35FPS_600" },
            { 5, "MODE_5_45FPS_500" },
            { 6, "MODE_5_45FPS_1000" },
            { 7, "MODE_5_35FPS_1000" },
            { 1000, "MODE_PLAYBACK" }
        }
    };

    String getUseCaseName (int32_t mode)
    {
        auto ucName = useCaseNames.find (mode);
        if (ucName == useCaseNames.end())
        {
            return "";
        }

        return ucName->second;
    }

}

TEST (TestString, TestUseCases)
{
    royaletest::StringVectorTest test;
    auto mode = test.getUseCases();
    for (size_t i = 0; i < mode.size(); ++i)
    {
        ASSERT_NO_THROW (royaletest::getUseCaseName (mode[i]));
    }
}

TEST (TestString, TestVectorOfString)
{
    Vector<String> stringVector =
    {
        "String1",
        "String2",
        "String3",
    };

    ASSERT_EQ (stringVector.capacity(), 3u);
    ASSERT_EQ (stringVector.size(), 3u);

    ASSERT_NO_THROW (stringVector.pop_back());
    ASSERT_EQ (stringVector.capacity(), 3u);
    ASSERT_EQ (stringVector.size(), 2u);

    ASSERT_EQ (stringVector.at (0), "String1");
    ASSERT_EQ (stringVector.at (1), "String2");

    ASSERT_NO_THROW (stringVector.at (0) = "String0");
    ASSERT_NO_THROW (stringVector[1] = "String1");

    ASSERT_EQ (stringVector.capacity(), 3u);
    ASSERT_EQ (stringVector.size(), 2u);

    ASSERT_EQ (stringVector.at (0), "String0");
    ASSERT_EQ (stringVector.at (1), "String1");

    ASSERT_NO_THROW (stringVector.push_back ("String2"));
    ASSERT_EQ (stringVector.capacity(), 3u);
    ASSERT_EQ (stringVector.size(), 3u);

    ASSERT_EQ (stringVector.at (0), "String0");
    ASSERT_EQ (stringVector.at (1), "String1");
    ASSERT_EQ (stringVector.at (2), "String2");

    ASSERT_ANY_THROW (stringVector.at (3));

    String string0 = stringVector.at (0);
    String string1 (stringVector.at (1));
    std::string string2 = stringVector.at (2).toStdString();

    ASSERT_EQ (string0, "String0");
    ASSERT_EQ (string1, "String1");
    ASSERT_EQ (string2, "String2");

    Vector<String> stringVector2 (stringVector);
    ASSERT_EQ (stringVector.size(), stringVector2.size());

    for (size_t i = 0; i < stringVector2.size(); ++i)
    {
        ASSERT_EQ (stringVector[i], stringVector2[i]);
    }

    auto ptrString0 = stringVector[0].data();
    auto ptrString1 = stringVector[1].data();
    auto ptrString2 = stringVector[2].data();

    ASSERT_EQ (strcmp (ptrString0, "String0") == 0, true);
    ASSERT_EQ (strcmp (ptrString1, "String1") == 0, true);
    ASSERT_EQ (strcmp (ptrString2, "String2") == 0, true);

    ASSERT_NO_THROW (stringVector.clear());

    ASSERT_EQ (string0, "String0");
    ASSERT_EQ (string1, "String1");
    ASSERT_EQ (string2, "String2");

    ASSERT_NO_THROW (stringVector.push_back (string0));
    ASSERT_NO_THROW (stringVector.push_back (string1));
    ASSERT_NO_THROW (stringVector.push_back (string2));

    ASSERT_NO_THROW (string0.append (" Hello there"));
    ASSERT_NO_THROW (string0.reserve (100));
    ASSERT_EQ (string0.length(), 19u);
    ASSERT_GE (string0.capacity(), string0.length());
    ASSERT_EQ (string0.capacity(), 100u);
    string0.shrink_to_fit();
    ASSERT_EQ (string0.capacity(), string0.length());
}


TEST (TestString, TestMoveStringObject)
{
    String royString1 ("Hello World");
    String royString2 (std::move (royString1));

    ASSERT_NE (royString1, royString2);
    ASSERT_EQ (royString2, "Hello World");

    String royString3 = std::move (royString2);
    ASSERT_NE (royString2, royString3);
    ASSERT_EQ (royString3, "Hello World");

    ASSERT_NO_THROW (royString1.clear());
    ASSERT_NO_THROW (royString2.clear());

    ASSERT_NO_THROW (royString2 = "Hello world");
    ASSERT_NE (royString2, royString3);
    ASSERT_EQ (royString3, "Hello World");
    ASSERT_EQ (royString2, "Hello world");

    ASSERT_NO_THROW (royString2.clear());
    ASSERT_NO_THROW (royString3.clear());

    ASSERT_NO_THROW (std::cout << royString1);
    ASSERT_NO_THROW (std::cout << royString2);
    ASSERT_NO_THROW (std::cout << royString3);
}

template <typename C>
std::string container_forward (const C &container)
{
    std::stringstream compareString;
    for (typename C::const_iterator it = container.begin();
            it != container.end(); ++it)
    {
        compareString << *it;
    }
    return compareString.str();
}

template <typename C>
std::string container_reverse (const C &container)
{
    std::stringstream compareString;
    for (typename C::const_reverse_iterator it = container.rbegin();
            it != container.rend(); ++it)
    {
        compareString << *it;
    }
    return compareString.str();
}

TEST (TestString, TestIterators)
{
    {
        std::stringstream streamCheck;
        String royString ("now step live...");
        std::string stdString ("now step live...");

        for (String::iterator it = royString.begin(); it != royString.end(); ++it)
        {
            streamCheck << *it;
        }
        ASSERT_EQ (royString, streamCheck.str());
        streamCheck.str (std::string());

        ASSERT_EQ (royString, container_forward (royString));

        for (String::const_iterator cit (royString.cbegin()); cit != royString.cend(); cit++)
        {
            streamCheck << *cit;
        }
        ASSERT_EQ (royString, streamCheck.str());
        streamCheck.str (std::string());

        String::iterator it (royString.begin());
        while (it != royString.end())
        {
            streamCheck << it.nextItem();
        }
        ASSERT_EQ (royString, streamCheck.str());
        streamCheck.str (std::string());

        String::iterator royIt (royString.begin());
        std::advance (royIt, 4);
        while (royIt != royString.end())
        {
            streamCheck << royIt.nextItem();
        }
        ASSERT_EQ (streamCheck.str(), "step live...");
        streamCheck.str (std::string());

        String::iterator royItNew = royString.begin();
        for (size_t i = 4; i < royString.length(); i++)
        {
            streamCheck << royItNew[i];
        }
        ASSERT_EQ (streamCheck.str(), "step live...");
        streamCheck.str (std::string());

        ASSERT_EQ (container_reverse (royString), "...evil pets won");

        for (String::reverse_iterator rit = royString.rbegin(); rit != royString.rend(); ++rit)
        {
            streamCheck << *rit;
        }
        ASSERT_EQ (streamCheck.str(), "...evil pets won");
        streamCheck.str (std::string());

        //Compiler checks - for copy ctors
        {
            // Royale
            {
                String::iterator it0 (royString.rbegin());                 // forward iterator may take reverse iterator
                (void) it0;
                String::const_iterator it1 (royString.rbegin());           // const forward iterator may take const reverse iterator
                (void) it1;
                String::const_iterator it2 (royString.begin());            // const forward iterator may take const forward iterator
                (void) it2;
                String::iterator it3 (royString.begin());                  // forward iterator may take forward iterator
                (void) it3;
                String::const_iterator it4 (royString.cbegin());           // const forward iterator may take strict const forward iterator
                (void) it4;

                String::reverse_iterator it5 (royString.begin());          // reverse iterator may take forward one
                (void) it5;
                String::const_reverse_iterator it6 (royString.begin());    // const reverse iterator may take const forward one
                (void) it6;
                String::const_reverse_iterator it7 (royString.rbegin());   // const reverse iterator may take const reverse one
                (void) it7;
                String::reverse_iterator it8 (royString.rbegin());         // reverse iterator may take reverse iterator
                (void) it8;
                String::const_reverse_iterator it9 (royString.crbegin());  // const reverse iterator may take const reverse iterator
                (void) it9;

                ASSERT_EQ (it3 == it4, true);

                ASSERT_EQ (it2 == it4, true);
                ASSERT_EQ (it8 == it9, true);

                ASSERT_EQ (it5 != it8, true);

                ASSERT_EQ (it2 != it4, false);
                ASSERT_EQ (it8 != it9, false);
            }

            // STL
            {
                //std::string::iterator it0(stdString.rbegin());                  // forward iterator may take reverse iterator
                //(void)it0;
                //std::string::const_iterator it1(stdString.rbegin());            // const forward iterator may take const reverse iterator
                //(void)it1;
                std::string::const_iterator it2 (stdString.begin());            // const forward iterator may take const forward iterator
                (void) it2;
                std::string::iterator it3 (stdString.begin());                  // forward iterator may take forward iterator
                (void) it3;
                std::string::const_iterator it4 (stdString.cbegin());           // const forward iterator may take strict const forward iterator
                (void) it4;

                std::string::reverse_iterator it5 (stdString.begin());          // reverse iterator may take forward one
                (void) it5;
                std::string::const_reverse_iterator it6 (stdString.begin());    // const reverse iterator may take const forward one
                (void) it6;
                std::string::const_reverse_iterator it7 (stdString.rbegin());   // const reverse iterator may take const reverse one
                (void) it7;
                std::string::reverse_iterator it8 (stdString.rbegin());         // reverse iterator may take reverse iterator
                (void) it8;
                std::string::const_reverse_iterator it9 (stdString.crbegin());  // const reverse iterator may take const reverse iterator
                (void) it9;
            }
        }

        {
            String::iterator it0 = royString.rbegin();                  // forward iterator may take reverse iterator
            (void) it0;
            String::const_iterator it1 = royString.rbegin();            // const forward iterator may take const reverse iterator
            (void) it1;
            String::const_iterator it2 = royString.begin();             // const forward iterator may take const forward iterator
            (void) it2;
            String::iterator it3 = royString.begin();                   // forward iterator may take forward iterator
            (void) it3;
            String::const_iterator it4 = royString.cbegin();            // const forward iterator may take strict const forward iterator
            (void) it4;

            String::reverse_iterator it5 = royString.begin();           // reverse iterator may take forward one
            (void) it5;
            String::const_reverse_iterator it6 = royString.begin();     // const reverse iterator may take const forward one
            (void) it6;
            String::const_reverse_iterator it7 = royString.rbegin();    // const reverse iterator may take const reverse one
            (void) it7;
            String::reverse_iterator it8 = royString.rbegin();          // reverse iterator may take reverse iterator
            (void) it8;
            String::const_reverse_iterator it9 = royString.crbegin();   // const reverse iterator may take const reverse iterator
            (void) it9;
        }


        for (String::iterator rit = royString.rbegin().base(); rit != royString.rend().base(); rit = rit.prev())
        {
            streamCheck << *rit;
        }
        ASSERT_EQ (streamCheck.str(), "...evil pets won");
        streamCheck.str (std::string());

        size_t index = royString.length() - 1;
        for (String::reverse_iterator rit = royString.rbegin(); rit != royString.rend(); ++rit)
        {
            streamCheck << *rit;
            ASSERT_EQ (royString.indexFromIterator (rit), index);
            index--;
        }
        ASSERT_EQ (streamCheck.str(), "...evil pets won");
        streamCheck.str (std::string());

        for (String::const_reverse_iterator it = royString.crbegin() + 8; it != royString.crend(); ++it)
        {
            streamCheck << *it;
        }
        ASSERT_EQ (streamCheck.str(), "pets won");
        streamCheck.str (std::string());

        for (String::iterator it = royString.end() - 1; it != royString.begin() - 1; --it)
        {
            streamCheck << *it;
        }
        ASSERT_EQ (streamCheck.str(), "...evil pets won");
        streamCheck.str (std::string());

        for (std::string::reverse_iterator it = stdString.rbegin() + 8; it != stdString.rend(); ++it)
        {
            streamCheck << *it;
        }
        ASSERT_EQ (streamCheck.str(), "pets won");
        streamCheck.str (std::string());
    }

    {
        std::stringstream streamCheck;
        const String royString ("now step live...");
        const std::string stdString ("now step live...");

        for (String::const_iterator it = royString.cbegin(); it != royString.cend(); ++it)
        {
            streamCheck << *it;
        }
        ASSERT_EQ (royString, streamCheck.str());
        streamCheck.str (std::string());

        size_t index = royString.length() - 1;
        for (String::const_reverse_iterator rit = royString.crbegin(); rit != royString.crend(); rit++)
        {
            streamCheck << *rit;
            ASSERT_EQ (royString.indexFromIterator (rit), index);
            index--;
        }
        ASSERT_EQ (streamCheck.str(), "...evil pets won");
        streamCheck.str (std::string());
    }

}

TEST (TestString, TestMemory)
{
    {
        String checkString2 ("Hello World #");
        String checkString (checkString2);
        checkString += String ("6");
        ASSERT_EQ (checkString, "Hello World #6");

        String checkStringNew;
        checkStringNew += 'A';
        ASSERT_EQ (checkStringNew, "A");

        checkStringNew.append ('B');
        ASSERT_EQ (checkStringNew, "AB");

        String testString = String().append ("abcdef", 3);
        ASSERT_EQ (testString, "abc");

        String testString2;
        testString2 = testString2 + 'A';
        ASSERT_EQ (testString2, "A");

        testString = testString + 'A' + testString2;
        ASSERT_EQ (testString, "abcAA");
    }
}

TEST (TestString, TestStringReplace)
{
    // iterator driven
    {
        // replace without buffer overrun / new allocation
        {
            String royString ("123456");
            royString.reserve (10);
            String royString2 ("XXXXXX");

            royString.replace (royString.end() - 2, royString2.begin(), royString2.begin() + 5);

            ASSERT_EQ (royString.at (0), '1');
            ASSERT_EQ (royString.at (1), '2');
            ASSERT_EQ (royString.at (2), '3');
            ASSERT_EQ (royString.at (3), '4');
            ASSERT_EQ (royString.at (4), 'X');
            ASSERT_EQ (royString.at (5), 'X');
            ASSERT_EQ (royString.at (6), 'X');
            ASSERT_EQ (royString.at (7), 'X');
            ASSERT_EQ (royString.at (8), 'X');

            ASSERT_EQ (royString.size(), 9u);
            ASSERT_GE (royString.capacity(), 9u);
        }

        // replace without buffer overrun / new allocation
        {
            String royString ("123456");
            String royString2 ("abcdef");

            royString.replace (royString.end() - 2, royString2.rbegin(), royString2.rbegin() + 5);

            ASSERT_EQ (royString.at (0), '1');
            ASSERT_EQ (royString.at (1), '2');
            ASSERT_EQ (royString.at (2), '3');
            ASSERT_EQ (royString.at (3), '4');
            ASSERT_EQ (royString.at (4), 'f');
            ASSERT_EQ (royString.at (5), 'e');
            ASSERT_EQ (royString.at (6), 'd');
            ASSERT_EQ (royString.at (7), 'c');
            ASSERT_EQ (royString.at (8), 'b');

            ASSERT_EQ (royString.size(), 9u);
            ASSERT_GE (royString.capacity(), 9u);
        }

        // replace without buffer overrun / new allocation
        {
            String royString ("123456");
            String royString2 ("abcdef");

            royString.replace (royString.begin() + 1, royString.end() - 2, royString2.rbegin(), royString2.rend());

            ASSERT_EQ (royString.at (0), '1');
            ASSERT_EQ (royString.at (1), 'f');
            ASSERT_EQ (royString.at (2), 'e');
            ASSERT_EQ (royString.at (3), 'd');
            ASSERT_EQ (royString.at (4), '5');
            ASSERT_EQ (royString.at (5), '6');

            ASSERT_EQ (royString.size(), 6u);
            ASSERT_EQ (royString.capacity(), 6u);
        }

        // replace without buffer overrun / new allocation
        {
            String royString ("123456");
            royString.reserve (10);

            String royString2 ("XXXXXX");
            royString2.reserve (6);

            ASSERT_EQ (royString.capacity(), 10u);
            royString.replace (royString.end() - 2, royString2);
            ASSERT_EQ (royString.capacity(), 10u);

            ASSERT_EQ (royString.at (0), '1');
            ASSERT_EQ (royString.at (1), '2');
            ASSERT_EQ (royString.at (2), '3');
            ASSERT_EQ (royString.at (3), '4');
            ASSERT_EQ (royString.at (4), 'X');
            ASSERT_EQ (royString.at (5), 'X');
            ASSERT_EQ (royString.at (6), 'X');
            ASSERT_EQ (royString.at (7), 'X');
            ASSERT_EQ (royString.at (8), 'X');
            ASSERT_EQ (royString.at (9), 'X');

            ASSERT_EQ (royString.size(), 10u);
            ASSERT_EQ (royString.capacity(), 10u);
        }

        // replace with buffer reallocation
        {
            String royString ("123456");
            royString.reserve (6);

            String royString2 ("XXXXXX");
            royString2.reserve (6);

            royString.replace (royString.end() - 2, royString2.begin(), royString2.begin() + 5);

            ASSERT_EQ (royString.at (0), '1');
            ASSERT_EQ (royString.at (1), '2');
            ASSERT_EQ (royString.at (2), '3');
            ASSERT_EQ (royString.at (3), '4');
            ASSERT_EQ (royString.at (4), 'X');
            ASSERT_EQ (royString.at (5), 'X');
            ASSERT_EQ (royString.at (6), 'X');
            ASSERT_EQ (royString.at (7), 'X');
            ASSERT_EQ (royString.at (8), 'X');

            ASSERT_EQ (royString.size(), 9u);
            ASSERT_GE (royString.capacity(), 9u);
        }

        // replace with buffer reallocation
        {
            String royString ("123456");
            royString.reserve (6);

            String royString2 ("abcdef");

            royString.replace (royString.end() - 2, royString2.rbegin(), royString2.rbegin() + 5);

            ASSERT_EQ (royString.at (0), '1');
            ASSERT_EQ (royString.at (1), '2');
            ASSERT_EQ (royString.at (2), '3');
            ASSERT_EQ (royString.at (3), '4');
            ASSERT_EQ (royString.at (4), 'f');
            ASSERT_EQ (royString.at (5), 'e');
            ASSERT_EQ (royString.at (6), 'd');
            ASSERT_EQ (royString.at (7), 'c');
            ASSERT_EQ (royString.at (8), 'b');

            ASSERT_EQ (royString.size(), 9u);
            ASSERT_GE (royString.capacity(), 9u);
        }

        // replace without buffer reallocation
        {
            String royString ("123456");
            String royString2 ("XXXXXX");

            royString.replace (royString.end() - 2, royString.end(), royString2.begin(), royString2.begin() + 5);

            ASSERT_EQ (royString.at (0), '1');
            ASSERT_EQ (royString.at (1), '2');
            ASSERT_EQ (royString.at (2), '3');
            ASSERT_EQ (royString.at (3), '4');
            ASSERT_EQ (royString.at (4), 'X');
            ASSERT_EQ (royString.at (5), 'X');

            ASSERT_EQ (royString.size(), 6u);
            ASSERT_GE (royString.capacity(), 6u);
        }

        // replace without buffer reallocation
        {
            String royString ("123456");
            String royString2 ("XXXXXX");

            royString.replace (royString.size() - 2, royString2.begin(), royString2.end());

            ASSERT_EQ (royString.at (0), '1');
            ASSERT_EQ (royString.at (1), '2');
            ASSERT_EQ (royString.at (2), '3');
            ASSERT_EQ (royString.at (3), '4');
            ASSERT_EQ (royString.at (4), 'X');
            ASSERT_EQ (royString.at (5), 'X');
            ASSERT_EQ (royString.at (6), 'X');
            ASSERT_EQ (royString.at (7), 'X');
            ASSERT_EQ (royString.at (8), 'X');
            ASSERT_EQ (royString.at (9), 'X');

            ASSERT_EQ (royString.size(), 10u);
            ASSERT_GE (royString.capacity(), 10u);
        }
    }

    // index driven
    {
        // replace without buffer overrun / new allocation
        {
            String royString ("123456");
            royString.reserve (10);
            String royString2 ("XXXXXX");

            royString.replace (royString.size() - 2, royString2);

            ASSERT_EQ (royString.at (0), '1');
            ASSERT_EQ (royString.at (1), '2');
            ASSERT_EQ (royString.at (2), '3');
            ASSERT_EQ (royString.at (3), '4');
            ASSERT_EQ (royString.at (4), 'X');
            ASSERT_EQ (royString.at (5), 'X');
            ASSERT_EQ (royString.at (6), 'X');
            ASSERT_EQ (royString.at (7), 'X');
            ASSERT_EQ (royString.at (8), 'X');
            ASSERT_EQ (royString.at (9), 'X');

            ASSERT_EQ (royString.size(), 10u);
            ASSERT_EQ (royString.capacity(), 10u);
        }

        // replace with buffer reallocation
        {
            String royString ("123456");
            royString.reserve (10);
            String royString2 ("XXXXXX");

            royString.replace (royString.size() - 2, royString2.begin(), royString2.begin() + 5);

            ASSERT_EQ (royString.at (0), '1');
            ASSERT_EQ (royString.at (1), '2');
            ASSERT_EQ (royString.at (2), '3');
            ASSERT_EQ (royString.at (3), '4');
            ASSERT_EQ (royString.at (4), 'X');
            ASSERT_EQ (royString.at (5), 'X');
            ASSERT_EQ (royString.at (6), 'X');
            ASSERT_EQ (royString.at (7), 'X');
            ASSERT_EQ (royString.at (8), 'X');

            ASSERT_EQ (royString.size(), 9u);
            ASSERT_GE (royString.capacity(), 9u);
        }
    }
}

TEST (TestString, TestStringInsert)
{
    {
        // no realloc
        String royString ("12349");
        royString.reserve (10);

        String royString2 ("5678");

        royString.insert (royString.end() - 1, royString2.begin(), royString2.end());

        ASSERT_EQ (royString.at (0), '1');
        ASSERT_EQ (royString.at (1), '2');
        ASSERT_EQ (royString.at (2), '3');
        ASSERT_EQ (royString.at (3), '4');
        ASSERT_EQ (royString.at (4), '5');
        ASSERT_EQ (royString.at (5), '6');
        ASSERT_EQ (royString.at (6), '7');
        ASSERT_EQ (royString.at (7), '8');
        ASSERT_EQ (royString.at (8), '9');
    }

    {
        String royString ("123414");
        royString.reserve (6);
        String royString2 ("23");

        royString.insert (royString.end() - 1, royString2.begin(), royString2.end());

        ASSERT_EQ (royString.at (0), '1');
        ASSERT_EQ (royString.at (1), '2');
        ASSERT_EQ (royString.at (2), '3');
        ASSERT_EQ (royString.at (3), '4');
        ASSERT_EQ (royString.at (4), '1');
        ASSERT_EQ (royString.at (5), '2');
        ASSERT_EQ (royString.at (6), '3');
        ASSERT_EQ (royString.at (7), '4');
    }

    // insert with new allocation
    // replace with buffer reallocation
    {
        String royString ("123456");
        royString.reserve (6);

        String royString2 ("XXXXXX");
        royString2.reserve (6);

        royString.insert (royString.end() - 2, royString2.begin(), royString2.begin() + 5);

        ASSERT_EQ (royString.at (0), '1');
        ASSERT_EQ (royString.at (1), '2');
        ASSERT_EQ (royString.at (2), '3');
        ASSERT_EQ (royString.at (3), '4');
        ASSERT_EQ (royString.at (4), 'X');
        ASSERT_EQ (royString.at (5), 'X');
        ASSERT_EQ (royString.at (6), 'X');
        ASSERT_EQ (royString.at (7), 'X');
        ASSERT_EQ (royString.at (8), 'X');
        ASSERT_EQ (royString.at (9), '5');
        ASSERT_EQ (royString.at (10), '6');

        ASSERT_EQ (royString.size(), 11u);
        ASSERT_GT (royString.capacity(), 11u);
    }

    {
        // Realloc neccessary
        String royString ("14");
        String firstRoyString (royString);
        String royString2 ("23");
        String secondRoyString (royString2);

        size_t oldCap = royString.capacity();

        royString.insert (royString.begin() + 1, royString2.begin(), royString2.end());

        ASSERT_NE (oldCap, royString.capacity());

        ASSERT_EQ (royString.at (0), '1');
        ASSERT_EQ (royString.at (1), '2');
        ASSERT_EQ (royString.at (2), '3');
        ASSERT_EQ (royString.at (3), '4');
        ASSERT_EQ (royString, "1234");

        royString.reserve (20);

        royString.insert (royString.end(), firstRoyString.begin(), firstRoyString.end());

        ASSERT_EQ (royString.at (0), '1');
        ASSERT_EQ (royString.at (1), '2');
        ASSERT_EQ (royString.at (2), '3');
        ASSERT_EQ (royString.at (3), '4');
        ASSERT_EQ (royString.at (4), '1');
        ASSERT_EQ (royString.at (5), '4');
        ASSERT_EQ (royString, "123414");

        royString.insert (royString.end() - 1, secondRoyString.begin(), secondRoyString.end());

        ASSERT_EQ (royString.at (0), '1');
        ASSERT_EQ (royString.at (1), '2');
        ASSERT_EQ (royString.at (2), '3');
        ASSERT_EQ (royString.at (3), '4');
        ASSERT_EQ (royString.at (4), '1');
        ASSERT_EQ (royString.at (5), '2');
        ASSERT_EQ (royString.at (6), '3');
        ASSERT_EQ (royString.at (7), '4');
        ASSERT_EQ (royString, "12341234");

        royString.insert (royString.begin(), firstRoyString.rbegin(), firstRoyString.rend());

        ASSERT_EQ (royString.at (0), '4');
        ASSERT_EQ (royString.at (1), '1');
        ASSERT_EQ (royString.at (2), '1');
        ASSERT_EQ (royString.at (3), '2');
        ASSERT_EQ (royString.at (4), '3');
        ASSERT_EQ (royString.at (5), '4');
        ASSERT_EQ (royString.at (6), '1');
        ASSERT_EQ (royString.at (7), '2');
        ASSERT_EQ (royString.at (8), '3');
        ASSERT_EQ (royString.at (9), '4');
        ASSERT_EQ (royString, "4112341234");

        royString.insert (royString.begin() + 1, secondRoyString.rbegin(), secondRoyString.rend());

        // Added reverse order at begin
        ASSERT_EQ (royString.at (0), '4');
        ASSERT_EQ (royString.at (1), '3');
        ASSERT_EQ (royString.at (2), '2');
        ASSERT_EQ (royString.at (3), '1');
        ASSERT_EQ (royString.at (4), '1');
        ASSERT_EQ (royString.at (5), '2');
        ASSERT_EQ (royString.at (6), '3');
        ASSERT_EQ (royString.at (7), '4');
        ASSERT_EQ (royString.at (8), '1');
        ASSERT_EQ (royString.at (9), '2');
        ASSERT_EQ (royString.at (10), '3');
        ASSERT_EQ (royString.at (11), '4');
        ASSERT_EQ (royString, "432112341234");

        royString.insert (royString.end(), 'X');
        ASSERT_EQ (royString, "432112341234X");
    }

    // Test with indicies
    {
        {
            String royString ("12349");
            royString.reserve (10);

            String royString2 ("5678");

            royString.insert (royString.size() - 1, royString2);

            ASSERT_EQ (royString.at (0), '1');
            ASSERT_EQ (royString.at (1), '2');
            ASSERT_EQ (royString.at (2), '3');
            ASSERT_EQ (royString.at (3), '4');
            ASSERT_EQ (royString.at (4), '5');
            ASSERT_EQ (royString.at (5), '6');
            ASSERT_EQ (royString.at (6), '7');
            ASSERT_EQ (royString.at (7), '8');
            ASSERT_EQ (royString.at (8), '9');
            ASSERT_EQ (royString, "123456789");

            royString.insert (royString.size(), "10");
            ASSERT_EQ (royString.at (9), '1');
            ASSERT_EQ (royString.at (10), '0');
            ASSERT_EQ (royString.size(), 11u);
            ASSERT_EQ (royString, "12345678910");
        }

        {
            String royString ("12349");
            royString.reserve (12);

            String royString2 ("5678");

            royString.insert (royString.size() - 1, royString2, 0, royString2.length());

            ASSERT_EQ (royString.at (0), '1');
            ASSERT_EQ (royString.at (1), '2');
            ASSERT_EQ (royString.at (2), '3');
            ASSERT_EQ (royString.at (3), '4');

            ASSERT_EQ (royString.at (4), '5');
            ASSERT_EQ (royString.at (5), '6');
            ASSERT_EQ (royString.at (6), '7');
            ASSERT_EQ (royString.at (7), '8');

            ASSERT_EQ (royString.at (8), '9');

            royString.insert (royString.size(), "10");
            ASSERT_EQ (royString.at (9), '1');
            ASSERT_EQ (royString.at (10), '0');
            ASSERT_EQ (royString.size(), 11u);

            royString.insert (royString.size(), royString2, 2, 2);
            ASSERT_EQ (royString.at (11), '7');
            ASSERT_EQ (royString.at (12), '8');
            ASSERT_EQ (royString, "1234567891078");
            ASSERT_EQ (royString.size(), 13u);
            ASSERT_GE (royString.capacity(), 13u);
        }

        {
            {
                String royString ("12349");
                royString.reserve (10);
                String royString2 ("78");

                royString.insert (royString.size() - 1, royString2);
                royString.insert (royString.begin() + 4, { '5', '6' });

                ASSERT_EQ (royString.at (0), '1');
                ASSERT_EQ (royString.at (1), '2');
                ASSERT_EQ (royString.at (2), '3');
                ASSERT_EQ (royString.at (3), '4');

                ASSERT_EQ (royString.at (4), '5');
                ASSERT_EQ (royString.at (5), '6');
                ASSERT_EQ (royString.at (6), '7');
                ASSERT_EQ (royString.at (7), '8');

                ASSERT_EQ (royString.at (8), '9');

                royString.insert (royString.size(), 2, 'X');
                ASSERT_EQ (royString.at (9), 'X');
                ASSERT_EQ (royString.at (10), 'X');
                ASSERT_EQ (royString.size(), 11u);
                ASSERT_GE (royString.capacity(), 11u);
                ASSERT_EQ (royString, "123456789XX");

                royString.insert (royString.end(), 2, 'Y');
                ASSERT_EQ (royString.at (11), 'Y');
                ASSERT_EQ (royString.at (12), 'Y');
                ASSERT_EQ (royString.size(), 13u);
                ASSERT_GE (royString.capacity(), 13u);
                ASSERT_EQ (royString, "123456789XXYY");

                royString.insert (royString.size(), "Hello World around me", 11u);
                ASSERT_EQ (royString.size(), 24u);
                ASSERT_GE (royString.capacity(), 24u);
                ASSERT_EQ (royString, "123456789XXYYHello World");

                royString.insert (0, "Hello World");
                ASSERT_EQ (royString.size(), 35u);
                ASSERT_GE (royString.capacity(), 35u);
                ASSERT_EQ (royString, "Hello World123456789XXYYHello World");
            }
        }
    }
}

TEST (TestString, TestStringIndexIteratorConversions)
{
    // reverse iterator
    {
        String royString ("123456");
        royString.resize (10);

        ASSERT_EQ (*royString.iteratorFromIndex (2), '3');
        ASSERT_EQ (*royString.iteratorFromIndex (5), '6');

        ASSERT_EQ (* (royString.begin() + 2), '3');
        ASSERT_EQ (* (royString.begin() + 5), '6');

        ASSERT_EQ (royString.indexFromIterator (royString.begin() + 2), 2u);
        ASSERT_EQ (royString.indexFromIterator (royString.begin() + 5), 5u);

        ASSERT_EQ (royString.indexFromIterator (royString.end()), royString.size());

        String::reverse_iterator revIter (royString.reverseIteratorFromIndex (2));
        ASSERT_EQ (*revIter, '3');
        ASSERT_EQ (*++revIter, '2');
        ASSERT_EQ (*--revIter, '3');
        ASSERT_EQ (* (revIter + 1), '2');
        ASSERT_EQ (* (revIter - 1), '4');

        ASSERT_EQ (*revIter.next(), '2');
        ASSERT_EQ (*revIter.prev(), '4');

        ASSERT_EQ (revIter.nextItem(), '3');
        ASSERT_EQ (revIter.prevItem(), '2');

        ASSERT_EQ (*revIter, * (royString.data() + 2));
        ASSERT_EQ (*revIter.base(), * (royString.data() + 2));

        revIter -= 3;

        ASSERT_EQ (*revIter, '6');
        revIter = royString.reverseIteratorFromIndex (2);
        ASSERT_EQ (*revIter, '3');
        revIter += 1;
        ASSERT_EQ (*revIter, '2');
        ASSERT_EQ (revIter, royString.reverseIteratorFromIndex (1));
        ASSERT_EQ (revIter, royString.iteratorFromIndex (1));
        ASSERT_EQ (revIter, royString.reverseIteratorFromIndex (1).base());

        --revIter;

        String::iterator forwardIter;
        forwardIter = revIter.base();

        ASSERT_EQ (*forwardIter, '3');
        ASSERT_EQ (*forwardIter++, '3');
        ASSERT_EQ (*forwardIter--, '4');
        ASSERT_EQ (* (forwardIter + 1), '4');
        ASSERT_EQ (* (forwardIter - 1), '2');

        ASSERT_EQ (*forwardIter.next(), '4');
        ASSERT_EQ (*forwardIter.prev(), '2');

        ASSERT_EQ (forwardIter.nextItem(), '3');
        ASSERT_EQ (forwardIter.prevItem(), '4');

        ASSERT_EQ (*forwardIter, * (royString.data() + 2));

        forwardIter += 3;

        ASSERT_EQ (*forwardIter, '6');
        forwardIter = royString.iteratorFromIndex (2);
        ASSERT_EQ (*forwardIter, '3');
        forwardIter -= 1;
        ASSERT_EQ (*forwardIter, '2');
        ASSERT_EQ (forwardIter, royString.reverseIteratorFromIndex (1));
        ASSERT_EQ (forwardIter, royString.iteratorFromIndex (1));
        ASSERT_EQ (forwardIter, royString.reverseIteratorFromIndex (1).base());
    }

    // const reverse iterator
    {
        String royString ("123456");
        royString.resize (10);

        const String &s = royString;

        ASSERT_EQ (*s.iteratorFromIndex (2), '3');
        ASSERT_EQ (*s.iteratorFromIndex (5), '6');

        ASSERT_EQ (* (s.begin() + 2), '3');
        ASSERT_EQ (* (s.begin() + 5), '6');

        ASSERT_EQ (s.indexFromIterator (s.begin() + 2), 2u);
        ASSERT_EQ (s.indexFromIterator (s.begin() + 5), 5u);

        ASSERT_EQ (s.indexFromIterator (s.end()), s.size());

        String::const_reverse_iterator revIter (s.reverseIteratorFromIndex (2));
        ASSERT_EQ (*revIter, '3');
        ASSERT_EQ (*revIter++, '3');
        ASSERT_EQ (*revIter--, '2');
        ASSERT_EQ (* (revIter + 1), '2');
        ASSERT_EQ (* (revIter - 1), '4');

        ASSERT_EQ (*revIter.next(), '2');
        ASSERT_EQ (*revIter.prev(), '4');

        ASSERT_EQ (revIter.nextItem(), '3');
        ASSERT_EQ (revIter.prevItem(), '2');

        ASSERT_EQ (*revIter, * (s.data() + 2));
        ASSERT_EQ (*revIter.base(), * (s.data() + 2));

        revIter -= 3;

        ASSERT_EQ (*revIter, '6');
        revIter = s.reverseIteratorFromIndex (2);
        ASSERT_EQ (*revIter, '3');
        revIter += 1;
        ASSERT_EQ (*revIter, '2');
        ASSERT_EQ (revIter, s.reverseIteratorFromIndex (1));
        ASSERT_EQ (revIter, s.iteratorFromIndex (1));
        ASSERT_EQ (revIter, s.reverseIteratorFromIndex (1).base());

        revIter--;

        String::const_iterator forwardIter (s.iteratorFromIndex (2));
        ASSERT_EQ (*forwardIter, '3');
        ASSERT_EQ (*++forwardIter, '4');
        ASSERT_EQ (*--forwardIter, '3');
        ASSERT_EQ (* (forwardIter + 1), '4');
        ASSERT_EQ (* (forwardIter - 1), '2');

        ASSERT_EQ (*forwardIter.next(), '4');
        ASSERT_EQ (*forwardIter.prev(), '2');

        ASSERT_EQ (forwardIter.nextItem(), '3');
        ASSERT_EQ (forwardIter.prevItem(), '4');

        ASSERT_EQ (*forwardIter, * (s.data() + 2));

        forwardIter += 3;

        ASSERT_EQ (*forwardIter, '6');
        forwardIter = s.iteratorFromIndex (2);
        ASSERT_EQ (*forwardIter, '3');
        forwardIter -= 1;
        ASSERT_EQ (*forwardIter, '2');
        ASSERT_EQ (forwardIter, s.reverseIteratorFromIndex (1));
        ASSERT_EQ (forwardIter, s.iteratorFromIndex (1));
        ASSERT_EQ (forwardIter, s.reverseIteratorFromIndex (1).base());
    }
}

TEST (TestString, TestSubString)
{
    String helloWorld ("Hello World");
    String b = helloWorld.substr (6u, 100u);
    String c = helloWorld.substr (6u, helloWorld.length());
    String d = helloWorld.substr (6u, 5u);

    ASSERT_EQ (b, c);
    ASSERT_EQ (b, d);
    ASSERT_EQ (c, d);

    ASSERT_THROW (helloWorld.substr (11u, 100u), std::out_of_range);
}

TEST (TestString, TestStringOstream)
{
    String ostreamTestString ("This output is a test");
    std::stringstream s;
    s << ostreamTestString;
    ASSERT_EQ (ostreamTestString, s.str());
}

TEST (TestString, TestRemove)
{
    // Royale
    {
        {
            String helloWorld ("Hello World");
            helloWorld.remove (helloWorld.cbegin() + 5, helloWorld.cend() + 2);
            ASSERT_EQ (helloWorld, "Hello");
        }

        {
            String helloWorld ("Hello World");
            helloWorld.remove (helloWorld.cbegin() + 5, helloWorld.cend());
            ASSERT_EQ (helloWorld, "Hello");

            ASSERT_NO_THROW (ASSERT_EQ (helloWorld.remove(), ""));

            ASSERT_EQ (helloWorld.empty(), true);
            ASSERT_NO_THROW (helloWorld.remove());
            ASSERT_EQ (helloWorld.length(), 0u);
        }

        {
            String helloWorld ("Hello World");
            helloWorld.remove (helloWorld.cbegin(), helloWorld.cbegin() + 1);
            ASSERT_EQ (helloWorld, "ello World"); //

            helloWorld.remove (helloWorld.cend() - 1);
            ASSERT_EQ (helloWorld, "ello Worl");

            helloWorld.remove (helloWorld.cbegin());
            ASSERT_EQ (helloWorld, "llo Worl");

            helloWorld.remove (helloWorld.end() - 1);
            ASSERT_EQ (helloWorld, "llo Wor");

            helloWorld.remove (helloWorld.begin());
            ASSERT_EQ (helloWorld, "lo Wor");

            helloWorld.remove (helloWorld.begin() + 2, helloWorld.begin() + 3);
            ASSERT_EQ (helloWorld, "loWor");

            helloWorld.remove (helloWorld.begin(), 2);
            ASSERT_EQ (helloWorld, "Wor");

            helloWorld.remove (1, 2);
            ASSERT_EQ (helloWorld, "W");
        }
    }
}

TEST (TestString, TestErase)
{
    // Royale
    {
        {
            String helloWorld ("Hello World");
            helloWorld.erase (helloWorld.cbegin() + 5, helloWorld.cend());
            ASSERT_EQ (helloWorld, "Hello");

            ASSERT_NO_THROW (ASSERT_EQ (helloWorld.erase(), ""));

            ASSERT_EQ (helloWorld.empty(), true);
            ASSERT_NO_THROW (helloWorld.erase());
            ASSERT_NO_THROW (helloWorld.erase());
            ASSERT_EQ (helloWorld.length(), 0u);
        }

        {
            String helloWorld ("Hello World");
            helloWorld.erase (helloWorld.cbegin() + 4, helloWorld.cbegin() + 4 + 1);
            ASSERT_EQ (helloWorld, "Hell World");
        }

        {
            String helloWorld ("Hello World");
            helloWorld.erase (helloWorld.cbegin(), helloWorld.cbegin() + 1);
            ASSERT_EQ (helloWorld, "ello World"); //

            helloWorld.erase (helloWorld.cend() - 1);
            ASSERT_EQ (helloWorld, "ello Worl");

            helloWorld.erase (helloWorld.cbegin());
            ASSERT_EQ (helloWorld, "llo Worl");

            helloWorld.erase (helloWorld.end() - 1);
            ASSERT_EQ (helloWorld, "llo Wor");

            helloWorld.erase (helloWorld.begin());
            ASSERT_EQ (helloWorld, "lo Wor");

            helloWorld.erase (helloWorld.begin() + 2, helloWorld.begin() + 3);
            ASSERT_EQ (helloWorld, "loWor");

            helloWorld.erase (0, 2);
            ASSERT_EQ (helloWorld, "Wor");

            helloWorld.erase (1, 2);
            ASSERT_EQ (helloWorld, "W");
        }

        {
            String helloWorld ("Hello World");
            ASSERT_EQ (helloWorld.erase (0, 1u), "ello World");
            ASSERT_EQ (helloWorld.erase (helloWorld.size() - 1, 1u).erase (0, 3), "o Worl");

            ASSERT_EQ (helloWorld.erase (helloWorld.size() - 1, 3u), "o Wor");

            ASSERT_EQ (helloWorld.erase (0, 1u).erase (0, 1u), "Wor");
            ASSERT_EQ (helloWorld.erase (helloWorld.size() - 1 - 1, 2u), "W");
            ASSERT_EQ (helloWorld.size(), 1u);
        }
    }
}

TEST (TestString, TestCompare)
{
    // STL
    {
        std::string checkString ("Hello World #");
        ASSERT_EQ (checkString.compare (checkString), 0);

        ASSERT_EQ (checkString.compare (checkString.c_str()), 0);

        const char *testString = "Hello World";

        ASSERT_EQ (checkString.compare (0, strlen (testString), testString), 0);
        ASSERT_NE (checkString.compare (0, 5, testString, strlen (testString)), 0);

        std::string str1 ("green apple");
        std::string str2 ("red apple");
        ASSERT_EQ (str1.compare (6, 5, str2, 4, 5), 0);
        ASSERT_NE (str1.compare (6, 5, str2, 5, 5), 0);
        ASSERT_NE (str1.compare (7, 5, str2, 4, 5), 0);
        ASSERT_NE (str1.compare (6, 4, str2, 5, 4), 0);
        ASSERT_NE (str1.compare (5, 6, str2, 4, 5), 0);

        ASSERT_NE (str1.compare (str2), 0);
        ASSERT_EQ (str1.compare (6, 5, "apple"), 0);
        ASSERT_EQ (str2.compare (str2.size() - 5, 5, "apple"), 0);
    }

    // Royale
    {
        String checkString ("Hello World #");
        ASSERT_EQ (checkString.compare (checkString), 0);

        ASSERT_EQ (checkString.compare (checkString.c_str()), 0);

        const char *testString = "Hello World";

        ASSERT_EQ (checkString.compare (0, strlen (testString), testString), 0);
        ASSERT_NE (checkString.compare (0, 5, testString, strlen (testString)), 0);

        String str1 ("green apple");
        String str2 ("red apple");
        ASSERT_EQ (str1.compare (6, 5, str2, 4, 5), 0);
        ASSERT_NE (str1.compare (6, 5, str2, 5, 5), 0);
        ASSERT_NE (str1.compare (7, 5, str2, 4, 5), 0);
        ASSERT_NE (str1.compare (6, 4, str2, 5, 4), 0);
        ASSERT_NE (str1.compare (5, 6, str2, 4, 5), 0);
        ASSERT_THROW (str1.compare (5, 6, str2, str2.length(), 5), std::out_of_range);
        ASSERT_THROW (str1.compare (str1.length(), 6, str2, 4, 5), std::out_of_range);

        ASSERT_NE (str1.compare (str2), 0);
        ASSERT_EQ (str1.compare (6, 5, "apple"), 0);
        ASSERT_EQ (str2.compare (str2.size() - 5, 5, "apple"), 0);
    }
}

namespace royale
{
    namespace StringTest
    {
        size_t indexFromIterator (royale_const_iterator<std::random_access_iterator_tag, char> it, const String &str)
        {
            return (it - str.begin());
        }
    }
}

TEST (TestString, TestFind)
{
    // STL
    {
        std::string str ("There are two needles in this haystack with needles.");
        std::string str2 ("needle");

        // different member versions of find in the same order as above:
        std::size_t found = str.find (str2, 30u);
        ASSERT_EQ (found, 44u);

        found = str.find (str2);
        ASSERT_EQ (found, 14u);

        found = str.find ("needle", 30u);
        ASSERT_EQ (found, 44u);

        found = str.find (str2);
        ASSERT_EQ (found, 14u);

        found = str.find ("needles are small", found + 1, 6);
        ASSERT_EQ (found, 44u);

        found = str.find ("haystack");
        ASSERT_EQ (found, 30u);

        found = str.find ('.');
        ASSERT_EQ (found, 51u);

        // let's replace the first needle:
        //str.replace(str.find(str2), str2.length(), "preposition");
        //std::cout << str << '\n';
    }

    // ROYALE
    {
        String str ("There are two needles in this haystack with needles.");
        String str2 ("needle");

        // different member versions of find in the same order as above:
        std::size_t found = str.find (str2, 30u);
        ASSERT_EQ (found, 44u);

        found = str.find (str2);
        ASSERT_EQ (found, 14u);

        found = str.find ("needle", 30u);
        ASSERT_EQ (found, 44u);

        found = str.find (str2);
        ASSERT_EQ (found, 14u);

        found = str.find ("needles are small", str.length(), 6);
        ASSERT_EQ (found, String::npos);

        found = str.find ("do not have needles", 0, 2);
        ASSERT_EQ (found, String::npos);

        found = str.find ("needles are small", 15, 6);
        ASSERT_EQ (found, 44u);

        found = str.find ("haystack");
        ASSERT_EQ (found, 30u);

        found = str.find ('.');
        ASSERT_EQ (found, 51u);

        found = str.find ('!');
        ASSERT_EQ (found, String::npos);

        found = str.find ('!', str.length());
        ASSERT_EQ (found, String::npos);

        // let's replace the first needle:
        //str.replace(str.find(str2), str2.length(), "preposition");
        //std::cout << str << '\n';
    }
}

TEST (TestString, TestStringEdgeCases)
{
    String royString;
    ASSERT_EQ (royString.size(), 0u);
    ASSERT_EQ (royString.length(), 0u);
    ASSERT_EQ (royString.capacity(), 0u);

    royString.shrink_to_fit();
    ASSERT_EQ (royString.size(), 0u);
    ASSERT_EQ (royString.length(), 0u);
    ASSERT_EQ (royString.capacity(), 0u);

    royString.resize (100);
    ASSERT_EQ (royString.size(), 100u);
    ASSERT_EQ (royString.length(), 100u);
    ASSERT_GE (royString.capacity(), 100u);

    royString.shrink_to_fit();
    ASSERT_EQ (royString.size(), 100u);
    ASSERT_EQ (royString.length(), 100u);
    ASSERT_GE (royString.capacity(), 100u);

    royString.append ("a");
    ASSERT_EQ (royString.at (100), 'a');

    while (royString.length())
    {
        royString.pop_back();
    }
    ASSERT_EQ (royString.length(), 0u);


    String emptyString;
    ASSERT_THROW (emptyString.back(), std::out_of_range);
    ASSERT_THROW (emptyString.front(), std::out_of_range);

    const String cEmptyString;
    ASSERT_THROW (cEmptyString.back(), std::out_of_range);
    ASSERT_THROW (cEmptyString.front(), std::out_of_range);

    emptyString.erase (emptyString.begin(), emptyString.end());
    emptyString.remove (emptyString.begin(), emptyString.end());
}

TEST (TestString, TestStringAssign)
{
    {
        String royString ("123456");
        royString.resize (10);
        ASSERT_NO_THROW (royString.assign ({ 'a', 'b' }));
        ASSERT_EQ (royString.length(), 2u);
        ASSERT_EQ (royString, "ab");
        ASSERT_NE (royString, "a");

        ASSERT_EQ (royString.at (0), 'a');
        ASSERT_EQ (royString.at (1), 'b');
        ASSERT_GE (royString.capacity(), 2u);

        royString.shrink_to_fit();
        ASSERT_EQ (royString.length(), 2u);
        ASSERT_EQ (royString.capacity(), 2u);
    }

    {
        String royString ("123456");
        royString.resize (10);
        ASSERT_EQ (royString.length(), 10u);
        String royString2 ("abcd");

        ASSERT_NO_THROW (royString.assign (royString2));
        ASSERT_EQ (royString2.length(), 4u);
        ASSERT_GE (royString2.capacity(), 4u);
        ASSERT_EQ (royString2, "abcd");

        ASSERT_EQ (royString.at (0), 'a');
        ASSERT_EQ (royString.at (1), 'b');
        ASSERT_EQ (royString.at (2), 'c');
        ASSERT_EQ (royString.at (3), 'd');
        ASSERT_EQ (royString, "abcd");

        ASSERT_EQ (royString.length(), 4u);
        ASSERT_GE (royString.capacity(), 4u);
    }

    {
        String royString ("123456");
        royString.resize (10);
        ASSERT_EQ (royString.length(), 10u);
        String royString2 ("abcd");

        ASSERT_NO_THROW (royString.assign (std::move (royString2)));
        //ASSERT_EQ(royString2.length(), 0u);
        //ASSERT_EQ(royString2, "");

        ASSERT_EQ (royString.at (0), 'a');
        ASSERT_EQ (royString.at (1), 'b');
        ASSERT_EQ (royString.at (2), 'c');
        ASSERT_EQ (royString.at (3), 'd');
        ASSERT_EQ (royString, "abcd");

        ASSERT_EQ (royString.length(), 4u);
        ASSERT_GE (royString.capacity(), 4u);
    }

    {
        String royString ("123456");
        royString.resize (10);

        ASSERT_NO_THROW (royString.assign (3, 'X'));
        ASSERT_EQ (royString.at (0), 'X');
        ASSERT_EQ (royString.at (1), 'X');
        ASSERT_EQ (royString.at (2), 'X');

        ASSERT_EQ (royString.size(), 3u);
        ASSERT_GE (royString.capacity(), 3u);
    }

    {
        String royString;

        ASSERT_NO_THROW (royString.assign (100, 'X'));
        ASSERT_EQ (royString.at (0), 'X');
        ASSERT_EQ (royString.at (1), 'X');
        ASSERT_EQ (royString.at (2), 'X');

        ASSERT_EQ (royString.size(), 100u);
        ASSERT_GE (royString.capacity(), 100u);
    }

    {
        String royString;
        royString.reserve (100);

        ASSERT_NO_THROW (royString.assign (30, 'X'));
        ASSERT_EQ (royString.at (0), 'X');
        ASSERT_EQ (royString.at (1), 'X');
        ASSERT_EQ (royString.at (2), 'X');

        ASSERT_EQ (royString.size(), 30u);
        ASSERT_GE (royString.capacity(), 100u);
    }

    {
        String royString ("123456");
        ASSERT_NO_THROW (royString.assign ("Hello World of mine", 11));
        ASSERT_EQ (royString.length(), 11u);
        ASSERT_EQ (royString, "Hello World");

        ASSERT_GE (royString.capacity(), 11u);

        royString.shrink_to_fit();
        ASSERT_EQ (royString.length(), 11u);
        ASSERT_EQ (royString.capacity(), 11u);
    }

    {
        String royString ("123456");
        ASSERT_NO_THROW (royString.assign ("Hello World of mine"));
        ASSERT_EQ (royString.length(), 19u);
        ASSERT_EQ (royString, "Hello World of mine");

        ASSERT_GE (royString.capacity(), 19u);

        royString.shrink_to_fit();
        ASSERT_EQ (royString.length(), 19u);
        ASSERT_EQ (royString.capacity(), 19u);
    }

    {
        String royString ("123456");
        ASSERT_NO_THROW (royString.assign (std::string ("Hello World of mine"), 6u, royString.max_size()));
        ASSERT_EQ (royString.length(), 13u);
        ASSERT_EQ (royString, "World of mine");

        ASSERT_GE (royString.capacity(), 13u);

        royString.shrink_to_fit();
        ASSERT_EQ (royString.length(), 13u);
        ASSERT_EQ (royString.capacity(), 13u);
    }

    {
        String royString ("123456");
        ASSERT_THROW (royString.assign (std::string ("Hello World of mine"), 19u, 5u), std::out_of_range);
        ASSERT_NO_THROW (royString.assign (std::string ("Hello World of mine"), 6u, 5u));
        ASSERT_EQ (royString.length(), 5u);
        ASSERT_EQ (royString, "World");

        royString.shrink_to_fit();
        ASSERT_EQ (royString.length(), 5u);
        ASSERT_EQ (royString.capacity(), 5u);
    }

    {
        String royString ("123456");
        ASSERT_NO_THROW (royString.assign ("Hello World of mine", 6u, String::npos));
        ASSERT_EQ (royString.length(), 13u);
        ASSERT_EQ (royString, "World of mine");

        ASSERT_GE (royString.capacity(), 13u);

        royString.shrink_to_fit();
        ASSERT_EQ (royString.length(), 13u);
        ASSERT_EQ (royString.capacity(), 13u);
    }

    {
        String royString ("123456");
        ASSERT_THROW (royString.assign ("Hello World of mine", 19u, 5u), std::out_of_range);
        ASSERT_NO_THROW (royString.assign ("Hello World of mine", 6u, 5u));
        ASSERT_EQ (royString.length(), 5u);
        ASSERT_EQ (royString, "World");

        ASSERT_GE (royString.capacity(), 5u);

        royString.shrink_to_fit();
        ASSERT_EQ (royString.length(), 5u);
        ASSERT_EQ (royString.capacity(), 5u);
    }

    {
        String royString ("123456");
        String royString2 ("abcdef");
        ASSERT_NO_THROW (royString.assign (royString2.begin() + 2, royString2.begin() + 4));
        ASSERT_EQ (royString.length(), 2u);
        ASSERT_EQ (royString, "cd");

        ASSERT_GE (royString.capacity(), 2u);

        royString.shrink_to_fit();
        ASSERT_EQ (royString.capacity(), 2u);
    }

    {
        String royString;
        royString.resize (100);
        String royString2 ("abcdef");
        ASSERT_NO_THROW (royString.assign (royString2.begin() + 1, royString2.end()));
        ASSERT_EQ (royString.length(), 5u);
        ASSERT_EQ (royString, "bcdef");

        ASSERT_GE (royString.capacity(), 5u);

        royString.shrink_to_fit();
        ASSERT_EQ (royString.capacity(), 5u);
    }
}

TEST (TestString, CompareOperatorEmpty)
{
    String r;
    String s;
    ASSERT_EQ (r == s, true);
    ASSERT_EQ (s == r, true);
}

TEST (TestString, CompareOperatorNotEmpty)
{
    String r = "";
    String s = "xx";
    ASSERT_EQ (r == s, false);
    ASSERT_EQ (s == r, false);
}

TEST (TestString, CompareEmpty)
{
    String r;
    String s;
    ASSERT_EQ (r.compare (s), 0);
    ASSERT_EQ (s.compare (r), 0);
}

TEST (TestString, CompareNotEmpty)
{
    String r = "";
    String s = "xx";
    ASSERT_EQ (r.compare (s), 2);
    ASSERT_EQ (s.compare (r), -2);
}

TEST (TestString, EmptyStringData)
{
    String emptyString;
    ASSERT_NE (emptyString.data(), nullptr);
    ASSERT_NO_FATAL_FAILURE (emptyString.find ("TEST"));
}

TEST (TestString, DataIsNotSharedBetweenCopies)
{
    String original ("aaaa");
    String copy (original);
    ASSERT_EQ (String ("aaaa"), original);
    ASSERT_EQ (String ("aaaa"), copy);

    copy.at (1) = 'b';
    copy.at (2) = 'c';
    copy.at (3) = 'd';
    ASSERT_EQ (String ("aaaa"), original);
    ASSERT_EQ (String ("abcd"), copy);
}

TEST (TestString, MoveLeavesAValidState)
{
    // move construction or move assignment should leave the moved-from object in a valid state
    String original ("aaaa");
    String copy (std::move (original));
    ASSERT_EQ (String ("aaaa"), copy);

    ASSERT_NE (original.c_str(), nullptr);
}

TEST (TestString, TestStringStartsWith)
{
    String camName ("picoFlexx_1");
    ASSERT_EQ (stringStartsWith (camName, { "picoFlexx", "Daedalus", "Skylla" }), true);
    ASSERT_EQ (stringStartsWith (camName, { "Daedalus", "Skylla" }), false);
    ASSERT_EQ (stringStartsWith (camName, { "Daedalus", "picoFlexx", "Skylla" }), true);
    ASSERT_EQ (stringStartsWith (camName, {}), false);
    ASSERT_EQ (stringStartsWith ("", {}), false);
    ASSERT_EQ (stringStartsWith (camName, {}), false);
    ASSERT_EQ (stringStartsWith (camName, { "" }), true);
    ASSERT_EQ (stringStartsWith ("", { "" }), true);
}
