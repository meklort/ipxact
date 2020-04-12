////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/HeaderWriter.cpp
///
/// @project    ipxact
///
/// @brief      C Header Writer
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

#include <HeaderWriter.hpp>
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

HeaderWriter::HeaderWriter(const char* filename) : Writer(filename)
{
    mFilename = strdup(filename);
    mIndent = 0;
}

HeaderWriter::~HeaderWriter()
{
    if(mFilename) free(mFilename);
}

std::string HeaderWriter::getComponentFile(const char* componentname)
{
    string retstr = mFilename;
    retstr = retstr.substr(0, retstr.find_last_of("."));
    retstr += + "_" + string(componentname) + ".h";

    return retstr;
}

std::string HeaderWriter::get_type_name(Component& component)
{
    string componentTypeID = component.getTypeID();
    string componentname = componentTypeID.empty() ? component.getName() : componentTypeID;
    std::transform(componentname.begin(), componentname.end(), componentname.begin(), ::toupper);
    componentname += "_t";

    return componentname;
}

std::string HeaderWriter::get_type_name(Component& component, Register& reg)
{
    string componentTypeID = component.getTypeID();
    string componentname = componentTypeID.empty() ? component.getName() : componentTypeID;
    std::transform(componentname.begin(), componentname.end(), componentname.begin(), ::toupper);

    string registerTypeID = reg.getTypeID();

    string regname       = registerTypeID.empty() ? reg.getName() : registerTypeID;
    string reg_type = "Reg" +  componentname + camelcase(regname) + "_t";

    return reg_type;
}

