/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
* Unauthorized copying of this file, via any medium is strictly prohibited
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/exceptions/APIExceptionHandling.hpp>
#include <common/exceptions/Exception.hpp>
using namespace royale;
using namespace royale::common;

CameraStatus APIExceptionHandler::handleCurrentAPILevelException ()
{
    try
    {
        // throw; calls terminate() if we do not currently handle an exception. So let's check if this method got
        // called while stack unwinding from an exception.
        // std::current_exception() returns an exception_ptr which is zero when we are not currently within a catch block.
        // it is only set if we are actively handling an exception at the moment
        if ( (bool) std::current_exception())
        {
            // rethrow and check which type it was
            throw;
        }
    }
    catch (royale::common::Exception &royaleException)
    {
        // if a corresponding CameraStatus exists, return it.
        return royaleException.getStatus();
    }
    catch (std::exception &e)
    {
        // every other type of exception gets logged. and RUNTIME_ERROR is returned.
        LOG (ERROR) << e.what();
        return CameraStatus::RUNTIME_ERROR;
    }
    catch (...)
    {
        // if a non exception type was catched, log it and rethrow it.
        LOG (ERROR) << "Catched unknown type on API level.";
        throw;
    }
    return CameraStatus::SUCCESS;
}
