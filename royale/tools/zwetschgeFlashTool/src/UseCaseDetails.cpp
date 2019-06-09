/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <UseCaseDetails.hpp>

UseCaseDetails::UseCaseDetails()
{
    setupUi (this);

    m_registerMapTable = new RegisterMapTable();
    gbTimed->layout()->addWidget (m_registerMapTable);
}

UseCaseDetails::~UseCaseDetails()
{
}
