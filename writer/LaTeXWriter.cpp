////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/LaTeXWriter.cpp
///
/// @project    ipxact
///
/// @brief      LaTex document writer
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

#include <LaTeXWriter.hpp>
#include <Register.hpp>
#include <main.hpp>
#include <string.h>

#include <map>
#include <iostream>
#include <sstream>
using namespace std;

// <FILE>
// <PROJECT>
// <DESCRIPTION>
// <GUARD>
const char header_prefix[] = "";

const char header_suffix[] = "";


LaTeXWriter::LaTeXWriter(const char* filename) : Writer(filename)
{
    mFilename = strdup(filename);
    mIndent = 0;
}

LaTeXWriter::~LaTeXWriter()
{
    if(mFilename) free(mFilename);
}

std::string LaTeXWriter::accessType(RegisterBitmap::Type type) const
{
    string regtype;

    switch(type)
    {
        case RegisterBitmap::ReadOnly: return "RO";
        case RegisterBitmap::WriteOnly: return "WO";
        case RegisterBitmap::ReadWrite: return "RW";
        case RegisterBitmap::ReadWriteOnce: return "RW1";
        case RegisterBitmap::WriteOnce: return "W1";
        case RegisterBitmap::Reserved: return "";
    }
    return "";
}

std::string LaTeXWriter::escape(const std::string instring) const
{
    std::string outstring = instring;


    unsigned int i = 0;
    while(i < outstring.length())
    {
        if(outstring[i] == '_')
        {
            outstring.replace(i, 1, "\\_");
            i++;
        }
        else if(outstring[i] == '$')
        {
            outstring.replace(i, 1, "\\$");
            i++; 
        }
        i++;
    }


    return outstring;
}

std::string LaTeXWriter::indent(int modifier)
{
    ostringstream indent;
    mIndent += modifier;
    for(int i = 0; i < mIndent; i++)
    {
        indent << "    ";
    }
    return indent.str();
}


static bool enums_mutually_exclusive(RegisterBitmap& bitmap)
{
    unsigned int usedbits = 0;
    const std::list<Enumeration*>& bits = bitmap.get();
    std::list<Enumeration*>::const_iterator bits_it;
    for(bits_it = bits.begin(); bits_it != bits.end(); bits_it++)
    {
        Enumeration* thisenum = *bits_it;
        if(thisenum)
        {
            unsigned int value = thisenum->getValue();

            if(usedbits & value || value == 0)
            {
                return false;
            }
            else
            {
                usedbits |= value;
            }

            while(value)
            {
                if(value & 1 && value & ~1) return false; // multiple bits set.
                value >>= 1;
            }

        }
    }
    return true;
}

string LaTeXWriter::serialize_enum_definition(Component& component, Register& reg, RegisterBitmap& bitmap, Enumeration& thisenum)
{
    string enumname = thisenum.getName();
    unsigned int value = thisenum.getValue();

    //std::transform(enumname.begin(), enumname.end(), enumname.begin(), ::toupper);

    ostringstream decl;

    if(enums_mutually_exclusive(bitmap))
    {
        // TODO: handle values of 0.
        if(value)
        {
            int i = 0;
            // determine bit position.
            while((value & 1) == 0)
            {
                i++;
                value >>= 1;
            }

            decl << indent() << "[" << std::dec << i << "] " << enumname;
        }
    }
    else
    {
        decl << indent() << "0x" << std::hex << value << ": " << enumname;
    }

    return decl.str();
}

string LaTeXWriter::serialize_bitmap_definition(Component& component, Register& reg, RegisterBitmap& bitmap, int regwidth)
{
    string bitmapname = bitmap.getName();
    RegisterBitmap::Type access = bitmap.getType();
    //const string& regname = reg.getName();
    //const string& componentname = component.getName();

    //std::transform(bitmapname.begin(), bitmapname.end(), bitmapname.begin(), ::toupper);


    if(bitmapname.substr(0, strlen("reserved")) == "reserved")
    {
        access = RegisterBitmap::Reserved;
        bitmapname = "reserved";
    }

    ostringstream decl;

    //     [31:28] & name & access & reset & desc \\ \hline
    if(bitmap.getStart() == bitmap.getStop())
    {
        decl << indent() << "[" << std::dec << bitmap.getStart() << "] & ";
    }
    else
    {
        decl << indent() << "[" << std::dec << bitmap.getStart() << ":" << bitmap.getStop() << "] & ";

    }
    decl << escape(bitmapname) << " & ";                        // name
    decl << accessType(access) << " & ";     // access type
    decl << "" << " & ";                                // reset val
    decl << escape(bitmap.getDescription());

    if(!bitmap.get().empty())
    {
        if(escape(bitmap.getDescription()).size())
        {
            decl << " \\newline" << endl;
        }
        else
        {
            decl << endl;
        }

        const std::list<Enumeration*>& bits = bitmap.get();
        std::list<Enumeration*>::const_iterator bits_it;
        for(bits_it = bits.begin(); bits_it != bits.end(); bits_it++)
        {
            Enumeration* thisenum = *bits_it;
            if(thisenum)
            {
                std::list<Enumeration*>::const_iterator next_it = bits_it;
                next_it++;
                // FIXME: determine if all enums occupy one bit only. if so, chagne to a bitmap type enum (output as [V] instead of 0xV:)
                thisenum->sort();
                string out = serialize_enum_definition(component, reg, bitmap, *thisenum);
                if(out.size())
                {
                    decl << out;
                    if(next_it != bits.end()) decl << " \\newline" << endl;
                    else decl << endl << indent();
                }
            }
        }
    }
    else
    {

    }

    decl << " \\\\ \\hline" << endl;
    return decl.str();
}

