////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/ASMWriter.cpp
///
/// @project    ipxact
///
/// @brief      Assembly header writer.
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

#include <ASMSymbols.hpp>
#include <Register.hpp>
#include <main.hpp>
#include <string.h>
#include <resources.h>

#include <map>
#include <iostream>
#include <sstream>
using namespace std;

ASMSymbols::ASMSymbols(const char* filename) : Writer(filename)
{
    mFilename = strdup(filename);
}

ASMSymbols::~ASMSymbols()
{
    if(mFilename) free(mFilename);
}

std::string ASMSymbols::serialize_component_declaration(Component& component)
{
    ostringstream decl;

    string componentname = component.getName();
    std::transform(componentname.begin(), componentname.end(), componentname.begin(), ::toupper);

	int size = 0;
    if(component.getRange())
    {
        size = component.getRange();
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

                size = (dims * (endwidth/8)) + endaddr;
            }
        }
    }

    decl << ".global " << componentname << endl;
    decl << ".equ    " << componentname << ", 0x" << std::hex << component.getBase() << endl;
    decl << ".size   " << componentname << ", 0x" << std::hex << size << endl;

    return decl.str();
}

void ASMSymbols::strreplace(string& origstr, const string& find, const string& replace)
{
    if(replace == find) return;

    int findlen = find.length();
    while (origstr.find(find) != string::npos)
    {
        origstr.replace(origstr.find(find), findlen, replace);
    }
}

bool ASMSymbols::write(Components& components)
{
    string filename(mFilename);
    ostringstream output;
    string* file_contents = new RESOURCE_STRING(resources_ASMSymbols_s);

    const std::list<Component*> &componentList = components.get();
    std::list<Component*>::const_iterator it;
    for(it = componentList.begin(); it != componentList.end(); it++)
    {
        Component* component = *it;
        if(component)
        {
            component->sort();
            output << serialize_component_declaration(*component);
            output << endl;
        }
    }

    UpdateTemplate(*file_contents, filename);
    strreplace(*file_contents, "<SERIALIZED>", output.str());

    return WriteToFile(filename, *file_contents);
}
