////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/IPXACTReader.cpp
///
/// @project    ipxact
///
/// @brief      IP-XACT Reader
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

#include <IPXACTReader.hpp>
#include <Number.hpp>

#include <pugixml.hpp>
#include <stdio.h>

#include <string>
#include <sstream>

using namespace pugi;
using namespace std;

IPXACTReader::IPXACTReader(const char* filename, Components& components) : Reader(filename, components)
{
}

IPXACTReader::~IPXACTReader()
{

}

bool IPXACTReader::read()
{
    // cout << "IPXACTReader::read" << endl;
    string tmp;
    string xml;
    while(std::getline(mFile, tmp)) {
        xml += tmp;
    }

    xml_document doc;
    doc.load_string(xml.c_str());
    // cout << "IPXACTReader::parse" << endl;

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
        cout << "ERR";
        return false;
    }

    return true;
}

bool IPXACTReader::parseElement(pugi::xml_node& elem)
{
    bool status = true;
    // search for component block - ipxact:addressBlock
    // const char* text = elem.GetText() ? elem.GetText() : "";
    // cout << "Parsing block " << elem.Name() << " : " << text << endl;

    // TODO: parse ipxact:library for project name

    for (xml_node child = elem.first_child(); child; child = child.next_sibling())
    {
        // do something with each child element
        if(child.name() == string("ipxact:addressBlock"))
        {
            // parse registers
            cout << "**********************" << endl;

            if(!parseComponent(child))
            {
                status = false;
            }
        }
        else if(child.name() == string("ipxact:addressUnitBits"))
        {
            // Skip, already handled in parseComponent
        }
        else
        {
            status = status && parseElement(child);
        }
    }

    return status;
}

bool IPXACTReader::parseComponent(const pugi::xml_node& elem)
{
    bool update = false;
    bool status = true;
    bool noregs = false;
    int addressUnitBits = 8;
    string componentname;

    // Determine addressable unit for this block, if set.
    const xml_node adressable = elem.next_sibling("ipxact:addressUnitBits");
    if(adressable)
    {
        if(adressable.child_value())
        {
            Number bits(adressable.child_value());

            if(bits.isValid())
            {
                addressUnitBits = bits.getValue();
            }
            else
            {
                cerr << "Error: ipxact:addressUnitBits with invalid text: " << adressable.child_value() << endl;
                status = false;
            }
        }
    }

    // First pass: find component name
    for (xml_node current = elem.first_child(); current; current = current.next_sibling())
    {
        if(string(current.name()) == "ipxact:name")
        {
            componentname = current.child_value();
        }
    }

    Component* component = mComponents.get(componentname);
    if(!component)
    {
        component = new Component(componentname);
        mComponents.set(componentname, component);
    }
    else
    {
        // component already exists, don't add new elemetns, just update them.
        update = true;
    }

    component->setAddressUnitBits(addressUnitBits);

    // Second pass.
    if(component)
    {
        // if(!update) cout << "Created component "  << componentname << endl;
        // else        cout << "Updating component " << componentname << endl;


        for (xml_node current = elem.first_child(); current && status; current = current.next_sibling())
        {
            if(string(current.name()) == "ipxact:vendorExtensions")
            {
                xml_node pModuleName = current.child("hdlModuleName");

                if(pModuleName && pModuleName.child_value())
                {
                    component->setModuleName(pModuleName.child_value());
                }
            }

            if(string(current.name()) == "ipxact:description")
            {
                if(current.child_value())
                {
                    if(update)
                    {
                        // cout << "Replacing " << componentname << " description with " << current.child_value() << endl;
                    }
                    component->setDescription(current.child_value());
                }
            }

            if(string(current.name()) == "ipxact:range")
            {
                if(current.child_value())
                {
                    Number range(current.child_value());
                    if(range.isValid())
                    {
                        component->setRange(range.getValue());
                    }
                    else
                    {
                        status = false;
                        cerr << "Error: ipxact:range with invalid text." << endl;
                    }
                }
            }

            if(string(current.name()) == "ipxact:baseAddress")
            {
                if(current.child_value())
                {
                    Number base(current.child_value());
                    if(base.isValid())
                    {
                        if(update)
                        {
                            // cout << "Replacing " << componentname << " addr with 0x" << std::hex << base.getValue() << endl;
                        }

                        component->setBase(base.getValue());
                    }
                    else
                    {
                        status = false;
                        cerr << "Error: ipxact:baseAddress with invalid text." << endl;
                    }
                }
            }

            if(string(current.name()) == "ipxact:typeIdentifier")
            {
                const char *type = current.child_value();
                if(type)
                {
                    string typeID = type;
                    Component* source_element = mComponents.getElementWithTypeID(typeID);
                    if(source_element)
                    {
                        const std::list<Register*> &regList = source_element->get();
                        std::list<Register*>::const_iterator regit;
                        for(regit = regList.begin(); regit != regList.end(); regit++)
                        {
                            Register* const reg = *regit;
                            if(reg)
                            {
                                std::string regname = reg->getName();
                                Register* realreg = source_element->get(regname);
                                // component->set(regname, reg);
                                component->set(regname, realreg);
                                noregs = true;
                            }
                        }
                        // std::map<std::string, Component*>::iterator  it;
                        // for(it = mComponents.begin(); it != mComponents.end(); it++)
                        // *component = *source_element;

                        // This is a copy.
                        component->setTypeID(type, source_element->getName());
                    }
                    else
                    {
                        component->setTypeID(type, component->getName());
                    }
                }
            }


            if(string(current.name()) == "ipxact:register")
            {
                if(noregs)
                {
                    cout << "Unable to redefine registers for already defined component types.\n";
                    exit(-1);
                }
                else
                {
                    if(!parseRegister(current, *component, update))
                    {
                        status = false;
                    }
                }
            }
        }
    }

    return status;
}