std::string HeaderWriter::type(int width, bool isSigned) const
{

    string guard(mFilename);
    std::transform(guard.begin(), guard.end(), guard.begin(), ::toupper);
    std::replace(guard.begin(), guard.end(), '.', '_');
    std::replace(guard.begin(), guard.end(), '/', '_');

    string regtype = guard + "_";

    switch(isSigned)
    {
        case true: regtype += ""; break;
        case false: regtype += "u"; break;
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

std::string HeaderWriter::indent(int modifier)
{
    ostringstream indent;
    mIndent += modifier;
    for(int i = 0; i < mIndent; i++)
    {
        indent << "    ";
    }
    return indent.str();
}

string HeaderWriter::serialize_enum_definition(Component& component, Register& reg, RegisterBitmap& bitmap, Enumeration& thisenum)
{
    string enumname = thisenum.getName();
    string bitmapname = bitmap.getName();

    string registerTypeID   = reg.getTypeID();
    string componentTypeID  = component.getTypeID();

    string regname          = registerTypeID.empty() ? reg.getName() : registerTypeID;
    string componentname    = componentTypeID.empty() ? component.getName() : componentTypeID;

    escape(enumname);
    escape(bitmapname);
    escape(regname);
    escape(componentname);

    std::transform(bitmapname.begin(),    bitmapname.end(),    bitmapname.begin(),    ::toupper);
    std::transform(regname.begin(),       regname.end(),       regname.begin(),       ::toupper);
    std::transform(componentname.begin(), componentname.end(), componentname.begin(), ::toupper);
    std::transform(enumname.begin(), enumname.end(), enumname.begin(), ::toupper);

    ostringstream decl;

    decl << "#define     " << componentname << "_" << regname << "_" << bitmapname << "_" << enumname << " 0x" << std::hex << thisenum.getValue() << "u" << endl;

    return decl.str();
}

string HeaderWriter::serialize_bitmap_definition(Component& component, Register& reg, RegisterBitmap& bitmap, int regwidth)
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
    decl << "#define     " << componentname << "_" << regname << "_" << bitmapname << "_SHIFT " << bitmap.getStop() << "u" << endl;
    decl << "#define     " << componentname << "_" << regname << "_" << bitmapname << "_MASK  0x" << std::hex << mask << "u" << endl;
    decl << "#define GET_" << componentname << "_" << regname << "_" << bitmapname << "(__reg__)  (((__reg__) & 0x" << std::hex << mask << ") >> " << std::dec << bitmap.getStop() << "u)" << endl;
    decl << "#define SET_" << componentname << "_" << regname << "_" << bitmapname << "(__val__)  (((__val__) << "  << std::dec << bitmap.getStop() << "u) & 0x" << std::hex << mask << "u)" << endl;

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

string HeaderWriter::serialize_bitmap_declaration(Component& component, Register& reg, RegisterBitmap& bitmap, int regwidth)
{
    int bitwidth = (bitmap.getStart() - bitmap.getStop() + 1);
    ostringstream decl;
    decl << indent() << "/** @brief " << bitmap.getDescription() << " */" << endl;

    string bitmapname(bitmap.getName());
    escapeEnum(bitmapname);
    if(isdigit(bitmapname[0]))
    {
        // Ensure variables start with a valid identifier.
        bitmapname = string("_") + bitmapname;
    }

    if(bitwidth != regwidth)
    {
        decl << indent() << "BITFIELD_MEMBER(" << type(regwidth, false) << ", " << bitmapname << ", " << bitmap.getStop() << ", " << bitwidth << ")" << endl;

        // decl << indent() << type(regwidth, false) << " " << bitmapname << ":" << bitwidth << ";" << endl;
    }
    else
    {
        decl << indent() << "BITFIELD_MEMBER(" << type(regwidth, false) << ", " << bitmapname << ", " << bitmap.getStop() << ", " << bitwidth << ")" << endl;
        // decl << indent() << type(regwidth, false) << " " << bitmapname << ";" << endl;
    }

    return decl.str();
}

#if 0
static int getExpectedWidth(RegisterBitmap& bit, std::list<RegisterBitmap*>::const_iterator& next, const std::list<RegisterBitmap*>& bits)
{
    int base_bit = bit.getStop();
    int end_bit  = bit.getStart() + 1;
    int minWidth = end_bit - base_bit;
    int width;
    // We are at a byte boundary, see if all of the bitfields fit in an 8bit value.
    std::list<RegisterBitmap*>::const_iterator bits_next = next;
    bits_next++;
    for(; bits_next != bits.end(); bits_next++)
    {
        if((end_bit % 8))
        {
            if(*bits_next)
            {

                end_bit = (*bits_next)->getStart() + 1;

                if(((*bits_next)->getType()) == RegisterBitmap::Reserved)
                {
                    width = (end_bit - base_bit);

                    if(width >= 8)
                    {
                        end_bit = (*bits_next)->getStop();
                        width = end_bit - base_bit;

                        if(width % 8)
                        {
                            // round up to next 8 bit entry for the min width.
                            width += 8 - (width % 8);
                        }


                        end_bit = (*bits_next)->getStart() + 1;
                        int max_width = end_bit - base_bit;
                        if(max_width % 8)
                        {
                            // round up to next 8 bit entry for the max width.
                            max_width += 8 - (max_width % 8);
                        }
                        if(max_width == 24 && width >= minWidth) // not allowed, error towards small
                        {
                            return width;
                        }
                        else
                        {
                            return max_width;
                        }

                        printf("splitting reserved to width %d for bitfield %s\n", width, bit.getName().c_str());
                        return width;
                    }
                }
                else
                {
                    end_bit = (*bits_next)->getStart() + 1;
                }
            }
        }
    }
    width = end_bit - base_bit;

    //cout << bit.getName() << " width is " << width << "(s:" << base_bit << " e: " << end_bit << ")" << endl;
    return width;
}

static int getNextExpectedWidth(RegisterBitmap& bit, std::list<RegisterBitmap*>::const_iterator& next, const std::list<RegisterBitmap*>& bits)
{
    //int base_bit = bit.getStop();
    int base_bit = 0;

    // We are at a byte boundary, see if all of the bitfields fit in an 8bit value.
    std::list<RegisterBitmap*>::const_iterator bits_next = next;
    bits_next++;
    for(; bits_next != bits.end(); bits_next++)
    {
        RegisterBitmap* next_bit = *bits_next;
        if(next_bit)
        {
            base_bit = (*bits_next)->getStop();

            if(0 == (base_bit % 8))
            {
                //cout << "next: ";
                // this bit is perfectly valid, check for it's next.
                return  getExpectedWidth(*next_bit, bits_next, bits);
            }
        }
    }

    return 0;
}
#endif

string HeaderWriter::serialize_bitmap_constructor(Component& component, Register& reg, RegisterBitmap& bitmap, int regwidth)
{
    ostringstream decl;
    string bitmapname(bitmap.getName());
    escapeEnum(bitmapname);
    if(isdigit(bitmapname[0]))
    {
        // Ensure variables start with a valid identifier.
        bitmapname = string("_") + bitmapname;
    }

    int width = reg.getWidth();

    string bitvar   = string("bits.") + bitmapname;
    string basename = string("r") + to_string(width);


    decl << indent() << bitvar << ".setBaseRegister(&" + basename + ");" << endl;
    decl << indent() << bitvar << ".setName(\"" + bitmapname + "\");" << endl;

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

                string enumname = (*thisenum).getName();
                int value = (*thisenum).getValue();

                decl << indent() << bitvar << ".addEnum(\"" << enumname << "\", 0x" << std::hex << value << ");" << endl;

                // decl << serialize_enum_definition(component, reg, bitmap, *thisenum);
            }
        }

         decl << endl;
    }

    return decl.str();
}


