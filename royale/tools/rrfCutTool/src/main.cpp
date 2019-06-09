/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <RRFReader.h>
#include <RRFWriter.h>

#include <argUtilities.hpp>

#include <cstddef>
#include <cstring>
#include <string>
#include <iostream>

namespace args
{
    bool find_arg_help (int const argc, char const **argv)
    {
        static char const *arg_input_file_upper_case = "-h";
        static char const *arg_input_file_lower_case = "-H";
        static char const *arg_input_file_explicit = "--help";

        return contains (arg_input_file_upper_case, argc, argv)
               || contains (arg_input_file_lower_case, argc, argv)
               || contains (arg_input_file_explicit, argc, argv);
    }

    bool find_arg_input_file (int const argc, char const **argv, char const *&input_file)
    {
        static char const *arg_input_file_upper_case = "-i";
        static char const *arg_input_file_lower_case = "-I";
        static char const *arg_input_file_explicit = "--input";

        // check if the first arg is a parameter
        if (1 < argc)
        {
            if (may_param (argv[1]))
            {
                input_file = argv[1];
                return true;
            }
        }

        int index = 0;

        if (index_of (arg_input_file_upper_case, argc, argv, index)
                || index_of (arg_input_file_lower_case, argc, argv, index)
                || index_of (arg_input_file_explicit, argc, argv, index))
        {
            if (index + 1 < argc)
            {
                if (may_param (argv[index + 1]))
                {
                    input_file = argv[index + 1];
                    return true;
                }
            }
        }

        return false;
    }

    bool find_arg_output_file (int const argc, char const **argv, char const *&output_file)
    {
        static char const *arg_output_file_upper_case = "-o";
        static char const *arg_output_file_lower_case = "-O";
        static char const *arg_output_file_explicit = "--output";

        int index = 0;

        if (index_of (arg_output_file_upper_case, argc, argv, index)
                || index_of (arg_output_file_lower_case, argc, argv, index)
                || index_of (arg_output_file_explicit, argc, argv, index))
        {
            if (index + 1 < argc)
            {
                if (may_param (argv[index + 1]))
                {
                    output_file = argv[index + 1];
                    return true;
                }
            }
        }

        return false;
    }

    bool find_arg_cut_range (int const argc, char const **argv, uint32_t &from, uint32_t &to)
    {
        static char const *arg_cut_range_upper_case = "-r";
        static char const *arg_cut_range_lower_case = "-R";
        static char const *arg_cut_range_explicit = "--range";

        int index = 0;

        if (index_of (arg_cut_range_upper_case, argc, argv, index)
                || index_of (arg_cut_range_lower_case, argc, argv, index)
                || index_of (arg_cut_range_explicit, argc, argv, index))
        {
            if (index + 1 < argc)
            {
                if (may_param (argv[index + 1]))
                {
                    from = static_cast<uint32_t> (std::stoul (argv[index + 1]));

                    if (index + 2 < argc)
                    {
                        if (may_param (argv[index + 2]))
                        {
                            to = static_cast<uint32_t> (std::stoul (argv[index + 2]));
                            return true;
                        }
                    }

                    to = static_cast<uint32_t> (std::stoul (argv[index + 1]));
                    return true;
                }
            }
        }

        return false;
    }

    bool find_arg_debug (int const argc, char const **argv)
    {
        static char const *arg_debug_upper_case = "-d";
        static char const *arg_debug_lower_case = "-D";
        static char const *arg_debug_explicit = "--debug";

        return contains (arg_debug_upper_case, argc, argv)
               || contains (arg_debug_lower_case, argc, argv)
               || contains (arg_debug_explicit, argc, argv);
    }

    void print_argv (int const argc, char const **argv, char const *postfix = "")
    {
        std::cout << std::endl;
        std::cout << "--- argv" << postfix << ": ---" << std::endl;

        char const *input_file = nullptr;
        if (find_arg_input_file (argc, argv, input_file))
        {
            std::cout << "input_file: " << input_file << std::endl;
        }

        char const *output_file = nullptr;
        if (find_arg_output_file (argc, argv, output_file))
        {
            std::cout << "output_file: " << output_file << std::endl;
        }

        uint32_t from = 0;
        uint32_t to = 0;
        if (find_arg_cut_range (argc, argv, from, to))
        {
            std::cout << "from: " << from << ", to: " << to << std::endl;
        }

        if (find_arg_debug (argc, argv))
        {
            std::cout << "debug: true" << std::endl;
        }

        if (find_arg_help (argc, argv))
        {
            std::cout << "help: true" << std::endl;
        }
    }

