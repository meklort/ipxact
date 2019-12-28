////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/SimulatorWriter.cpp
///
/// @project    ipxact
///
/// @brief      C++ Simulation geneartor
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

#include <SimulatorWriter.hpp>
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

SimulatorWriter::SimulatorWriter(const char* filename) : Writer(filename)
{
    mFilename = strdup(filename);
    mIndent = 0;
}

SimulatorWriter::~SimulatorWriter()
{
    if(mFilename) free(mFilename);
}

std::string SimulatorWriter::getComponentFile(const char* componentname)
{
    string retstr = mFilename;
    retstr = retstr.substr(0, retstr.find_last_of("."));
    retstr += + "_" + string(componentname) + ".cpp";

    return retstr;
}

std::string SimulatorWriter::getComponentMMAPFile(const char* componentname)
{
    string retstr = mFilename;
    retstr = retstr.substr(0, retstr.find_last_of("."));
    retstr += + "_" + string(componentname) + "_sim.cpp";

    return retstr;
}

std::string SimulatorWriter::get_type_name(Component& component)
{
    string componentTypeID = component.getTypeID();
    string componentname = componentTypeID.empty() ? component.getName() : componentTypeID;
    std::transform(componentname.begin(), componentname.end(), componentname.begin(), ::toupper);
    componentname += "_t";

    return componentname;
}

std::string SimulatorWriter::get_type_name(Component& component, Register& reg)
{
    string componentTypeID = component.getTypeID();
    string componentname = componentTypeID.empty() ? component.getName() : componentTypeID;
    std::transform(componentname.begin(), componentname.end(), componentname.begin(), ::toupper);

    string registerTypeID = reg.getTypeID();

    string regname       = registerTypeID.empty() ? reg.getName() : registerTypeID;
    string reg_type = "Reg" +  componentname + camelcase(regname) + "_t";

    return reg_type;
}


std::string SimulatorWriter::type(int width, bool isSigned) const
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

std::string SimulatorWriter::indent(int modifier)
{
    ostringstream indent;
    mIndent += modifier;
    for(int i = 0; i < mIndent; i++)
    {
        indent << "    ";
    }
    return indent.str();
}

string SimulatorWriter::serialize_bitmap_declaration(Component& component, Register& reg, RegisterBitmap& bitmap, int regwidth)
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


string SimulatorWriter::serialize_register_definition(Component& component, Register& reg)
{
    string regname = reg.getName();
    string componentType = get_type_name(component);

    std::transform(regname.begin(),       regname.end(),       regname.begin(),       ::toupper);

    ostringstream decl;

    decl << indent() << "/** @brief Bitmap for @ref " << componentType << "." << camelcase(regname) << ". */" << endl;

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

string SimulatorWriter::serialize_register_mmap_definition(Component& component, Register& reg, Register* prevreg)
{
    string regname = reg.getName();
    string componentType = get_type_name(component);

    std::transform(regname.begin(), regname.end(), regname.begin(), ::toupper);

    ostringstream decl;


    int padding = 0;
    int expStart = 0;
    if(prevreg)
    {
        int width = prevreg->getWidth() / component.getAddressUnitBits(); // in bytes.
        expStart  = prevreg->getAddr() + (width * prevreg->getDimensions());
        padding   = reg.getAddr() - expStart;
    }
    else
    {
        expStart = 0;
        padding  = reg.getAddr();
    }

    if(padding)
    {
        if(padding > 0)
        {
            if(prevreg)
            {
                fprintf(stdout, "Info: adding %d bytes of padding between register %s and %s.\n", padding, prevreg->getName().c_str(), reg.getName().c_str());
            }
            else
            {
                fprintf(stdout, "Info: adding %d bytes of padding before first register %s.\n", padding, reg.getName().c_str());
            }
            if(0 == padding % 4)
            {
                padding /= 4;
            }
            else if(0 == padding % 2)
            {
                padding /= 2;
            }

            string basename = string(component.getName()) + string(".") + "reserved_";
            decl << indent() << "for(int i = 0; i < " << std::dec << padding << "; i++)" << endl;
            decl << indent() << "{" << endl;
            indent(1);
            decl << indent() << basename << std::dec << expStart << "[i].installReadCallback(read_from_ram, (uint8_t *)base);" << endl;
            decl << indent() << basename << std::dec << expStart << "[i].installWriteCallback(write_to_ram, (uint8_t *)base);" << endl;
            decl << indent(-1) << "}" << endl;
        }
        else
        {
            if(prevreg)
            {
                fprintf(stderr, "Error: requested %d bytes of padding between component type '%s' registers '%s' and '%s'.\n",
                    padding, componentType.c_str(), prevreg->getName().c_str(), reg.getName().c_str());
            }
            else
            {
                fprintf(stderr, "Error: requested %d bytes of padding before component type %s's first register '%s'.\n",
                    padding, componentType.c_str(), reg.getName().c_str());
            }
            while(1);
        }
    }


    decl << indent() << "/** @brief Bitmap for @ref " << componentType << "." << camelcase(regname) << ". */" << endl;

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
        decl << indent() << basename << ".installReadCallback(read_from_ram, (uint8_t *)base);" << endl;
        decl << indent() << basename << ".installWriteCallback(write_to_ram, (uint8_t *)base);" << endl;
        indent(-1);
        decl << indent() << "}" << endl;
    }
    else
    {
        string basename = string(component.getName()) + string(".") + newname + string(".r") + to_string(width);
        decl << indent() << basename << ".installReadCallback(read_from_ram, (uint8_t *)base);" << endl;
        decl << indent() << basename << ".installWriteCallback(write_to_ram, (uint8_t *)base);" << endl;
    }

    decl << endl;


    return decl.str();;
}

