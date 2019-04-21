////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/Number.cpp
///
/// @project    ipxact
///
/// @brief      Basic number parser with partial support for verilog numbers.
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
#include <Number.hpp>

#include <regular_expressions.hpp>

#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <iostream>
using namespace std;

const char VERILOG_PARSER_REGEX[]  = "\\s*([0-9]+)'([hbdo])([0-9A-Fa-f_]+)\\s*";
const char NUMBER_PARSER_REGEX[]  = "^\\s*([0]?[xX]?)([0-9A-Fa-f]+)\\s*$";

Number::Number(const string& numstring)
{
    mValid = false;
    mWidth = 0;
    mValue = 0;

    RegExp regex(VERILOG_PARSER_REGEX);
    RegExp numregex(NUMBER_PARSER_REGEX);
    if(regex.ExactMatch(numstring))
    {
        const string& bits   = regex.CaptureText(1);
        const string& type   = regex.CaptureText(2);
        const string& valstr = regex.CaptureText(3);

        // remove all '_'
        string no_under(valstr);
        std::remove(no_under.begin(), no_under.end(), '_');

        stringstream(bits) >> mWidth;
        mValid = true;
        // fixme
        if(type == "h")
        {
            stringstream(no_under) >> std::hex >> mValue;
        }
        else if(type == "o")
        {
            stringstream(no_under) >> std::oct >> mValue;
        }
        else if(type == "d")
        {
            stringstream(no_under) >> std::dec >> mValue;
        }
        else if(type == "b")
        {
            int bit = 0;
            for ( string::const_reverse_iterator it=no_under.rbegin(); it!=no_under.rend(); ++it)
            {
                if(*it == '1')
                {
                    mValue += (1 << bit);
                }
                else if(*it != '0')
                {
                    fprintf(stderr, "Invalid binary character '%c'", *it);
                    mValid = false;
                }
                bit++;
            }
        }
        else
        {
            fprintf(stderr, "Error: unable to handle base format of '%s'\n", type.c_str());
            mValid = false;
        }
    }
    else if(numregex.ExactMatch(numstring))
    {
        mValid = true;
        const string& type   = numregex.CaptureText(1);
        const string& valstr = numregex.CaptureText(2);

        if(type == "")
        {
            stringstream(valstr) >> std::dec >> mValue;
        }
        else if(type == "0x" || type == "0X")
        {
            stringstream(valstr) >> std::hex >> mValue;
        }
        else if(type == "0")
        {
            stringstream(valstr) >> std::oct >> mValue;
        }
        else
        {
            fprintf(stderr, "Error: unable to handle base format of '%s'\n", type.c_str());
            mValid = false;
        }
    }
}
