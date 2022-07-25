////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/APESimulatorWriter.cpp
///
/// @project    ipxact
///
/// @brief      Simulation Writer code for indirect access to the APE.
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

#include <APESimulatorWriter.hpp>
#include <Register.hpp>
#include <main.hpp>
#include <string.h>
#include <resources.h>

#include <map>
#include <iostream>
#include <sstream>
using namespace std;

// <FILE>           filename.h
// <PROJECT>        project
// <DESCRIPTION>    filename registers
// <GUARD>          PATH_TO_FILENAME_H

APESimulatorWriter::APESimulatorWriter(const char* filename) : Writer(filename)
{
    mFilename = strdup(filename);
    mIndent = 0;
}

APESimulatorWriter::~APESimulatorWriter()
{
    if(mFilename) free(mFilename);
}

std::string APESimulatorWriter::getComponentFile(const char* componentname)
{
    string retstr = mFilename;
    retstr = retstr.substr(0, retstr.find_last_of("."));
    retstr += + "_" + string(componentname) + ".cpp";

    return retstr;
}

std::string APESimulatorWriter::getComponentAPEFile(const char* componentname)
{
    string retstr = mFilename;
    retstr = retstr.substr(0, retstr.find_last_of("."));
    retstr += + "_" + string(componentname) + "_sim.cpp";

    return retstr;
}


std::string APESimulatorWriter::type(int width, bool isSigned) const
{
    string regtype;

    switch(isSigned)
    {
        case true: regtype = ""; break;
        case false: regtype = "u"; break;
    }


    switch(width)
    {
        case  8: regtype += "int8_t"; break;
        case 16: regtype += "int16_t"; break;
        case 32: regtype += "int32_t"; break;
        default:
            fprintf(stderr, "Error: unable to handle a register width of %d, please use 8, 16, or 32.\n", width);
            break;
    }

    return regtype;
}

std::string APESimulatorWriter::indent(int modifier)
{
    ostringstream indent;
    mIndent += modifier;
    for(int i = 0; i < mIndent; i++)
    {
        indent << "    ";
    }
    return indent.str();
}

string APESimulatorWriter::serialize_bitmap_declaration(Component& component, Register& reg, RegisterBitmap& bitmap, int regwidth)
{
    ostringstream decl;

    string bitmapname(bitmap.getName());
    string regname(reg.getName());
    escapeEnum(bitmapname);
    if(isdigit(bitmapname[0]))
    {
        // Ensure variables start with a valid identifier.
        bitmapname = string("_") + bitmapname;
    }

    string newname = regname;
    newname = camelcase(escape(newname));
    if(isdigit(regname[0]))
    {
        newname = string("_") + newname;
        cout << "Invalid: " << newname << endl;
        // exit(-1);
    }

    int width = reg.getWidth();

    string bitvar   = string(component.getName()) + string(".") + newname + string(".bits.") + bitmapname;
    string basename = string(component.getName()) + string(".") + newname + string(".r") + to_string(width);

    // decl << indent() << bitvar << ".setBaseRegister(&" + basename + ");" << endl;

    return decl.str();
}


string APESimulatorWriter::serialize_register_definition(Component& component, Register& reg)
{
    string regname = reg.getName();
    string componentname = component.getName();

    std::transform(regname.begin(),       regname.end(),       regname.begin(),       ::toupper);
    std::transform(componentname.begin(), componentname.end(), componentname.begin(), ::toupper);

    ostringstream decl;

    decl << indent() << "/** @brief Bitmap for @ref " << componentname << "_t." << camelcase(regname) << ". */" << endl;

    if(!reg.get().empty())
    {
        const std::list<RegisterBitmap*>& bits = reg.get();
        std::list<RegisterBitmap*>::const_iterator bits_it;
        int width = reg.getWidth();
        for(bits_it = bits.begin(); bits_it != bits.end(); bits_it++)
        {
            RegisterBitmap* bit = *bits_it;
            if(bit)
            {
                decl << serialize_bitmap_declaration(component, reg, *bit, width);
            }
        }
     }
    decl << endl;


    return decl.str();;
}

string APESimulatorWriter::serialize_register_ape_definition(Component& component, Register& reg)
{
    string regname = reg.getName();
    string componentname = component.getName();

    std::transform(regname.begin(),       regname.end(),       regname.begin(),       ::toupper);
    std::transform(componentname.begin(), componentname.end(), componentname.begin(), ::toupper);

    ostringstream decl;

    decl << indent() << "/** @brief Bitmap for @ref " << componentname << "_t." << camelcase(regname) << ". */" << endl;

    int width = reg.getWidth();


    string newname = regname;
    newname = camelcase(escape(newname));
    if(isdigit(regname[0]))
    {
        newname = string("_") + newname;
        cout << "Invalid: " << newname << endl;
        // exit(-1);
    }


    int dim = reg.getDimensions();
    if(dim > 1)
    {
        string basename = string(component.getName()) + string(".") + newname + string("[i].r") + to_string(width);
        decl << indent() << "for(int i = 0; i < " << dim << "; i++)" << endl;
        decl << indent() << "{" << endl;
        indent(1);
        decl << indent() << basename << ".installReadCallback(read, (uint8_t *)base);" << endl;
        decl << indent() << basename << ".installWriteCallback(write, (uint8_t *)base);" << endl;
        indent(-1);
        decl << indent() << "}" << endl;
    }
    else
    {
        string basename = string(component.getName()) + string(".") + newname + string(".r") + to_string(width);
        decl << indent() << basename << ".installReadCallback(read, (uint8_t *)base);" << endl;
        decl << indent() << basename << ".installWriteCallback(write, (uint8_t *)base);" << endl;
    }

    decl << endl;


    return decl.str();;
}

