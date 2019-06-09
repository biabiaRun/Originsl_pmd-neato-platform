/****************************************************************************\
 * Copyright (C) 2016 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <QApplication>
#include <rrftool.hpp>

int main (int argc, char *argv[])
{
    QApplication app (argc, argv);
    app.setOrganizationName ("pmdtechnologies ag & Infineon Technologies AG");
    app.setApplicationName ("rrfTool");

    RRFTool rrfTool;
    rrfTool.show();
    return app.exec();
}
