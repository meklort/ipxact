////////////////////////////////////////////////////////////////////////////////
///
/// @file       includes/LaTeXWriter.hpp
///
/// @project    ipxact
///
/// @brief      LaTeX Writer.
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

#ifndef LATEXWRITER_H
#define LATEXWRITER_H

#include <Writer.hpp>

#include <string>
#include <algorithm>

class RegisterBitmap;

class LaTeXWriter : public Writer
{
public:
    LaTeXWriter(const char* filename);
    ~LaTeXWriter();

    virtual bool write(Components& components);

protected:
    virtual std::string serialize_bitmap_definition(Component& component, Register& reg, RegisterBitmap& bitmap, int regwidth);
    virtual std::string serialize_bitmap_declaration(Component& component, Register& reg, RegisterBitmap& bitmap, int regwidth);

    virtual std::string serialize_enum_definition(Component& component, Register& reg, RegisterBitmap& bitmap, Enumeration& thisenum);

    virtual std::string serialize_register_definition(Component& component, Register& reg);
    virtual std::string serialize_register_declaration(Component& component, Register& reg);

    virtual std::string serialize_component_declaration(Component& component);

    virtual std::string camelcase(const std::string& str);

    virtual std::string indent(int modifier = 0);

    virtual std::string accessType(RegisterBitmap::Type type) const;
    virtual std::string escape(const std::string instring) const;
private:
    int mIndent;
    char* mFilename;

    void strreplace(std::string& origstr, const std::string& find, const std::string& replace);
};

#endif /* !LATEXWRITER_H */