RegisterBitmap::Type IPXACTReader::registerType(const string& type) const
{
    if(type == "read-only")
    {
        return RegisterBitmap::ReadOnly;
    }

    if(type == "write-only")
    {
        return RegisterBitmap::WriteOnly;
    }

    if(type == "read-write")
    {
        return RegisterBitmap::ReadWrite;
    }

    if(type == "writeOnce")
    {
        return RegisterBitmap::WriteOnce;
    }

    if(type == "read-writeOnce")
    {
        return RegisterBitmap::ReadWriteOnce;
    }

    return RegisterBitmap::Reserved;
}


bool IPXACTReader::parseRegister(const pugi::xml_node& elem, Component& component, bool update)
{
    bool status = true;
    string regname;
    Number* regaddr = NULL;
    Number* dimensions = NULL;
    bool has_bits = false;

    // First pass: find component name
    for (xml_node current = elem.first_child(); current && status; current = current.next_sibling())
    {
        if(string(current.name()) == "ipxact:name")
        {
            if(current.child_value())
            {
                regname = current.child_value();
            }
            else
            {
                cerr << "Error: ipxact:name with no text." << endl;
                status = false;
            }
        }

        if(string(current.name()) == "ipxact:addressOffset")
        {
            if(current.child_value())
            {
                regaddr = new Number(current.child_value());
            }
        }

        if(string(current.name()) == "ipxact:dim")
        {
            if(current.child_value())
            {
                dimensions = new Number(current.child_value());
            }
        }

        if(string(current.name()) == "ipxact:field")
        {
            has_bits = true;
        }
    }


    // grab data struct
    Register* reg;
    optparse::Values& options = *gOptions;
    if(options.get("merge-addr") && regaddr)
    {
        reg = component.get(regaddr->getValue());
        if(reg)
        {
            reg->setName(regname);
            reg->clear(); // remove all bitfields.
            if(has_bits) update = false; // force adding new bitfields, we are merging based on address.
        }
    }
    else
    {
        reg = component.get(regname);
    }

    if(!reg)
    {
        reg = new Register(regname);
        component.set(regname, reg);

        if(update) cout << "  **Register " << regname << " not found." << endl;
        update = false; // new register, don't try to update old data, there is none.
    }
    else
    {
        //update = true;??
    }

    // Second pass.
    if(reg)
    {
        // if(!update) cout << "  Created register "  << regname << endl;
        // else        cout << "  Updating register " << regname << endl;
        if(dimensions)
        {
            reg->setDimensions(dimensions->getValue());
        }

        for (xml_node current = elem.first_child(); current && status; current = current.next_sibling())
        {
            if(string(current.name()) == "ipxact:description")
            {
                if(current.child_value())
                {
                    if(update)
                    {
                        cout << "Replacing " << regname << " description with " << current.child_value() << endl;
                    }

                    reg->setDescription(current.child_value());
                }
            }

            if(string(current.name()) == "ipxact:size")
            {
                if(current.child_value())
                {
                    Number width(current.child_value());
                    if(width.isValid() && width.getValue())
                    {
                        if(update)
                        {
                            cout << "Replacing " << regname << " width with " << std::dec << width.getValue() << endl;
                        }

                        reg->setWidth(width.getValue());
                    }
                    else
                    {
                        cerr << "Error: ipxact:size with invalid text." << endl;
                        status = false;
                    }
                }
            }

            if(string(current.name()) == "ipxact:field")
            {
                if(!parseRegisterBitmap(current, *reg, update))
                {
                    status = false;
                }
            }
        }

        if(status && regaddr)
        {
            if(regaddr->isValid())
            {
                if(update)
                {
                    cout << "Replacing " << regname << " addr with 0x" << std::hex << regaddr->getValue() << endl;
                }
                reg->setAddr(regaddr->getValue());
            }
            else
            {
                cerr << "Error: invalid register address." << endl;
                status = false;
            }
            delete regaddr;
        }
    }



    return status;
}