string HeaderWriter::serialize_register_constructor(Component& component, Register& reg)
{
    string regname = reg.getName();
    string componentType = get_type_name(component);

    std::transform(regname.begin(),       regname.end(),       regname.begin(),       ::toupper);

    ostringstream decl;

    decl << indent() << "/** @brief constructor for @ref " << componentType << "." << camelcase(regname) << ". */" << endl;
    int width = reg.getWidth();

    string basename = string("r") + to_string(width);
    decl << indent() << basename << ".setName(\"" + camelcase(regname) + "\");" << endl;

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
                decl << serialize_bitmap_constructor(component, reg, *bit, width);
            }
        }
     }

    return decl.str();
}

static string convert_single_bitmap(HeaderWriter& writer, Component& component, Register& reg, RegisterBitmap* bit, int &prev_position, RegisterBitmap &padding)
{
    ostringstream decl;
    if(bit)
    {
        string regname = reg.getName();
        string componentname = component.getName();

        std::transform(regname.begin(),       regname.end(),       regname.begin(),       ::toupper);
        std::transform(componentname.begin(), componentname.end(), componentname.begin(), ::toupper);
        int width = reg.getWidth();
        int base_bit = bit->getStop();
        if(base_bit % 8 == 0)
        {
            int nextWidth = reg.getWidth();//getNextExpectedWidth(*bit, bits_it, bits);
            width = reg.getWidth();//getExpectedWidth(*bit, bits_it, bits);
            if(width % 8)
            {
                fprintf(stdout, "Warning: %s has an unexpected bit width of %d.\n", (*bit).getName().c_str(), width);

                // Bitfield has missing bits.
                width += (8 - width % 8);
            }

            if(nextWidth == 24 || (nextWidth == 16 && base_bit == 0 && width == 8))
            {
                fprintf(stdout, "Warning: converting 8bit field  %s to 32bit due to next entry requiring 24bits.\n", (*bit).getName().c_str());
                width = 32;
            }

            if(width == 24)
            {
                if(base_bit > 8) // promote due to above. next expected width
                {
                    fprintf(stdout, "Error: unexpected promotion of 24bit field %s to 32bits.\n", (*bit).getName().c_str());
                }
                width = 32; // 24bit width not allowed, convert to 32.
            }

            // if((base_bit ==  8 || base_bit == 24) && width == 16)
            // {
            //     fprintf(stdout, "Warning: attempting to create unaligned bitfield %s, switching to 32 bits.\n", (*bit).getName().c_str());
            //     width = 32;
            // }

            if((base_bit == 8))

            if(width == 0)
            {
                fprintf(stdout, "%s.%s.%s start bit is %d, end is %d.\n",
                    componentname.c_str(), writer.camelcase(regname).c_str(), (*bit).getName().c_str(), base_bit, bit->getStart() + 1);
                width = 32;
            }
        }

        // width = 32;
        bit->sort();
        if(width <= bit->getStart() - bit->getStop())
        {
            fprintf(stdout, "Warning: bitfield  %s has an expected width of %d.\n", (*bit).getName().c_str(), width);

            // split the register into two or more.
            std::string origName = bit->getName();
            int actualStart = bit->getStart();
            int actualStop = bit->getStop();
            int actualWidth = actualStart - bit->getStop();
            int currentBit = 0;
            while(currentBit < actualWidth)
            {
                ostringstream name;
                currentBit += width;
                int currentStop = currentBit + actualStop;
                currentStop = currentStop / 8;
                currentStop = currentStop * 8;
                if(currentStop > actualStart)
                {
                    currentStop = actualStart + 1;
                }
                bit->setStart(currentStop - 1);

                name << "reserved" << "_" << std::dec << (int)bit->getStart() << "_" << (int)bit->getStop();

                bit->setName(name.str());

                decl << writer.serialize_bitmap_declaration(component, reg, *bit, width);

                // set next start.
                bit->setStop(currentStop);
            }
            bit->setName(origName);
            bit->setStart(actualStart);
            bit->setStop(actualStop);
        }
        else
        {
            if(prev_position != bit->getStop())
            {
                // Bits have a gap. Add some padding.
                int reservedStart = ((int)bit->getStop() - 1);
                int reservedStop = prev_position;
                int currentStop = reservedStop;
                int loops = 0;
                while (currentStop < reservedStart+1)
                {
                    loops++;
                    int newStart;
                    // FUnd max number of bits possible
                    int maxBits = width - (currentStop % width);
                    newStart = currentStop + maxBits - 1 < reservedStart ? currentStop + maxBits - 1 : reservedStart;

                    // cout << "newStart = " << newStart << endl;


                    ostringstream name;
                    // Pad out to the needed position
                    name << "reserved" << "_" << std::dec << newStart << "_" << currentStop;
                    cout << "Adding padding " << name.str() << "(width: " << width << ")" << endl;
                    cout << "Error: " << bit->getName() << " bit position gap: " << newStart << " to " << currentStop << endl;
                    // exit(-1);
                    // RegisterBitmap padding(name.str());
                    padding.setName(name.str());
                    padding.setStart(newStart);
                    padding.setStop(currentStop);
                    padding.setDescription("Padding");
                    // decl << writer.serialize_bitmap_declaration(component, reg, padding, width);

                    currentStop = newStart + 1; //

                }
                if(loops > 1)
                {
                    cout << "reduced." << endl;
                    // /exit(-1);
                }
            }
            decl << writer.serialize_bitmap_declaration(component, reg, *bit, width);
        }

        prev_position = bit->getStart() + 1;
        cout << "Wrote bit " << bit->getName() << " from " << bit->getStart() << " to " << bit->getStop() << endl;
    }
    return decl.str();;
}