string& APESimulatorWriter::escapeEnum(std::string& str)
{
    while(str.find(" ") != std::string::npos)
    {
        str.replace(str.find(" "), 1, ""); // Replace @'s with _AT_'s.
    }

    return escape(str);
}


string& APESimulatorWriter::escape(std::string& str)
{
    std::replace(str.begin(), str.end(), ' ', '_'); // No spaces allowed
    std::replace(str.begin(), str.end(), '-', '_'); // Replace -'s with _'s
    std::replace(str.begin(), str.end(), '.', '_'); // Replace .'s with _'s
    std::replace(str.begin(), str.end(), ',', '_'); // Replace ,'s with _'s
    std::replace(str.begin(), str.end(), ':', '_'); // Replace :'s with _'s
    std::replace(str.begin(), str.end(), '[', '_'); // Replace ['s with _'s
    std::replace(str.begin(), str.end(), ']', '_'); // Replace ]'s with _'s

    while(str.find("—") != std::string::npos)
    {
        str.replace(str.find("—"), strlen("—"), "_"); // Replace @'s with _AT_'s.
    }

    while(str.find("@") != std::string::npos)
    {
        str.replace(str.find("@"), 1, "_AT_"); // Replace @'s with _AT_'s.
    }

    while(str.find("/") != std::string::npos)
    {
        str.replace(str.find("/"), 1, "_DIV_"); // Replace /'s with _DIV_'s.
    }

    return str;
}

string APESimulatorWriter::camelcase(const string& str)
{
    string newstring = str;
    std::replace(newstring.begin(), newstring.end(), ' ', '_'); // No spaces allowed
    std::replace(newstring.begin(), newstring.end(), '.', '_'); // Replace .'s with _'s
    std::replace(newstring.begin(), newstring.end(), ',', '_'); // Replace ,'s with _'s
    std::replace(newstring.begin(), newstring.end(), ':', '_'); // Replace :'s with _'s
    std::replace(newstring.begin(), newstring.end(), '[', '_'); // Replace ['s with _'s
    std::replace(newstring.begin(), newstring.end(), ']', '_'); // Replace ]'s with _'s
    std::replace(newstring.begin(), newstring.end(), '-', '_'); // Replace -'s with _'s

    while(newstring.find("—") != std::string::npos)
    {
        newstring.replace(newstring.find("—"), strlen("—"), "_"); // Replace -'s with _'s.
    }

    ostringstream camelstr;
    size_t srcpos = 0;
    bool needscap = true;
    for(srcpos = 0; srcpos < newstring.length(); srcpos++)
    {
        if(newstring[srcpos] == '_')
        {
            // do nothing, drop char.
            needscap = true;
        }
        else
        {
            if(needscap)
            {
                camelstr << (char)toupper(newstring[srcpos]);
            }
            else
            {
                camelstr << (char)tolower(newstring[srcpos]);
            }
            needscap = false;
        }
    }
    return camelstr.str();
}

std::string    APESimulatorWriter::serialize_component_declaration(Component& component)
{
    const string& componentname = component.getName();
    ostringstream decl;

    const std::list<Register*>& regs = component.get();
    std::list<Register*>::const_iterator it;

    decl << indent() << "/** @brief Component Registers for @ref " << componentname << ". */" << endl;
    for(it = regs.begin(); it != regs.end(); it++)
    {
        Register* reg = *it;
        if(reg)
        {
            reg->sort();
            decl << serialize_register_definition(component, *reg);
        }
    }

    return decl.str();
}

std::string    APESimulatorWriter::serialize_ape_declaration(Component& component)
{
    const string& componentname = component.getName();
    ostringstream decl;

    const std::list<Register*>& regs = component.get();
    std::list<Register*>::const_iterator it;

    decl << indent() << "/** @brief Component Registers for @ref " << componentname << ". */" << endl;
    for(it = regs.begin(); it != regs.end(); it++)
    {
        Register* reg = *it;
        if(reg)
        {
            reg->sort();
            decl << serialize_register_ape_definition(component, *reg);
        }
    }

    return decl.str();
}

void APESimulatorWriter::strreplace(string& origstr, const string& find, const string& replace)
{
    if(replace == find) return;

    int findlen = find.length();
    while (origstr.find(find) != string::npos)
    {
        origstr.replace(origstr.find(find), findlen, replace);
    }
}

bool APESimulatorWriter::write(Components& components)
{
    bool status = true;

    const std::list<Component*> &componentList = components.get();
    std::list<Component*>::const_iterator it;
    for(it = componentList.begin(); it != componentList.end(); it++)
    {
        Component* component = *it;
        if(component)
        {
            status = status && writeComponent(*component);
        }
    }
    return status;
}


bool APESimulatorWriter::writeComponent(Component &component)
{
    const string& componentname = component.getName();

    string filename = getComponentFile(componentname.c_str());
    string ape_filename = getComponentAPEFile(componentname.c_str());


    string* file_contents = new RESOURCE_STRING(resources_SimulatorOutput_cpp);
    string* ape_contents = new RESOURCE_STRING(resources_SimulatorOutput_ape_cpp);
    ostringstream base_addr;
    base_addr << "0x" << std::hex << (component.getBase());

    indent(1);
    component.sort();

    UpdateTemplate(*file_contents, filename, component);
    strreplace(*file_contents, "<SERIALIZED>", serialize_component_declaration(component));

    UpdateTemplate(*ape_contents, ape_filename, component);
    strreplace(*ape_contents, "<SERIALIZED>", serialize_ape_declaration(component));

    strreplace(*ape_contents, "<BASE_ADDR>", base_addr.str());


    indent(-1);
    return WriteToFile(filename, *file_contents) && WriteToFile(ape_filename, *ape_contents);
}
