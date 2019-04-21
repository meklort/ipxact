////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/XHTMLReader.cpp
///
/// @project    ipxact
///
/// @brief      Ortega XHTML Reader code
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

#include <XHTMLReader.hpp>
#include <Number.hpp>

#include <pugixml.hpp>
#include <stdio.h>

#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>

using namespace pugi;
using namespace std;

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}


XHTMLReader::XHTMLReader(const char* filename, Components& components) : Reader(filename, components)
{
}

XHTMLReader::~XHTMLReader()
{

}

bool XHTMLReader::read()
{
    string tmp;
    string xml;
    while(std::getline(mFile, tmp)) {
        xml += tmp;
    }

    xml_document doc;
    doc.load_string(xml.c_str());

    xml_node root = doc.document_element();
    if(root)
    {
        if(!parseElement(root))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}

bool XHTMLReader::parseElement(const pugi::xml_node& elem)
{
    bool status = true;
    // search for component block - ipxact:addressBlock
    const char* text = elem.value() ? elem.value() : "";
    const char* attr = elem.attribute("class").value();
    attr = attr ? attr : "";

    cout << "Parsing block " << elem.name() << " : " << text << " : " << attr << endl;

    // TODO: parse ipxact:library for project name

    for (xml_node child = elem.first_child(); child; child = child.next_sibling())
    {
        const char* id = child.attribute("id").value();
        id = id ? id : "";
        string idstr = id;
        // do something with each child element
        if(child.name() == string("section") &&
            id != string(""))
        {
            // parse registers
            cout << "**********************" << endl;

            if(!addComponent(child, idstr))
            {
                status = false;
            }
        }
        else if(child.name() == string("section"))
        {
            // Section with no ID - likely TOC
            // skip
        }
        else
        {
            status = status && parseElement(child);
        }
    }

    return status;
}

bool XHTMLReader::addRegister(const pugi::xml_node& elem, Component &component, int width)
{
    bool ismii = false;
    if(component.getName() == "MII")
    {
        ismii = true;
    }

    const char* childid = elem.attribute("id").value();
    childid = childid ? childid : "";

    size_t div = string(childid).find("-");
    if(div == string::npos)
    {
        // Invalid id.
        return false;
    }

    string addr = string(childid).substr(div+1, string::npos);
    uint64_t register_address = strtol(addr.c_str(), NULL, 16);

    // Grab register description
    pugi::xml_node info = elem.child("h2");
    if(!info)
    {
        // Invalid format.
        return false;
    }
    const char* desc = info.child("a").value();
    string namestr;
    string notestr;

    for (xml_node current = info.child("span"); current; current = current.next_sibling("span"))
    {
        const char* classattr = current.attribute("class").value();
        classattr = classattr ? classattr : "";

        if(classattr == string("res-attrs"))
        {
            // Attributes.
            for (xml_node attribute = current.child("span"); attribute; attribute = attribute.next_sibling("span"))
            {
                classattr = attribute.attribute("class").value();
                classattr = classattr ? classattr : "";
                if(classattr == string("res-symbol"))
                {
                    // Register name
                    const char* name = attribute.value();
                    name = name ? name : "";
                    namestr = name;
                    namestr = trim(namestr);

                    // Remove any leading REG_
                    string stripName = "REG_";
                    if(namestr.find(stripName) == 0)
                    {
                        namestr = namestr.substr(stripName.length(), string::npos); // skip REG_
                    }

                    // REmove any leading component names
                    stripName = component.getName() + "__";
                    if(namestr.find(stripName) == 0)
                    {
                        namestr = namestr.substr(stripName.length(), string::npos); // skip REG_
                    }
                }
            }
        }
    }

    xml_node body = elem.child("div");
    if(body &&
        body.attribute("class") &&
        body.attribute("class").value() == string("res-body"))
    {
        // Body found
        xml_node notes = body.child("div");
        if(notes &&
            notes.attribute("class") &&
            notes.attribute("class").value() == string("res-notes"))
        {
            xml_node p = notes.child("p");
            const char* nt =  p ? p.value() : notes.value();
            notestr = nt ? nt : "";
        }
        else
        {
            // Invalid
            return false;
        }

        cout << "\tID: " << childid << " at " << register_address << endl;
        cout << "\t\tName: " << namestr << endl;
        cout << "\t\tLong Name: " << desc << endl;
        cout << "\t\tDesc: " << notestr << endl;

        if(ismii && string(desc) == "Miscellaneous Control")
        {
            // REgister address has a typo in the xhml file.
            register_address |= 0xFFFFF000;
        }

        if(ismii &&
            string(desc).find("[") == 0) // subcomponent
        {
            // TODO: FIXME
            cout << "UNION... SKIPPING" << endl;
            return true;
        }
        if(ismii &&
            (register_address & 0xFFFFF000) == 0xFFFFF000)
        {
            cout << "PAGED REGISTER: SKIPPING" << endl;
            return true;
        }
        if(ismii &&
            (register_address & 0xFFFFF000) == 0xFFFF0000)
        {
            cout << "OTHER0 PAGED REGISTER: SKIPPING" << endl;
            return true;
            // exit(-1);
        }
        if(ismii &&
            (register_address & 0xFFFFF000) == 0xFFFF1000)
        {
            cout << "OTHER1 PAGED REGISTER: SKIPPING" << endl;
            return true;
            // exit(-1);
        }

        if(string("") == namestr)
        {
            if(string("") == desc)
            {
                cerr << "Unknown name." << endl;
                exit(-1);
            }
            else
            {
                namestr = desc;
            }
        }
        // Create register, if needed
        Register* reg = component.get(namestr);
        if(!reg)
        {
            reg = new Register(namestr);
            component.set(namestr, reg);
        }

        reg->setDescription(notestr);
        reg->setWidth(width); // TODO
        reg->setAddr(register_address);

        // Parse Bitfield
        xml_node bits = body.child("table");
        if(bits &&
            bits.attribute("class") &&
            bits.attribute("class").value() == string("bits"))
        {
            cout << "\t\tHas bitfield." << endl;
            if(!addBitmap(bits, *reg))
            {
                return false;
            }
        }
    }
    else
    {
        // unexpected formatting.
        return false;
    }


    return true;
}

bool XHTMLReader::addBitmap(const pugi::xml_node& elem, Register &reg)
{
    int start;
    int stop;
    for (xml_node current = elem.child("tr"); current; current = current.next_sibling("tr"))
    {
        // The first td is the positsion
        xml_node position = current.child("td");
        const char* bitname = NULL;
        if(position)
        {
            const char* posstr = position.value();
            if(!posstr)
            {
                // Invalid
                return false;
            }

            xml_node body = position.next_sibling("td"); // Second column is the description.
            if(body)
            {
                xml_node nameelem = body.child("div");
                if(nameelem &&
                    nameelem.attribute("class") &&
                    nameelem.attribute("class").value() == string("bitname"))
                {
                    bitname = nameelem.value();
                }
                else
                {
                    cerr << "Unable to locate bit name" << endl;
                    // Invalid
                    return false;
                }

                string pos(posstr);
                size_t hyphen = pos.find("—");
                if(string::npos != hyphen)
                {
                    // has both start and stop bits.
                    string stopstr = pos.substr(0, hyphen);
                    string startstr = pos.substr(hyphen + strlen("—"));

                    start = atoi(startstr.c_str());
                    stop = atoi(stopstr.c_str());
                }
                else
                {
                    start = stop = atoi(posstr);
                }

                if(!bitname)
                {
                    string nm = "unknown_";
                    nm += to_string(start);
                    nm += string("_");
                    nm += to_string(stop);
                    bitname = nm.c_str();
                }

                cout << "\t\t\t" << posstr << " : " << bitname <<  endl;

                // grab data struct
                RegisterBitmap* bitmap = reg.get(bitname);
                if(!bitmap)
                {
                    bitmap = new RegisterBitmap(bitname);
                    reg.set(bitname, bitmap);
                }

                //bitmap->setDescription(current->GetText());
                bitmap->setType(RegisterBitmap::ReadWrite);
                bitmap->setStart(start);
                bitmap->setStop(stop);

                xml_node enums = body.child("table"); // Second column is the description.
                if(enums)
                {
                    addEnumerations(enums, *bitmap);
                }
            }
        }

    }
    return true;
}

bool XHTMLReader::addEnumerations(const pugi::xml_node& elem, RegisterBitmap& bitmap)
{
    int64_t val;
    for (xml_node current = elem.child("tr"); current; current = current.next_sibling("tr"))
    {
        xml_node value = current.child("td");
        if(value)
        {
            const char* valuestr = value.value();
            const char* namestr = NULL;
            if(!valuestr)
            {
                // Invalid
                return false;
            }

            xml_node valuename = value.next_sibling("td"); // Second column is the description/name.
            if(valuename)
            {
                namestr = valuename.child("div").value();
                if(!namestr)
                {
                    // No name for enum.
                    return false;
                }
            }
            else
            {
                return false;
            }
            cout << "\t\t\t\t" << valuestr << " : " << namestr << endl;
            // grab data struct
            Enumeration* bitenum = bitmap.get(namestr);
            if(!bitenum)
            {
                bitenum = new Enumeration(namestr);
                bitmap.set(namestr, bitenum);
            }

            val = atoi(valuestr);
            bitenum->setValue(val);
        }
    }
    return true;
}

bool XHTMLReader::addComponent(const pugi::xml_node& elem, std::string &id)
{
    int width = 32;
    int addressWidth = 8;
    if(id == "MEM" ||   // Memory Map - skip
        id == "DIRENTRY" || // Directory entries in NVM - skip
        id == "NVM" || // NVM layout - has sub-groups, skip for now.
        id == "PORT" // Looks to be an enum
        )
    {
        return true;
    }

    if(id == "REG")
    {
        id = "DEVICE";
    }

    if(id == "MII")
    {
        addressWidth = 16;
        width = 16;
    }

    bool status = true;
    pugi::xml_node descelem = elem.child("h1");
    const char* desc = descelem.value();
    desc = desc ? desc : "";

    cout << "Component: " << id <<  " : " << desc << endl;

    Component* component = mComponents.get(id);
    if(!component)
    {
        component = new Component(id);
        mComponents.set(id, component);
    }

    component->setDescription(desc);
    component->setAddressUnitBits(addressWidth);


    for (xml_node current = elem.child("div"); current; current = current.next_sibling("div"))
    {
        if(!addRegister(current, *component, width))
        {
            status = false;
        }
    }

    return status;
}

