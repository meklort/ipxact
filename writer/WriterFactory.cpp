////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/WriterFactory.cpp
///
/// @project    ipxact
///
/// @brief      Factory for file writers
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
#include <string.h>
#include <stdlib.h>
#include <sstream>

#include <main.hpp>


#include <Writer.hpp>

#include <ASMWriter.hpp>
#include <ASMSymbols.hpp>
#include <HeaderWriter.hpp>
#include <SimulatorWriter.hpp>
#include <APESimulatorWriter.hpp>
#include <IPXACTWriter.hpp>
#include <LaTeXWriter.hpp>

using namespace std;

Writer* WriterFactory::create(const char* filename, const char* force_extension)
{
    Writer* myWriter = NULL;
    char* file = strdup(filename);
    char* saveptr;
    char* partial = strtok_r(file, ".", &saveptr);

    // find file extension and return appropriate output writer.
    do
    {
        char* next;
        if(force_extension)
        {
            next = NULL;
            partial = strdup(force_extension);
        }
        else
        {
            next = strtok_r(NULL, ".", &saveptr);
        }

        if(!next)
        {
            printf("Checking extension '%s'\n", partial);

            if(0 == strncmp("h", partial, sizeof("h")))
            {
                // header.
                myWriter = new HeaderWriter(filename);
            }
            else if(0 == strncmp("xml", partial, sizeof("xml")))
            {
                // ipxact.
                myWriter = new IPXACTWriter(filename);
            }
            else if(0 == strncmp("tex", partial, sizeof("tex")))
            {
                // ipxact.
                myWriter = new LaTeXWriter(filename);
            }
            else if(0 == strncmp("asym", partial, sizeof("asym")))
            {
                // ipxact.
                myWriter = new ASMSymbols(filename);
            }
            else if(0 == strncmp("s", partial, sizeof("s")))
            {
                // ipxact.
                myWriter = new ASMWriter(filename);
            }
            else if(0 == strncmp("cpp", partial, sizeof("cpp")))
            {
                // Simulation / model.
                myWriter = new SimulatorWriter(filename);
            }
            else if(0 == strncmp("ape_cpp", partial, sizeof("ape_cpp")))
            {
                // Simulation / Model.
                myWriter = new APESimulatorWriter(filename);
            }
        }
        partial = next;
    } while(partial);

    free(file);
    return myWriter;
}


Writer::Writer(const char* filename)
{
    // Open output file
    mFile.open(filename, ios::out | ios::binary);
}

Writer::~Writer()
{
    mFile.close();

}

static void strreplace(string& origstr, const string& find, const string& replace)
{
    if(replace == find) return;

    int findlen = find.length();
    while (origstr.find(find) != string::npos)
    {
        origstr.replace(origstr.find(find), findlen, replace);
    }
}

void Writer::UpdateTemplate(std::string& contents, std::string& filename, Component &component)
{
    const string& componentname = component.getName();
    int componentSize = 0;

    string componentTypeID = component.getTypeID();
    string type = componentTypeID.empty() ? component.getName() : componentTypeID;
    std::transform(type.begin(), type.end(), type.begin(), ::toupper);

    if(component.getRange())
    {
        componentSize = component.getRange();
    }
    else
    {
        if(!component.get().empty())
        {
            Register* lastreg = component.get().back();
            if(lastreg)
            {
                int endwidth = lastreg->getWidth();
                int endaddr = lastreg->getAddr();
                int dims = lastreg->getDimensions();
                int addessablesize = component.getAddressUnitBits();

                componentSize = (dims * (endwidth/8)) + endaddr;
                componentSize *= addessablesize/8;
            }
        }
    }


    strreplace(contents, "<COMPONENT>", componentname.c_str());
    strreplace(contents, "<COMPONENT_TYPE>", type.c_str());
    strreplace(contents, "<COMPONENT_SIZE>", std::to_string(componentSize));

    UpdateTemplate(contents, filename);
}

void Writer::UpdateTemplate(std::string& contents, std::string& filename)
{
    string guard(filename);
    std::transform(guard.begin(), guard.end(), guard.begin(), ::toupper);
    std::replace(guard.begin(), guard.end(), '.', '_');
    std::replace(guard.begin(), guard.end(), '/', '_');

    strreplace(contents, "<FILE>", filename);
    strreplace(contents, "<PROJECT>", (*gOptions)["project"].c_str());

    string filename_strip = filename;

    // FIXME
    strreplace(filename_strip, ".cpp", "");
    strreplace(filename_strip, ".h", "");

    strreplace(contents, "<INIT_FUNCTION>", filename_strip);
    strreplace(contents, "<GUARD>", guard);

    ostringstream descstream;
    descstream << filename_strip;
    string description = descstream.str();

    strreplace(contents, "<DESCRIPTION>", description);

    string includes = "#include <" + filename_strip + ".h>";
    strreplace(includes, "_sim", ""); // hack
    strreplace(contents, "<INCLUDES>", includes);
}

bool Writer::WriteToFile(string& filename, string& contents)
{
    std::ofstream outFile;
    outFile.open(filename, ios::out | ios::binary);

    outFile << contents;

    return outFile.good();
}

