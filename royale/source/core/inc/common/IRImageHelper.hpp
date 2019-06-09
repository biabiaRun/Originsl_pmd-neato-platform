/****************************************************************************\
* Copyright (C) 2016 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <sstream>
#include <iostream>
#include <fstream>

#include <royale/IRImage.hpp>

namespace royale
{
    /**
    * Save the IRImage into a PGM file with the given filename.
    * If the filename is empty it will be created from the timestamp.
    * @param image The image that should be saved.
    * @param filename The filename of the output file.
    */

    void saveIRImageToPGM (const IRImage &image, std::string filename = "")
    {
        std::string pgmName (filename);
        if (pgmName.empty())
        {
            // If the filename is empty use the current timestamp as filename
            std::stringstream ss;

            ss << image.timestamp << ".pgm";
            pgmName = ss.str();
        }

        std::ofstream myfile;
        myfile.open (pgmName, std::ios::binary);

        // Write the PGM header (P5==Binary monochrome)
        myfile << "P5" << std::endl;
        myfile << image.width << " " << image.height << std::endl;
        myfile << "255" << std::endl;

        myfile.write ( (char *) &image.data[0], image.data.size());

        myfile.close();
    }

}
