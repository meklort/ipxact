////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/IPXACTWriter.cpp
///
/// @project    ipxact
///
/// @brief      IP-XACT Writer
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

#include <IPXACTWriter.hpp>
#include <main.hpp>
#include <pugixml.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace pugi;
using namespace std;

IPXACTWriter::IPXACTWriter(const char* filename) : Writer(filename)
{

}

IPXACTWriter::~IPXACTWriter()
{

}

void IPXACTWriter::serialize_bitmap_definition(xml_node& elem, RegisterBitmap& bitmap, int regwidth)
{


}

string IPXACTWriter::registerType(RegisterBitmap::Type type) const
{
	switch(type)
	{
		case RegisterBitmap::Reserved:
			return "read-only";
			break;

		case RegisterBitmap::ReadOnly:
			return "read-only";
			break;

		case RegisterBitmap::WriteOnly:
			return "write-only";
			break;

		default:
		case RegisterBitmap::ReadWrite:
			return "read-write";
			break;

		case RegisterBitmap::WriteOnce:
			return "writeOnce";
			break;

		case RegisterBitmap::ReadWriteOnce:
			return "read-writeOnce";
			break;
	}

}

pugi::xml_node IPXACTWriter::insertElement(pugi::xml_node& elem, const string& name)
{
	// xml_node doc = elem.root();
	xml_node node = elem.append_child(name.c_str());

	return node;
}

pugi::xml_node IPXACTWriter::insertElement(pugi::xml_node& elem, const string& name, const string& value)
{
	// xml_node doc = elem.root();
	xml_node newelem = elem.append_child(name.c_str());
	newelem.text().set(value.c_str());

	return newelem;
}

pugi::xml_node IPXACTWriter::insertElement(pugi::xml_node& elem, const string& name, unsigned int value)
{
	ostringstream valstr;
	valstr << "0x" << std::hex << value;

	return insertElement(elem, name, valstr.str());
}

void IPXACTWriter::serialize_bitmap_declaration(xml_node& elem, RegisterBitmap& bitmap, int regwidth)
{
	xml_node field = insertElement(elem, "ipxact:field");
	// xml_node doc = elem.root();


	//field
	insertElement(field, "ipxact:name", bitmap.getName());
	insertElement(field, "ipxact:description", bitmap.getDescription());
	insertElement(field, "ipxact:bitOffset", bitmap.getStop());
	insertElement(field, "ipxact:bitWidth", bitmap.getStart() - bitmap.getStop() + 1);
	insertElement(field, "ipxact:access", registerType(bitmap.getType()));


	if(!bitmap.get().empty())
	{
		xml_node enums = insertElement(field, "ipxact:enumeratedValues");
		field.insert_child_before(pugi::node_comment, enums).set_value(" LINK: enumeratedValue: see 6.11.10, Enumeration values ");

		const std::list<Enumeration*>& bits = bitmap.get();
		std::list<Enumeration*>::const_iterator bits_it;
		for(bits_it = bits.begin(); bits_it != bits.end(); bits_it++)
		{
			Enumeration* bit = *bits_it;
			if(bit)
			{
				xml_node thisenum = insertElement(enums, "ipxact:enumeratedValue");
				insertElement(thisenum, "ipxact:name", bit->getName());
				insertElement(thisenum, "ipxact:value", bit->getValue());

			}
		}
	}

#if 0
<ipxact:field>
	<ipxact:resets>
		<ipxact:reset>
			<ipxact:value>0x0</ipxact:value>
		</ipxact:reset>
		<ipxact:reset resetTypeRef="SOFT">
			<ipxact:value>0xf</ipxact:value>
			<ipxact:mask>0xa</ipxact:mask>
		</ipxact:reset>
	</ipxact:resets>
	<!-- LINK: fieldData: see 6.11.9 Field data group -->
	<ipxact:access>writeOnce</ipxact:access>
	<!-- Document pre-defined values that can be written to this field -->
	<ipxact:enumeratedValues>
		<!-- LINK: enumeratedValue: see 6.11.10, Enumeration values -->
		<ipxact:enumeratedValue>
			<ipxact:name>SetPos0</ipxact:name>
			<ipxact:value>0x1</ipxact:value>
		</ipxact:enumeratedValue>
		<ipxact:enumeratedValue>
			<ipxact:name>SetPos1</ipxact:name>
			<ipxact:value>0x2</ipxact:value>
		</ipxact:enumeratedValue>
	</ipxact:enumeratedValues>
</ipxact:field>

#endif

}