bool IPXACTReader::parsseEnumeration(const pugi::xml_node& elem, RegisterBitmap& bitmap, bool update)
{
    // force enums to be added in.
    update = false;

    bool status = true;
    Number* value = NULL;
    string enumname;

    for (xml_node current = elem.first_child(); current; current = current.next_sibling())
    {
        if(string(current.name()) == "ipxact:name")
        {
            enumname = current.child_value();
        }

        if(string(current.name()) == "ipxact:value")
        {
            value = new Number(current.child_value());
        }
    }


    // grab data struct
    Enumeration* bitenum = bitmap.get(enumname);
    if(!bitenum)
    {
        if(update)
        {
            cout << "    **Enumeration " << enumname << " not found, dropping." << endl;
        }
        else
        {
            bitenum = new Enumeration(enumname);
            bitmap.set(enumname, bitenum);
        }
    }
    // Second pass.
    if(bitenum && value && value->isValid())
    {
        // if(!update) cout << "    Created enumeration "  << enumname << " : " << value->getValue() << endl;
        // else        cout << "    Updating enumeration " << enumname << " : " << value->getValue() << endl;
        bitenum->setValue(value->getValue());
    }

    if(value)
    {
        delete value;
    }

    return status;
}

bool IPXACTReader::parseEnumerations(const pugi::xml_node& elem, RegisterBitmap& bitmap, bool update)
{
    bool status = true;

    for (xml_node current = elem.first_child(); current; current = current.next_sibling())
    {
        if(string(current.name()) == "ipxact:enumeratedValue")
        {
            bool parsed = parsseEnumeration(current, bitmap, update);
            status = status && parsed;
        }
    }


    return status;
}