string& SimulatorWriter::escapeEnum(std::string& str)
{
    while(str.find(" ") != std::string::npos)
    {
        str.replace(str.find(" "), 1, ""); // Replace @'s with _AT_'s.
    }

    return escape(str);
}


string& SimulatorWriter::escape(std::string& str)
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

string SimulatorWriter::camelcase(const string& str)
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

std::string    SimulatorWriter::serialize_component_declaration(Component& component)
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

std::string    SimulatorWriter::serialize_mmap_declaration(Component& component)
{
    const string& componentname = component.getName();
    ostringstream decl;

    const std::list<Register*>& regs = component.get();
    std::list<Register*>::const_iterator it;

    decl << indent() << "/** @brief Component Registers for @ref " << componentname << ". */" << endl;
    Register* prevreg = NULL;
    for(it = regs.begin(); it != regs.end(); it++)
    {
        Register* reg = *it;
        if(reg)
        {
            reg->sort();
            decl << serialize_register_mmap_definition(component, *reg, prevreg);
            prevreg = reg;
        }
    }

    return decl.str();
}

void SimulatorWriter::strreplace(string& origstr, const string& find, const string& replace)
{
    if(replace == find) return;

    int findlen = find.length();
    while (origstr.find(find) != string::npos)
    {
        origstr.replace(origstr.find(find), findlen, replace);
    }
}

bool SimulatorWriter::write(Components& components)
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


bool SimulatorWriter::writeComponent(Component &component)
{
    const string& componentname = component.getName();

    string filename = getComponentFile(componentname.c_str());
    string mmap_filename = getComponentMMAPFile(componentname.c_str());


    string* file_contents = new RESOURCE_STRING(resources_SimulatorOutput_cpp);
    string* mmap_contents = new RESOURCE_STRING(resources_SimulatorOutput_mmap_cpp);


    indent(1);
    component.sort();

    UpdateTemplate(*file_contents, filename, component);
    strreplace(*file_contents, "<SERIALIZED>", serialize_component_declaration(component));

    UpdateTemplate(*mmap_contents, mmap_filename, component);
    strreplace(*mmap_contents, "<SERIALIZED>", serialize_mmap_declaration(component));


    indent(-1);
    return WriteToFile(filename, *file_contents) && WriteToFile(mmap_filename, *mmap_contents);
}