void IPXACTWriter::serialize_register_definition(xml_node& elem, Register& reg)
{


}

void IPXACTWriter::serialize_register_declaration(xml_node& elem, Register& reg)
{
	xml_node addr = insertElement(elem, "ipxact:register");
	elem.insert_child_before(pugi::node_comment, addr).set_value(" LINK: registerDefinitionGroup: see 6.11.3, Register definition group ");

	insertElement(addr, "ipxact:name", reg.getName());
	insertElement(addr, "ipxact:description", reg.getDescription());

	ostringstream hexAddr;
	hexAddr << "0x" << std::hex << reg.getAddr();
	insertElement(addr, "ipxact:addressOffset", hexAddr.str());

	if(!reg.getTypeID().empty())
	{
		insertElement(addr, "ipxact:typeIdentifier", reg.getTypeID());
	}

	if(reg.getDimensions() > 1)
	{
		ostringstream hexDims;
		hexDims << "0x" << std::hex << reg.getDimensions();
		insertElement(addr, "ipxact:dim", hexDims.str());
	}

	insertElement(addr, "ipxact:size", reg.getWidth());
	insertElement(addr, "ipxact:volatile", "true");

	// <ipxact:access>read-writeOnce</ipxact:access>


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
				serialize_bitmap_declaration(addr, *bit, reg.getWidth());
			}
		}
	}
}

void IPXACTWriter::serialize_component_declaration(xml_node& elem, Component& component)
{
	// xml_node& doc = elem.root();
	xml_node map  = insertElement(elem, "ipxact:memoryMap");
	insertElement(map, "ipxact:name", component.getName());
	insertElement(map, "ipxact:description", component.getDescription());


	xml_node addr = insertElement(map, "ipxact:addressBlock");
	elem.insert_child_before(pugi::node_comment, addr).set_value(" LINK: addressBlockDefinitionGroup: see 6.9.3, Address blockdefinition group ");
	insertElement(addr, "ipxact:name", component.getName());
	insertElement(addr, "ipxact:description", component.getDescription());

	ostringstream hexAddr;
	hexAddr << "0x" << std::hex << component.getBase();
	insertElement(addr, "ipxact:baseAddress", hexAddr.str());

	// ipxact:range
	//insertElement(addr, "ipxact:range", ??);

	// ipxact:width
	//insertElement(addr, "ipxact:width", ??);

	if(!component.getTypeID().empty())
	{
		insertElement(addr, "ipxact:typeIdentifier", component.getTypeID());
	}

	// if(component.getRange())
	{
		ostringstream hexRange;
		hexRange << "0x" << std::hex << component.getRange();
		insertElement(addr, "ipxact:range", hexRange.str());
	}

	insertElement(addr, "ipxact:usage", "register");
	insertElement(addr, "ipxact:volatile", "false");


    if(!component.isTypeIDCopy())
	{
		const std::list<Register*>& regs = component.get();
		std::list<Register*>::const_iterator it;
		for(it = regs.begin(); it != regs.end(); it++)
		{
			Register* reg = *it;
			if(reg)
			{
				reg->sort();
				serialize_register_declaration(addr, *reg);
			}
		}
	}

	insertElement(map, "ipxact:addressUnitBits", component.getAddressUnitBits());

}



bool IPXACTWriter::write(Components& components)
{
	xml_document doc;
	xml_node decl = doc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "UTF-8";

	xml_node root = doc.append_child("ipxact:component");

	root.append_attribute("xmlns:xsi") = "http://www.w3.org/2001/XMLSchema-instance";
	root.append_attribute("xmlns:ipxact") = "http://www.accellera.org/XMLSchema/IPXACT/1685-2014";
	root.append_attribute("xsi:schemaLocation") = "http://www.accellera.org/images/XMLSchema/IPXACT/1685-2014/index.xsd";

	insertElement(root, "ipxact:vendor", "meklort");
	insertElement(root, "ipxact:library", (*gOptions)["project"].c_str());
	insertElement(root, "ipxact:name", "Register Definitions");
	insertElement(root, "ipxact:version", "1.0");

	if(!components.get().empty())
	{
		xml_node maps = 	insertElement(root, "ipxact:memoryMaps");

        const std::list<Component*> &componentList = components.get();
        std::list<Component*>::const_iterator it;
        for(it = componentList.begin(); it != componentList.end(); it++)
		{
			Component* component = *it;
			if(component) 
			{
				component->sort();
				serialize_component_declaration(maps, *component);
			}
		}
	}

	doc.save(mFile);

    return true;
}
