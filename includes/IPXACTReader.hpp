////////////////////////////////////////////////////////////////////////////////
///
/// @file       includes/IPXACTReader.hpp
///
/// @project    ipxact
///
/// @brief      IP-XACT Reader.
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

#ifndef IPXACTREADER_H
#define IPXACTREADER_H

#include <Reader.hpp>

#include <pugixml.hpp>

class IPXACTReader : public Reader
{
public:
    IPXACTReader(const char* filename, Components& components);
    ~IPXACTReader();


    virtual bool read();

private:
    virtual bool parseElement(pugi::xml_node& elem);

    virtual bool parseComponent(const pugi::xml_node& elem);
    virtual bool parseRegister(const pugi::xml_node& elem, Component& component, bool update = false);
    virtual bool parseRegisterBitmap(const pugi::xml_node& elem, Register& reg, bool update = false);
    virtual bool parseEnumerations(const pugi::xml_node& elem, RegisterBitmap& bitmap, bool update = false);
    virtual bool parsseEnumeration(const pugi::xml_node& elem, RegisterBitmap& bitmap, bool update = false);

    virtual RegisterBitmap::Type registerType(const std::string& type) const;

private:
    int mAddressUnitBits;
};

#endif /* !IPXACTREADER_H */
