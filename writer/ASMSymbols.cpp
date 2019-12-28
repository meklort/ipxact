////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/ASMSymbols.cpp
///
/// @project    ipxact
///
/// @brief      ASM Writer for component symbols
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

#include <ASMWriter.hpp>
#include <Register.hpp>
#include <main.hpp>
#include <string.h>
#include <resources.h>

#include <map>
#include <iostream>
#include <sstream>
using namespace std;

ASMWriter::ASMWriter(const char* filename) : Writer(filename)
{
    mFilename = strdup(filename);
    mIndent = 0;
}

ASMWriter::~ASMWriter()
{
    if(mFilename) free(mFilename);
}

std::string ASMWriter::type(int width, bool isSigned) const
{
    string regtype;

    switch(isSigned)
    {
        case true: regtype = "S"; break;
        case false: regtype = "U"; break;
    }


    switch(width)
    {
        case  8: regtype += "Int8"; break;
        case 16: regtype += "Int16"; break;
        case 32: regtype += "Int32"; break;
        default:
            fprintf(stderr, "Error: unable to handle a register width of %d, please use 8, 16, or 32.\n", width);
            break;
    }

    return regtype;
}

std::string ASMWriter::indent(int modifier)
{
    ostringstream indent;
    mIndent += modifier;
    for(int i = 0; i < mIndent; i++)
    {
        indent << "    ";
    }
    return indent.str();
}

string ASMWriter::serialize_enum_definition(Component& component, Register& reg, RegisterBitmap& bitmap, Enumeration& thisenum)
{
    string enumname = thisenum.getName();
    string bitmapname = bitmap.getName();
    string regname = reg.getName();
    string componentname = component.getName();

    escape(enumname);
    escape(bitmapname);
    escape(regname);
    escape(componentname);

    std::transform(bitmapname.begin(),    bitmapname.end(),    bitmapname.begin(),    ::toupper);
    std::transform(regname.begin(),       regname.end(),       regname.begin(),       ::toupper);
    std::transform(componentname.begin(), componentname.end(), componentname.begin(), ::toupper);
    std::transform(enumname.begin(), enumname.end(), enumname.begin(), ::toupper);

    ostringstream decl;

    decl << ".equ        " << componentname << "_" << regname << "_" << bitmapname << "_" << enumname << ", 0x" << std::hex << thisenum.getValue() << endl;

    return decl.str();
}


string ASMWriter::serialize_bitmap_definition(Component& component, Register& reg, RegisterBitmap& bitmap, int regwidth)
{
    string bitmapname    = bitmap.getName();
    string regname       = reg.getName();
    string componentname = component.getName();

    escape(bitmapname);
    escape(regname);
    escape(componentname);


    std::transform(bitmapname.begin(),    bitmapname.end(),    bitmapname.begin(),    ::toupper);
    std::transform(regname.begin(),       regname.end(),       regname.begin(),       ::toupper);
    std::transform(componentname.begin(), componentname.end(), componentname.begin(), ::toupper);

    int mask = 0;
    for(int i = bitmap.getStop(); i <= bitmap.getStart(); i++)
    {
        mask |= 1 << i;
    }


    ostringstream decl;
    decl << ".equ        " << componentname << "_" << regname << "_" << bitmapname << "_SHIFT, " << bitmap.getStop() << endl;
    decl << ".equ        " << componentname << "_" << regname << "_" << bitmapname << "_MASK,  0x" << std::hex << mask << endl;

    if(!bitmap.get().empty())
    {
        const std::list<Enumeration*>& bits = bitmap.get();
        std::list<Enumeration*>::const_iterator bits_it;
        for(bits_it = bits.begin(); bits_it != bits.end(); bits_it++)
        {
            Enumeration* thisenum = *bits_it;
            if(thisenum)
            {
                thisenum->sort();
                decl << serialize_enum_definition(component, reg, bitmap, *thisenum);
            }
        }

         decl << endl;
    }


    return decl.str();
}

string ASMWriter::serialize_bitmap_declaration(Component& component, Register& reg, RegisterBitmap& bitmap, int regwidth)
{
    ostringstream decl;

    return decl.str();
}

string ASMWriter::serialize_register_definition(Component& component, Register& reg)
{
    string regname = reg.getName();
    string componentname = component.getName();

    std::transform(regname.begin(),       regname.end(),       regname.begin(),       ::toupper);
    std::transform(componentname.begin(), componentname.end(), componentname.begin(), ::toupper);

    ostringstream decl;
    if(!reg.getDescription().empty())
    {
        decl <<  ".equ    REG_" << componentname << "_" << regname << ", 0x" << std::hex << component.getBase() + reg.getAddr() << " ; " << reg.getDescription() << endl;        
    }
    else
    {
        decl <<  ".equ    REG_" << componentname << "_" << regname << ", 0x" << std::hex << component.getBase() + reg.getAddr() << endl;
    }


    if(!reg.get().empty())
    {
        const std::list<RegisterBitmap*>& bits = reg.get();
        std::list<RegisterBitmap*>::const_iterator bits_it;
        for(bits_it = bits.begin(); bits_it != bits.end(); bits_it++)
        {
            RegisterBitmap* bit = *bits_it;
            if(bit)
            {
                bit->sort();
                decl << serialize_bitmap_definition(component, reg, *bit, reg.getWidth());
            }
        }
    }
    decl << endl;

    return decl.str();;
}

string ASMWriter::camelcase(const string& str)
{
    ostringstream camelstr;
    size_t srcpos = 0;
    bool needscap = true;
    for(srcpos = 0; srcpos < str.length(); srcpos++)
    {
        if(str[srcpos] == '_')
        {
            // do nothing, drop char.
            needscap = true;
        }
        else
        {
            if(needscap)
            {
                camelstr << (char)toupper(str[srcpos]);
            }
            else
            {
                camelstr << (char)tolower(str[srcpos]);
            }
            needscap = false;
        }
    }
    return camelstr.str();
}

string& ASMWriter::escape(std::string& str)
{
    std::replace(str.begin(), str.end(), ' ', '_'); // No spaces allowed
    std::replace(str.begin(), str.end(), '.', '_'); // Replace .'s with _'s
    std::replace(str.begin(), str.end(), ',', '_'); // Replace ,'s with _'s
    std::replace(str.begin(), str.end(), ':', '_'); // Replace :'s with _'s
    std::replace(str.begin(), str.end(), '[', '_'); // Replace ['s with _'s
    std::replace(str.begin(), str.end(), ']', '_'); // Replace ]'s with _'s

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

string ASMWriter::serialize_register_declaration(Component& component, Register& reg)
{
    ostringstream decl;
    

    return decl.str();
}

std::string    ASMWriter::serialize_component_declaration(Component& component)
{
    ostringstream decl;

    const std::list<Register*>& regs = component.get();
    std::list<Register*>::const_iterator it;

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

void ASMWriter::strreplace(string& origstr, const string& find, const string& replace)
{
    if(replace == find) return;

    int findlen = find.length();
    while (origstr.find(find) != string::npos)
    {
        origstr.replace(origstr.find(find), findlen, replace);
    }
}

bool ASMWriter::write(Components& components)
{
    string filename(mFilename);
    ostringstream output;
    string* file_contents = new RESOURCE_STRING(resources_ASMHeader_s);

    indent(1);
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
