#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/Timeout.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/WrongState.hpp>
#include <common/exceptions/OutOfBounds.hpp>
#include <common/exceptions/ImagerConfigNotFoundError.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/ValidButUnchanged.hpp>
#include <gtest/gtest.h>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace royale::common;

TEST (TestExceptions, TestNotImplemented)
{
    try
    {
        throw NotImplemented();
    }
    catch (Exception &e)
    {
        EXPECT_EQ (e.getExceptionType(), "NotImplemented exception");
        EXPECT_EQ (e.getExceptionType(), NotImplemented().getExceptionType());
        EXPECT_EQ (e.getTechnicalDescription(), "not implemented");
        EXPECT_EQ (e.getStatus(), royale::CameraStatus::NOT_IMPLEMENTED);
    }
}

TEST (TestExceptions, TestLogicError)
{
    try
    {
        throw LogicError();
    }
    catch (Exception &e)
    {
        EXPECT_EQ (e.getExceptionType(), "LogicError exception");
        EXPECT_EQ (e.getExceptionType(), LogicError().getExceptionType());
        EXPECT_EQ (e.getTechnicalDescription(), "logic error");
        EXPECT_EQ (e.getStatus(), royale::CameraStatus::LOGIC_ERROR);
    }
}

TEST (TestExceptions, TestTimeout)
{
    try
    {
        throw Timeout();
    }
    catch (Exception &e)
    {
        EXPECT_EQ (e.getExceptionType(), "Timeout exception");
        EXPECT_EQ (e.getExceptionType(), Timeout().getExceptionType());
        EXPECT_EQ (e.getTechnicalDescription(), "timeout");
        EXPECT_EQ (e.getStatus(), royale::CameraStatus::TIMEOUT);
    }
}

TEST (TestExceptions, TestWrongState)
{
    try
    {
        throw WrongState();
    }
    catch (Exception &e)
    {
        EXPECT_EQ (e.getExceptionType(), "WrongState exception");
        EXPECT_EQ (e.getExceptionType(), WrongState().getExceptionType());
        EXPECT_EQ (e.getTechnicalDescription(), "wrong state");
        EXPECT_EQ (e.getStatus(), royale::CameraStatus::NOT_IMPLEMENTED);
    }
}

TEST (TestExceptions, TestRuntimeError)
{
    try
    {
        throw RuntimeError();
    }
    catch (Exception &e)
    {
        EXPECT_EQ (e.getExceptionType(), "RuntimeError exception");
        EXPECT_EQ (e.getExceptionType(), RuntimeError().getExceptionType());
        EXPECT_EQ (e.getTechnicalDescription(), "runtime error");
        EXPECT_EQ (e.getStatus(), royale::CameraStatus::RUNTIME_ERROR);
    }
}

TEST (TestExceptions, TestOutOfBounds)
{
    try
    {
        throw OutOfBounds();
    }
    catch (Exception &e)
    {
        EXPECT_EQ (e.getExceptionType(), "OutOfBounds exception");
        EXPECT_EQ (e.getExceptionType(), OutOfBounds().getExceptionType());
        EXPECT_EQ (e.getTechnicalDescription(), "out of bounds");
        EXPECT_EQ (e.getStatus(), royale::CameraStatus::OUT_OF_BOUNDS);
    }
}

TEST (TestExceptions, TestRecovered)
{
    try
    {
        throw ValidButUnchanged();
    }
    catch (Exception &e)
    {
        EXPECT_EQ (e.getExceptionType(), "ValidButUnchanged exception");
        EXPECT_EQ (e.getExceptionType(), ValidButUnchanged().getExceptionType());
        EXPECT_EQ (e.getTechnicalDescription(), "failed to perform a valid operation and recovered previous state");
        EXPECT_EQ (e.getStatus(), royale::CameraStatus::RUNTIME_ERROR);
    }
}

TEST (TestExceptions, ImagerConfigNotFoundError)
{
    try
    {
        throw ImagerConfigNotFoundError();

    }
    catch (const Exception &e)
    {
        EXPECT_EQ (std::string (e.what()), "The external imager configuration file was not found.");
        EXPECT_EQ (e.getStatus(), royale::CameraStatus::UNKNOWN);
    }

    try
    {
        throw ImagerConfigNotFoundError();

    }
    catch (const ImagerConfigNotFoundError &e)
    {
        EXPECT_EQ (e.getConfigFileName(), "");
    }

    try
    {
        ImagerConfigNotFoundError e;

        e.setConfigFileName ("Salome_M2453.lena");
        throw e;

    }
    catch (const ImagerConfigNotFoundError &e)
    {
        EXPECT_EQ (e.getConfigFileName(), "Salome_M2453.lena");
    }

    try
    {
        throw ImagerConfigNotFoundError ("Salome_M2453.lena");

    }
    catch (const ImagerConfigNotFoundError &e)
    {
        EXPECT_EQ (e.getConfigFileName(), "Salome_M2453.lena");
    }
}

TEST (TestExceptions, TestInvalidValue)
{
    try
    {
        throw InvalidValue();
    }
    catch (Exception &e)
    {
        EXPECT_EQ (e.getExceptionType(), "InvalidValue exception");
        EXPECT_EQ (e.getExceptionType(), InvalidValue().getExceptionType());
        EXPECT_EQ (e.getTechnicalDescription(), "invalid value");
        EXPECT_EQ (e.getStatus(), royale::CameraStatus::INVALID_VALUE);
    }
}

TEST (TestExceptions, TestException)
{
    try
    {
        throw Exception();
    }
    catch (Exception &e)
    {
        EXPECT_EQ (e.getExceptionType(), "Generic exception");
        EXPECT_EQ (e.getTechnicalDescription(), "unknown error");
        EXPECT_EQ (e.getUserDescription(), "An unknown error occurred");
        EXPECT_EQ (e.getStatus(), royale::CameraStatus::UNKNOWN);

        EXPECT_EQ (std::string (e.what()), "Generic exception: unknown error");
    }

    try
    {
        throw Exception ("known error", "user description", royale::CameraStatus::COULD_NOT_OPEN);
    }
    catch (Exception &e)
    {
        EXPECT_EQ (e.getExceptionType(), "Generic exception");
        EXPECT_EQ (e.getTechnicalDescription(), "known error");
        EXPECT_EQ (e.getUserDescription(), "user description");
        EXPECT_EQ (e.getStatus(), royale::CameraStatus::COULD_NOT_OPEN);

        EXPECT_EQ (std::string (e.what()), "Generic exception: known error");
    }
}