string LaTeXWriter::serialize_bitmap_declaration(Component& component, Register& reg, RegisterBitmap& bitmap, int regwidth)
{
    ostringstream decl;

    return decl.str();
}

string LaTeXWriter::serialize_register_definition(Component& component, Register& reg)
{
    const string& regname = reg.getName();
    const string& componentname = component.getName();
    ostringstream decl;

    // decl << indent() << "\\phantomsection" << endl;
    // decl << indent() << "\\addcontentsline{toc}{subsection}{" << escape(regname) << "}" << endl;

    decl << indent() << "\\subsection{" + escape(regname) + "}" << endl;
    decl << indent() << escape(reg.getDescription()) << endl;
    decl << indent() << "\\begin{center}" << endl;
    decl << indent(1) << "\\rowcolors{1}{blue}{liteblue}" << endl;
    decl << indent()  << "\\begin{longtabu} to \\textwidth{ | X[2,r] | X[8,l] | X[2,l] | X[2,l] | X[16,l] |}" << endl;
    decl << indent(1) << "\\showrowcolors" << endl;
    decl << indent() << "\\hline" << endl;
    decl << indent() << "\\multicolumn{5}{|l|}{\\color{white} Register at 0x" << std::hex << component.getBase() + reg.getAddr() << ": " << componentname << "\\_" << escape(regname) << "} \\\\" << endl;
    decl << indent() << "\\hline" << endl;
    decl << indent() << "\\multicolumn{1}{|l|}{Bits} & Name & Access & Reset & Description \\\\ \\hline" << endl;
    decl << indent() << "\\hiderowcolors" << endl;
    decl << indent() << "\\endhead % all the lines above this will be repeated on every page" << endl;


    #if 0
\begin{center}
    \rowcolors{1}{blue}{liteblue}
    \begin{tabular}{ | r | l | l | l | p{10cm} |}
    \hline
    \multicolumn{5}{|l|}{\color{white} Register: CHP\_Trim0} \\
    \hline
    \multicolumn{1}{|l|}{Bits} & Name & Access & Reset & Description \\ \hline
    \hiderowcolors
    [31:28] & reserved &  &  &  \\ \hline
    [7:4] & por\_trim\_hi & RW & 0x0 & POR trim value for 0.81v threshold at 0.9V setting. \\ \hline
    \end{tabular}
\end{center}
#endif
    if(!reg.get().empty())
    {
        const std::list<RegisterBitmap*>& bits = reg.get();
        std::list<RegisterBitmap*>::const_reverse_iterator bits_it;
        for(bits_it = bits.rbegin(); bits_it != bits.rend(); bits_it++)
        {
            RegisterBitmap* bit = *bits_it;
            if(bit)
            {
                bit->sort();
                decl << serialize_bitmap_definition(component, reg, *bit, reg.getWidth());
            }
        }
    }
    else
    {
        RegisterBitmap* bit = new RegisterBitmap("r32");
        bit->setDescription("Direct access to the register data.");
        bit->setStart(reg.getWidth() - 1);
        bit->setStop(0);
        bit->setType(RegisterBitmap::ReadWrite);
        decl << serialize_bitmap_definition(component, reg, *bit, reg.getWidth());
    }

    decl << indent(-1) <<"\\end{longtabu}" << endl;
    decl << indent(-1) <<"\\end{center}" << endl << endl;

#if 0
    decl << indent() << "/** @brief Register definition for @ref " << componentname << "_t." << camelcase(regname) << ". */ " << endl;
    decl << indent() << " typedef union {" << endl;
    indent(1);

    int i = width;
    while(i > 0)
    {
        decl << indent() << "/** @brief " << std::dec << i << "bit direct register access. */ " << endl;
        if(width / i > 1)
        {
            decl << indent() << " r" << std::dec << i << "[" << width / i << "];" << endl;
        }
        else
        {
            decl << indent() << " r" << std::dec << i << ";" << endl;
        }

        if(i > 8) i /= 2;
        else i = 0;
    }

    if(!reg.get().empty())
    {
        decl << endl;
        decl << indent()  <<"struct {" << endl;
        indent(1);


        const std::list<RegisterBitmap*>& bits = reg.get();
        std::list<RegisterBitmap*>::const_iterator bits_it;
        int width = reg.getWidth();
        for(bits_it = bits.begin(); bits_it != bits.end(); bits_it++)
        {
            RegisterBitmap* bit = *bits_it;
            if(bit)
            {
                bit->sort();
                decl << serialize_bitmap_declaration(component, reg, *bit, width);
            }
        }

         decl << indent(-1) << "} bits;" << endl;
     }
    decl << indent(-1) << "} Reg" <<  componentname << camelcase(regname) << "_t;" << endl << endl;

#endif
    return decl.str();;
}