    void print_help (int const argc, char const **argv)
    {
        std::cout << std::endl;

        std::cout << "Description:                                  " << std::endl;
        std::cout << "This tool is designed to manipulate RRF-Files." << std::endl;
        std::cout << "Therefore it provides a variety of options to cut frames out of a RRF-File and store them in a new RRF-File." << std::endl;
        std::cout << std::endl;

        if (argc > 0)
        {
            std::cout << "Usage:                                                                 " << std::endl;
            std::cout << argv[0] << " -h                                                         " << std::endl;
            std::cout << argv[0] << " -i <file_input> -o <file_output> -r <index_from> <index_to>" << std::endl;
            std::cout << std::endl;

            std::cout << "Options:                                                                                                   " << std::endl;
            std::cout << "-h | -H | --help                             Print this help and stops execution immediately               " << std::endl;
            std::cout << "-i | -I | --input <file_input>               Sets the input file                - ALWAYS REQUIRED          " << std::endl;
            std::cout << "-o | -O | --output <file_output>             Sets the output file               - ALWAYS REQUIRED          " << std::endl;
            std::cout << "-r | -R | --range <index_from> <index_to>    Sets the range of frames to be cut (zero based).              " << std::endl;
            std::cout << "                                             If index_to is omitted only one frame will be exported.       " << std::endl;
            std::cout << "-d | -D | --debug                            Prints additional information while processing                " << std::endl;
            std::cout << std::endl;
        }
    }
}

int main (int const argc, char const **argv)
{
    char const *input_file = nullptr;
    char const *output_file = nullptr;

    uint32_t from = 0;
    uint32_t to = 0;
    bool const debug = args::find_arg_debug (argc, argv);

    /*************************************/
    /** Parse the command line arguments */
    /*************************************/

    if (debug)
    {
        args::print_help (argc, argv);
        args::print_argv (argc, argv);
    }

    if (args::find_arg_help (argc, argv))
    {
        args::print_help (argc, argv);
        return 0;
    }

    if (!args::find_arg_input_file (argc, argv, input_file))
    {
        std::cerr << "No source was given. Without the source, the tool has nothing to do!" << std::endl;
        std::cerr << "See --help for usage!" << std::endl;
        return -1;
    }

    if (!args::find_arg_output_file (argc, argv, output_file))
    {
        std::cerr << "No output was given. Without the output, the tool has nothing to do!" << std::endl;
        std::cerr << "See --help for usage!" << std::endl;
        return -1;
    }

    if (!args::find_arg_cut_range (argc, argv, from, to))
    {
        std::cerr << "Neither range nor frame was given. Without a range or a frame, the tool has nothing to do!" << std::endl;
        std::cerr << "See --help for usage!" << std::endl;
        return -1;
    }

    /************************/
    /** Execute the command */
    /************************/

    royale_rrf_handle input_handle;
    if (royale_open_input_file (&input_handle, input_file) != royale_rrf_api_error::RRF_NO_ERROR)
    {
        std::cerr << "Can not open the input file!" << std::endl;
        std::cerr << "See --help for usage!" << std::endl;
        return -1;
    }

    auto const info = royale_get_fileinformation (input_handle);

    auto numFrames = royale_get_num_frames (input_handle);
    if (from >= numFrames ||
            to >= numFrames)
    {
        std::cerr << "Range not valid for this RRF file!" << std::endl;
        std::cerr << "Number of frames in " << input_file << " : " << numFrames;
        return -1;
    }

    royale_rrf_handle output_handle;
    if (royale_open_output_file (&output_handle, output_file, info) != royale_rrf_api_error::RRF_NO_ERROR)
    {
        std::cerr << "Can not open the output file!" << std::endl;
        std::cerr << "See --help for usage!" << std::endl;
        return -1;
    }

    for (auto i = from; i <= to; i++)
    {
        if (royale_seek (input_handle, i) != royale_rrf_api_error::RRF_NO_ERROR)
        {
            std::cerr << "Error while seeking a frame. Maybe the range is not valid for this RRF-File!" << std::endl;
            std::cerr << "See --help for usage!" << std::endl;
            return -1;
        }

        royale_frame_v3 *frame_v3_ptr = nullptr;
        if (royale_get_frame (input_handle, &frame_v3_ptr) != royale_rrf_api_error::RRF_NO_ERROR)
        {
            std::cerr << "Error while receiving a frame. Maybe the range is not valid for this RRF-File!" << std::endl;
            std::cerr << "See --help for usage!" << std::endl;
            royale_free_frame (&frame_v3_ptr);
            return -1;
        }

        royale_output_data (output_handle, frame_v3_ptr);
        royale_free_frame (&frame_v3_ptr);
    }

    royale_close_output_file (output_handle);

    return 0;
}
