/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

/**
 * Some code is simplified by supporting a hard coded limit on the number of streams in each
 * use case.  This header file provides a value for such a limit.
 *
 * The class UseCaseDefinition does not enforce this limit itself, because it's useful for
 * test cases to have more.  Components that rely on this should check it in their own
 * verifyUseCase() method.
 */

#define ROYALE_USECASE_MAX_STREAMS 3