bool IPXACTReader::parseRegisterBitmap(const pugi::xml_node& elem, Register& reg, bool update)
{
    bool status = true;
    int stop = 0;
    int width = 0;
    string fieldname;
    string type;
    Number *pResetValue = 0;

    for (xml_node current = elem.first_child(); current; current = current.next_sibling())
    {
        if(string(current.name()) == "ipxact:name")
        {
            if(current.child_value())
            {
                fieldname = current.child_value();
            }
            else
            {
                cerr << "Error: ipxact:name with no text." << endl;
                status = false;
            }
        }
    }


    // grab data struct
    RegisterBitmap* bitmap = reg.get(fieldname);
    if(!bitmap)
    {
        if(update) 
        {
            cout << "    **Bitfield " << fieldname << " not found, dropping." << endl;
        }
        else
        {
            bitmap = new RegisterBitmap(fieldname);
            reg.set(fieldname, bitmap);
        }
    }
    // Second pass.
    if(bitmap)
    {
        // if(!update) cout << "    Created bitfield "  << fieldname << endl;
        // else        cout << "    Updating bitfield " << fieldname << endl;

        for (xml_node current = elem.first_child(); current; current = current.next_sibling())
        {
            if(string("ipxact:description") == current.name())
            {
                if(current.child_value())
                {
                    bitmap->setDescription(current.child_value());
                }
            }

            if(string("ipxact:resets") == current.name())
            {
                xml_node pReset = current.child("ipxact:reset");

                if(pReset)
                {
                    xml_node pResetValueElement = pReset.child("ipxact:value");

                    if(pResetValueElement && pResetValueElement.child_value())
                    {
                        pResetValue = new Number(pResetValueElement.child_value());
                    }
                }
            }

            // Assume the bit offset and width are correct from the source file.
            if(string("ipxact:bitOffset") == current.name())
            {
                if(current.child_value())
                {
                    Number nstop(current.child_value());
                    if(nstop.isValid())
                    {
                        stop = nstop.getValue();
                    }
                    else
                    {
                        status = false;
                        cerr << "Error: ipxact:bitOffset with invalid text." << endl;
                    }
                }
            }

            if(string("ipxact:vendorExtensions") == current.name())
            {
                xml_node pReserved = current.child("reserved");

                if(pReserved && pReserved.child_value())
                {
                    if(std::string("true") == pReserved.child_value())
                    {
                        bitmap->setReserved(true);
                    }
                    else
                    {
                        bitmap->setReserved(false);
                    }
                }

                xml_node pConstantValue = current.child("constantValue");

                if(pConstantValue && pConstantValue.child_value())
                {
                    if(std::string("true") == pConstantValue.child_value())
                    {
                        bitmap->setConstantValue(true);
                    }
                    else
                    {
                        bitmap->setConstantValue(false);
                    }
                }
            }


            if(string("ipxact:bitWidth") == current.name())
            {
                if(current.child_value())
                {
                    Number nwidth(current.child_value());
                    if(nwidth.isValid())
                    {
                        width = nwidth.getValue();
                    }
                    else
                    {
                        cerr << "Error: ipxact:bitWidth with invalid text." << endl;
                        status = false;
                    }
                }
            }

            if(string("ipxact:access") == current.name())
            {
                if(current.child_value())
                {
                    type = current.child_value();
                }
            }

            if(string("ipxact:enumeratedValues") == current.name())
            {
                parseEnumerations(current, *bitmap, update);
            }
        }

        if(!update)
        {
            bitmap->setStart(stop + width - 1);
            bitmap->setStop(stop);

            if(fieldname.find("reserved") == 0) // reserved at beginning of field name
            {
                bitmap->setType(RegisterBitmap::Reserved);
            }
            else
            {
                bitmap->setType(registerType(type));
            }
        }

        if(status && pResetValue)
        {
            if(pResetValue->getValue() != (pResetValue->getValue() &
                (bitmap->getMask() >> bitmap->getStop())))
            {
                cerr << "Reset value does not fit in field!" << endl;
                status = false;
            }
            else
            {
                bitmap->setResetValue(pResetValue->getValue());
            }

            delete pResetValue;
            pResetValue = 0;
        }
    }


    return status;
}