string HeaderWriter::serialize_register_definition(Component& component, Register& reg)
{
    int width = reg.getWidth();
    string regname = reg.getName();
    string componentname = component.getName();
    string registerType = get_type_name(component, reg);
    string componentType = get_type_name(component);

    std::transform(regname.begin(),       regname.end(),       regname.begin(),       ::toupper);
    std::transform(componentname.begin(), componentname.end(), componentname.begin(), ::toupper);

    ostringstream decl;
    string defregname = regname;
    decl <<  "#define REG_" << componentname << "_" << escape(defregname) << " ((volatile " <<  type(reg.getWidth(), false) << "*)0x" << std::hex << (component.getBase() + reg.getAddr()) << ") /* " << reg.getDescription() << " */" << endl;

    if(!(component.isTypeIDCopy() || reg.isTypeIDCopy()))
    {
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

             decl << endl;
         }

        decl << indent() << "/** @brief Register definition for @ref " << componentType << "." << camelcase(regname) << ". */" << endl;
        decl << indent() << "typedef register_container " << registerType << " {" << endl;
        indent(1);

        int i = width;
        while(i > 0)
        {
            decl << indent() << "/** @brief " << std::dec << i << "bit direct register access. */" << endl;
            if(width / i > 1)
            {
                decl << indent() << type(i, false) << " r" << std::dec << i << "[" << width / i << "];" << endl;
            }
            else
            {
                decl << indent() << type(i, false) << " r" << std::dec << i << ";" << endl;
            }

            // if(i > 8) i /= 2;
            // else i = 0;
            i = 0;
        }

        if(!reg.get().empty())
        {
            decl << endl;
            // decl << indent()  <<"struct {" << endl;
            decl << indent() << "BITFIELD_BEGIN(" << type(width, false) << ", bits)" << endl;
            decl << "#if defined(__LITTLE_ENDIAN__)" << endl;

            indent(1);

            const std::list<RegisterBitmap*>& bits = reg.get();
            std::list<RegisterBitmap*>::const_iterator bits_it;
            int width = reg.getWidth();
            int prev_position = 0;
            string reverse_order;
            RegisterBitmap* lastbit = NULL;
            RegisterBitmap padding("none");
            for(bits_it = bits.begin(); bits_it != bits.end(); bits_it++)
            {
                RegisterBitmap* bit = *bits_it;
                lastbit = *bits_it;
                padding.setName("none");
                string bitmap_str = convert_single_bitmap(*this, component, reg, bit, prev_position, padding);
                if("none" != padding.getName())
                {
                    RegisterBitmap padding_nop("nop");
                    // padding needed.
                    int prev_position_nop = prev_position;
                    string padding_str = convert_single_bitmap(*this, component, reg, &padding, prev_position_nop, padding_nop);
                    decl << padding_str;
                    reverse_order = padding_str + reverse_order;
                }
                decl << bitmap_str;
                reverse_order = bitmap_str + reverse_order;
            }

            if(lastbit && (lastbit->getStart() + 1 != width))
            {
                int stop = lastbit->getStart() + 1;
                RegisterBitmap padding_nop("nop");
                ostringstream name;
                // Pad out to the needed position
                name << "reserved" << "_" << std::dec << (width - 1) << "_" << stop;
                padding.setName(name.str());
                padding.setStart(width - 1);
                padding.setStop(stop);
                padding.setDescription("Padding");

                // padding needed
                string padding_str = convert_single_bitmap(*this, component, reg, &padding, prev_position, padding_nop);
                decl << padding_str;
                reverse_order = padding_str + reverse_order;

            }
            decl << "#elif defined(__BIG_ENDIAN__)" << endl;

            decl << reverse_order;
            decl << "#else" << endl;
            decl << "#error Unknown Endian" << endl;
            decl << "#endif" << endl;
            decl << indent(-1) << "BITFIELD_END(" << type(width, false) << ", bits)" << endl;
             // decl << indent(-1) << "} bits;" << endl;
        }

        decl << "#ifdef CXX_SIMULATOR" << endl;
        decl << indent() << "/** @brief Register name for use with the simulator. */" << endl;
        decl << indent() << "const char* getName(void) { return \"" << camelcase(regname) << "\"; }" << endl << endl;

        decl << indent() << "/** @brief Print register value. */" << endl;
        decl << indent() << "void print(void) { r" << width << ".print(); }" << endl << endl;

        decl << indent() << registerType << "()" << endl;
        decl << indent() << "{" << endl;
        indent(1);
        decl << serialize_register_constructor(component, reg);
        indent(-1);
        decl << indent() << "}" << endl;
        decl << indent() <<  registerType << "& operator=(const " << registerType << "& other)" << endl;
        decl << indent() << "{" << endl;
        decl << indent(1) << "r" << width << " = other.r" << width << ";" << endl;
        decl << indent() << "return *this;" << endl;
        indent(-1);
        decl << indent() << "}" << endl;
        decl << "#endif /* CXX_SIMULATOR */" << endl;

        decl << indent(-1) << "} " << registerType << ";" << endl << endl;
    }

    return decl.str();
}