string LaTeXWriter::camelcase(const string& str)
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

string LaTeXWriter::serialize_register_declaration(Component& component, Register& reg)
{
    const string& regname = reg.getName();
    ostringstream decl;

    decl << indent() << "0x" << std::hex << reg.getAddr() + component.getBase() << " & ";
    decl << escape(regname) << " & ";
    decl << accessType(RegisterBitmap::ReadWrite) << " & ";
    decl << "" << " & ";
    decl << escape(component.getName());

    decl << " \\\\ \\hline" << endl;


    return decl.str();
}

std::string    LaTeXWriter::serialize_component_declaration(Component& component)
{
    const string& componentname = component.getName();
    ostringstream decl;

    const std::list<Register*>& regs = component.get();
    std::list<Register*>::const_iterator it;


    decl << "\\section{" << componentname << "}" << endl;
    decl << "\\justify" << endl;
    decl << endl;
    //indent(1);


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

void LaTeXWriter::strreplace(string& origstr, const string& find, const string& replace)
{
    if(replace == find) return;

    int findlen = find.length();
    while (origstr.find(find) != string::npos)
    {
        origstr.replace(origstr.find(find), findlen, replace);
    }
}

bool LaTeXWriter::write(Components& components)
{


    ostringstream output;

    string prefix(header_prefix);
    string suffix(header_suffix);

    string guard(mFilename);
    std::transform(guard.begin(), guard.end(), guard.begin(), ::toupper);
    std::replace(guard.begin(), guard.end(), '.', '_');
    std::replace(guard.begin(), guard.end(), '/', '_');


    // <FILE>
    // <PROJECT>
    // <DESCRIPTION>
    // <GUARD>
    strreplace(prefix, "<FILE>", mFilename);
    strreplace(prefix, "<PROJECT>", (*gOptions)["project"].c_str());
    strreplace(prefix, "<GUARD>", guard);

    strreplace(suffix, "<GUARD>", guard);

    output << prefix;

    output << "\\section{Memory Map}" << endl;
    output << "\\justify" << endl;
    output << indent() << "\\begin{center}" << endl;
    output << indent(1) << "\\rowcolors{1}{blue}{liteblue}" << endl;
    output << indent() << "\\begin{longtabu} to \\textwidth{ | r | X | l | l | c |}" << endl;
    output << indent(1) << "\\showrowcolors" << endl;
    output << indent() << "\\color{white} Address & \\color{white} Register Name & \\color{white} CPU Access & \\color{white} Reset Source & \\color{white} Module \\\\ \\hline" << endl;
    output << indent() << "\\hiderowcolors" << endl;
    output << indent() << "\\endhead % all the lines above this will be repeated on every page" << endl;
    output << indent() << "\\hline % Ensure end of table has a line" << endl;
    output << indent() << "\\endfoot" << endl;

    const std::list<Component*> &componentList = components.get();
    std::list<Component*>::const_iterator it;
    for(it = componentList.begin(); it != componentList.end(); it++)
    {
        Component* component = *it;
        if(component)
        {
            component->sort();

            const std::list<Register*>& regs = component->get();
            std::list<Register*>::const_iterator it;

            for(it = regs.begin(); it != regs.end(); it++)
            {
                Register* reg = *it;


                if(reg)
                {
                    std::list<Register*>::const_iterator next_it = it;
                    next_it++;


                    const string& regname = reg->getName();

                    output << indent() << "0x" << std::hex << reg->getAddr() + component->getBase() << " & ";
                    output << escape(regname) << " & ";
                    output << accessType(RegisterBitmap::ReadWrite) << " & ";
                    output << "" << " & ";
                    if(it == regs.begin())
                    {
                        output << "\\multirow{" << std::dec << regs.size() << "}{*}{" << escape(component->getName()) << "}";
                    }
                    if(next_it != regs.end())
                    {
                        output << " \\\\ \\cline{1-4}" << endl;

                    }
                    else
                    {
                        output << " \\\\ \\hline" << endl;
                    }



                }
            }
            output << endl;
        }
    }
    output << indent(-1) <<"\\end{longtabu}" << endl;
    output << indent(-1) <<"\\end{center}" << endl << endl;

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

    output << suffix;

    mFile << output.str();
    cout << output.str();
    return true;
}
