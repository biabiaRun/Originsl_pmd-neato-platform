/****************************************************************************\
 * Copyright (C) 2019 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <QApplication>
#include <raw2rrf.hpp>

int main (int argc, char *argv[])
{
    QApplication app (argc, argv);
    app.setOrganizationName ("pmdtechnologies ag & Infineon Technologies AG");
    app.setApplicationName ("raw2rrf");

    auto styleKeys = QStyleFactory::keys();
    if (styleKeys.contains ("Windows"))
    {
        QApplication::setStyle (QStyleFactory::create ("Windows"));
    }

    Raw2RRF raw2rrf;
    raw2rrf.show();
    return app.exec();
}
