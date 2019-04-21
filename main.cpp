////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/main.cpp
///
/// @project    ipxact
///
/// @brief      Main routine
///
////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
///
/// @copyright Copyright (c) 2019, Evan Lojewski
/// @cond
///
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
/// 1. Redistributions of source code must retain the above copyright notice,
/// this list of conditions and the following disclaimer.
/// 2. Redistributions in binary form must reproduce the above copyright notice,
/// this list of conditions and the following disclaimer in the documentation
/// and/or other materials provided with the distribution.
/// 3. Neither the name of the <organization> nor the
/// names of its contributors may be used to endorse or promote products
/// derived from this software without specific prior written permission.
///
////////////////////////////////////////////////////////////////////////////////
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
/// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
/// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
/// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
/// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
/// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
/// POSSIBILITY OF SUCH DAMAGE.
/// @endcond
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include <iostream>
#include <fstream>
#include <map>
#include <string>

#include <OptionParser.h>
#include <Register.hpp>
#include <Writer.hpp>
#include <Reader.hpp>

using namespace std;
using namespace optparse;

Components gComponents;
Values* gOptions;

int main(int argc, char *argv[])
{
    OptionParser parser = OptionParser()
        // .usage(usage)
        // .version(version)
        // .description(desc)
        // .epilog(epilog);
    ;

    parser.set_defaults("merge-addr", "0");
    parser.set_defaults("project", "<PROJECT>");

    parser.add_option("-a", "--merge-addr").action("store_true").dest("merge-addr").help("Merge register by addresses for duplicate components");
    parser.add_option("-n", "--merge-name").action("store_false").dest("merge-name").help("Merge register by names for duplicate components");
    parser.add_option("-p", "--project").dest("project").help("Sets the project name to replace <PROJECT> with");
    parser.add_option("-t", "--type").dest("type") .help("Overrides the output file type");

    Values& options = parser.parse_args(argc, argv);
    gOptions = &options;
    vector<string> args = parser.args();

    if(args.size() < 2)
    {
        parser.print_help();
        exit(-1);
    }

    const char* outname = args.back().c_str();
    const char* force_ext = options.is_set("type") ? options["type"].c_str() : NULL;


    vector<string>::const_iterator it = args.begin();
    for (; it+1 != args.end(); ++it) {
        const char* filename = it->c_str();
        fprintf(stdout, "Reading file: %s\n", filename);

        Reader* myReader = ReaderFactory::open(filename, gComponents);
        if(myReader && myReader->is_open())
        {
            if(!myReader->read())
            {
                fprintf(stderr, "Reader failed to read file: %s\n", filename);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            fprintf(stderr, "Unable to open input file '%s' for reading\n", filename);
            exit(EXIT_FAILURE);
        }
        delete myReader;
    }

    // Attempt to open output file writer.
    fprintf(stdout, "OPening output file: %s\n", outname);
    Writer* myWriter = WriterFactory::create(outname, force_ext);

    if(!myWriter)
    {
        fprintf(stderr, "Unable to create file writer for '%s'.\n", outname);
        exit(EXIT_FAILURE);
    }
    else
    {
        if(myWriter->is_open())
        {
            // output file properly opened for writing.

        }
        else
        {
            fprintf(stderr, "Unable to open output file '%s' for writing\n", outname);
            delete myWriter;
            exit(EXIT_FAILURE);
        }
    }

    fprintf(stdout, "Writing output file: %s\n", outname);

    if(myWriter)
    {
        bool result = myWriter->write(gComponents);

        delete myWriter;

        if(!result)
        {
            fprintf(stderr, "Failed to write: %s\n", outname);
            exit(EXIT_FAILURE);
        }
    }

    // for(std::map<std::string, Component*>::const_iterator it =
    //     gComponents.begin(); it != gComponents.end(); ++it)
    // {
    //     // delete it->second;
    // }

    return EXIT_SUCCESS;
}