string& HeaderWriter::escapeEnum(std::string& str)
{
    while(str.find(" ") != std::string::npos)
    {
        str.replace(str.find(" "), 1, ""); // Replace @'s with _AT_'s.
    }

    return escape(str);
}


string& HeaderWriter::escape(std::string& str)
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

string HeaderWriter::camelcase(const string& str)
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

string HeaderWriter::serialize_register_declaration(Component& component, Register& reg)
{
    const string& regname = reg.getName();
    ostringstream decl;
    unsigned int dim = reg.getDimensions();
    string registerType = get_type_name(component, reg);


    string newname = regname;
    newname = camelcase(escape(newname));
    if(isdigit(regname[0]))
    {
        newname = string("_") + newname;
        cout << "Invalid: " << newname << endl;
        // exit(-1);
    }

    decl << indent() << "/** @brief " << reg.getDescription() << " */" << endl;
    if(dim > 1)
    {
        decl << indent() << registerType << " " << newname << "[" << dim <<  "];" << endl << endl;

    }
    else
    {
        decl << indent() << registerType << " " << newname << ";" << endl << endl;
    }


    return decl.str();
}

std::string HeaderWriter::serialize_component_declaration(Component& component)
{
    const string& componentname = component.getName();
    string componentType = get_type_name(component);
    ostringstream decl;

    const std::list<Register*>& regs = component.get();
    std::list<Register*>::const_iterator it;

    decl <<  "#define REG_" << componentname << "_BASE" << " ((volatile void*)0x" << std::hex << (component.getBase()) * (component.getAddressUnitBits() / 8u) << ") /* " << component.getDescription() << " */" << endl;
    if(component.getRange())
    {
        decl <<  "#define REG_" << componentname << "_SIZE" << " (0x" << std::hex << (component.getRange()) << ")" << endl;
    }
    else
    {
        decl <<  "#define REG_" << componentname << "_SIZE" << " (sizeof(" << componentType << "))" << endl;

    }
    decl << endl;

    for(it = regs.begin(); it != regs.end(); it++)
    {
        Register* reg = *it;
        if(reg)
        {
            reg->sort();
            decl << serialize_register_definition(component, *reg);
        }
    }

    if(!component.isTypeIDCopy())
    {
        decl << indent() << "/** @brief Component definition for @ref " << componentname << ". */" << endl;
        decl << indent() << "typedef struct " << componentType << " {" << endl;
        indent(1);

        Register* prevreg = NULL;
        for(it = regs.begin(); it != regs.end(); it++)
        {
            Register* reg = *it;

            if(reg)
            {
                // Ensure we don't have any gaps.
                int padding = 0;
                int expStart = 0;
                if(prevreg)
                {
                    int width = prevreg->getWidth() / component.getAddressUnitBits(); // in bytes.
                    expStart  = prevreg->getAddr() + (width * prevreg->getDimensions());
                    padding   = reg->getAddr() - expStart;
                }
                else
                {
                    expStart = 0;
                    padding  = reg->getAddr();
                }

                if(padding)
                {
                    if(padding > 0)
                    {
                        if(prevreg)
                        {
                            fprintf(stdout, "Info: adding %d bytes of padding between register %s and %s.\n", padding, prevreg->getName().c_str(), reg->getName().c_str());
                        }
                        else
                        {
                            fprintf(stdout, "Info: adding %d bytes of padding before first register %s.\n", padding, reg->getName().c_str());
                        }
                        int padwidth = component.getAddressUnitBits();
                        // 8 -> 16
                        if(padwidth <= 16 && 0 == padding % 2)
                        {
                            padding /= 2;
                            padwidth *= 2;

                        }
                        // 16 -> 32
                        if(padwidth <= 16 && 0 == padding % 2)
                        {
                            padding /= 2;
                            padwidth *= 2;

                        }

                        cout << indent() << "/** @brief " << "Reserved bytes to pad out data structure." << " */" << endl;
                        cout << indent() << type(padwidth, false) << " reserved_" << std::dec << expStart << "[" << padding << "];" << endl;


                        decl << indent() << "/** @brief " << "Reserved bytes to pad out data structure." << " */" << endl;
                        decl << indent() << type(padwidth, false) << " reserved_" << std::dec << expStart << "[" << padding << "];" << endl;
                        decl << endl;
                    }
                    else
                    {
                        if(prevreg)
                        {
                            fprintf(stderr, "Error: requested %d bytes of padding between component '%s' registers '%s' and '%s'.\n",
                                padding, componentname.c_str(), prevreg->getName().c_str(), reg->getName().c_str());
                        }
                        else
                        {
                            fprintf(stderr, "Error: requested %d bytes of padding before component %s's first register '%s'.\n",
                                padding, componentname.c_str(), reg->getName().c_str());
                        }
                        while(1);
                    }
                }

                reg->sort();
                decl << serialize_register_declaration(component, *reg);
            }
            prevreg = reg;
        }

        decl << "#ifdef CXX_SIMULATOR" << endl;

        decl << indent() << "typedef uint32_t (*callback_t)(uint32_t, uint32_t, void*);" << endl;
        decl << indent() << "callback_t mIndexReadCallback;" << endl;
        decl << indent() << "void* mIndexReadCallbackArgs;" << endl << endl;

        decl << indent() << "callback_t mIndexWriteCallback;" << endl;
        decl << indent() << "void* mIndexWriteCallbackArgs;" << endl << endl;

        decl << indent() << componentType << "() : mIndexReadCallback(0), mIndexReadCallbackArgs(0), mIndexWriteCallback(0), mIndexWriteCallbackArgs(0)" << endl;
        decl << indent() << "{" << endl;
        indent(1);
        prevreg = NULL;
        for(it = regs.begin(); it != regs.end(); it++)
        {
            Register* reg = *it;

            if(reg)
            {
                int width = reg->getWidth();
                int dim = reg->getDimensions();
                // Ensure we don't have any gaps.
                int padding = 0;
                int expStart = 0;
                if(prevreg)
                {
                    int width = prevreg->getWidth() / component.getAddressUnitBits(); // in bytes.
                    expStart  = prevreg->getAddr() + (width * prevreg->getDimensions());
                    padding   = reg->getAddr() - expStart;
                }
                else
                {
                    expStart = 0;
                    padding  = reg->getAddr();
                }

                if(padding)
                {
                    if(padding > 0)
                    {
                        if(0 == padding % 4)
                        {
                            padding /= 4;
                        }
                        else if(0 == padding % 2)
                        {
                            padding /= 2;
                        }

                        decl << indent() << "for(int i = 0; i < " << std::dec << padding << "; i++)" << endl;
                        decl << indent() << "{" << endl;
                        indent(1);
                        decl << indent() << "reserved_" << std::dec << expStart << "[i].setComponentOffset(0x" << std::hex << expStart << " + (i * " << to_string(width/8) << "));" << endl;
                        decl << indent(-1) << "}" << endl;

                    }
                }
                prevreg = reg;

                string regname = reg->getName();
                std::transform(regname.begin(),       regname.end(),       regname.begin(),       ::toupper);

                string newname = regname;
                newname = camelcase(escape(newname));
                if(isdigit(regname[0]))
                {
                    newname = string("_") + newname;
                    cout << "Invalid: " << newname << endl;
                    // exit(-1);
                }

                if(dim > 1)
                {
                    string basename = newname + string("[i].r") + to_string(width);
                    decl << indent() << "for(int i = 0; i < " << std::dec << dim << "; i++)" << endl;
                    decl << indent() << "{" << endl;
                    indent(1);
                    if(!reg->getTypeID().empty())
                    {
                        // Override the .r32 name to match the variable.
                        decl << indent() << basename << ".setName(\"" << newname << "\");" << endl;
                    }
                    decl << indent() << basename << ".setComponentOffset(0x" << std::hex << reg->getAddr() << " + (i * " << to_string(width/8) << "));" << endl;
                    decl << indent(-1) << "}" << endl;

                }
                else
                {
                    string basename = newname + string(".r") + to_string(width);
                    if(!reg->getTypeID().empty())
                    {
                        // Override the .r32 name to match the variable.
                        decl << indent() << basename << ".setName(\"" << newname << "\");" << endl;
                    }
                    decl << indent() << basename << ".setComponentOffset(0x" << std::hex << reg->getAddr() << ");" << endl;
                }
            }
        }
        decl << indent(-1) << "}" << endl;

        decl << indent() << "void print()" << endl;
        decl << indent() << "{" << endl;
        indent(1);
        prevreg = NULL;
        for(it = regs.begin(); it != regs.end(); it++)
        {
            Register* reg = *it;

            if(reg)
            {
                // Ensure we don't have any gaps.
                int padding = 0;
                int expStart = 0;
                if(prevreg)
                {
                    int width = prevreg->getWidth() / component.getAddressUnitBits(); // in bytes.
                    expStart  = prevreg->getAddr() + (width * prevreg->getDimensions());
                    padding   = reg->getAddr() - expStart;
                }
                else
                {
                    expStart = 0;
                    padding  = reg->getAddr();
                }

                if(padding)
                {
                    if(padding > 0)
                    {
                        if(0 == padding % 4)
                        {
                            padding /= 4;
                        }
                        else if(0 == padding % 2)
                        {
                            padding /= 2;
                        }

                        decl << indent() << "for(int i = 0; i < " << std::dec << padding << "; i++)" << endl;
                        decl << indent() << "{" << endl;
                        indent(1);
                        decl << indent() << "reserved_" << std::dec << expStart << "[i].print();" << endl;
                        decl << indent(-1) << "}" << endl;

                    }
                }
                prevreg = reg;


                string regname = reg->getName();
                std::transform(regname.begin(),       regname.end(),       regname.begin(),       ::toupper);

                string newname = regname;
                newname = camelcase(escape(newname));
                if(isdigit(regname[0]))
                {
                    newname = string("_") + newname;
                    cout << "Invalid: " << newname << endl;
                    // exit(-1);
                }

                int dim = reg->getDimensions();
                if(dim > 1)
                {
                    string basename = newname + string("[i]");
                    decl << indent() << "for(int i = 0; i < " << std::dec << dim << "; i++)" << endl;
                    decl << indent() << "{" << endl;
                    indent(1);
                    decl << indent() << basename << ".print();" << endl;
                    decl << indent(-1) << "}" << endl;

                }
                else
                {
                    decl << indent() << newname << ".print();" << endl;
                }
            }
        }
        decl << indent(-1) << "}" << endl;

        decl << indent() << "uint32_t read(int offset) { return mIndexReadCallback(0, offset, mIndexReadCallbackArgs); }" << endl;
        decl << indent() << "void write(int offset, uint32_t value) { (void)mIndexWriteCallback(value, offset, mIndexWriteCallbackArgs); }" << endl;
        decl << "#endif /* CXX_SIMULATOR */" << endl;

        decl << indent(-1) << "} " << componentType << ";" << endl << endl;
    }

    decl << indent() << "/** @brief " << component.getDescription() << " */" << endl;
    decl << indent() << "extern volatile " << componentType << " " << componentname << ";"<< endl << endl;

    return decl.str();
}

void HeaderWriter::strreplace(string& origstr, const string& find, const string& replace)
{
    if(replace == find) return;

    int findlen = find.length();
    while (origstr.find(find) != string::npos)
    {
        origstr.replace(origstr.find(find), findlen, replace);
    }
}

bool HeaderWriter::write(Components& components)
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

bool HeaderWriter::writeComponent(Component &component)
{
    const string& componentname = component.getName();
    string* header_contents = new RESOURCE_STRING(resources_HeaderWriter_h);

    string oldFIlename = mFilename;
    string filename = getComponentFile(componentname.c_str());

    string includePaths = "";
    if(component.isTypeIDCopy())
    {
        includePaths = "#include \"" + getComponentFile(component.getTypeIDCopy().c_str()) + "\"\n";
    }

    mFilename = strdup(filename.c_str());


    component.sort();
    strreplace(*header_contents, "<INCLUDES>", includePaths);
    UpdateTemplate(*header_contents, filename, component);
    strreplace(*header_contents, "<SERIALIZED>", serialize_component_declaration(component));
    strreplace(*header_contents, "<INIT_FUNCTIONS>", serialize_component_declaration(component));

    mFilename = strdup(oldFIlename.c_str());
    return WriteToFile(filename, *header_contents);
}
