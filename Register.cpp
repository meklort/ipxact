////////////////////////////////////////////////////////////////////////////////
///
/// @file       source/Register.cpp
///
/// @project    ipxact
///
/// @brief      Various register/component contain routines.
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

#include <Register.hpp>

using namespace std;



Register::Register(const string& name) : Container<RegisterBitmap>(name)
{
    mName = name;
    mWidth = 0;
    mAddress = 0;
    mDescription = "";
    mDimensions = 1;
}

Register::~Register()
{

}

void Register::setWidth(int width)
{
    mWidth = width;
}

void Register::setAddr(uint64_t addr)
{
    mAddress = addr;
}

void Register::setDimensions(unsigned int dim)
{
    mDimensions = dim;
}

unsigned int Register::getResetValue() const
{
    unsigned int resetValue = 0;

    for(std::list<RegisterBitmap*>::const_iterator it = mList.begin();
        it != mList.end(); ++it)
    {
        RegisterBitmap *pField = *it;

        if(pField->hasResetValue())
        {
            unsigned int mask = pField->getMask();
            int stop = pField->getStop();

            resetValue &= ~mask;
            resetValue |= (pField->getResetValue() << stop) & mask;
        }
    }

    return resetValue;
}

unsigned int Register::getWriteMask() const
{
    unsigned int mask = 0;

    for(std::list<RegisterBitmap*>::const_iterator it = mList.begin();
        it != mList.end(); ++it)
    {
        RegisterBitmap *pField = *it;

        if(RegisterBitmap::ReadOnly != pField->getType() &&
            !pField->isReserved())
        {
            mask |= pField->getMask();
        }
    }

    return mask;
}

unsigned int Register::getMask() const
{
    unsigned int mask = 0;

    for(std::list<RegisterBitmap*>::const_iterator it = mList.begin();
        it != mList.end(); ++it)
    {
        RegisterBitmap *pField = *it;

        if(!pField->isReserved())
        {
            mask |= pField->getMask();
        }
    }

    return mask;
}

bool Register::hasReadOnly() const
{
    bool isReadOnly = false;

    for(std::list<RegisterBitmap*>::const_iterator it = mList.begin();
        it != mList.end(); ++it)
    {
        RegisterBitmap *pField = *it;

        if(RegisterBitmap::ReadOnly == pField->getType() &&
            !pField->isReserved())
        {
            isReadOnly = true;
        }
    }

    return isReadOnly;
}

bool Register::hasWriteOnly() const
{
    bool isWriteOnly = false;

    for(std::list<RegisterBitmap*>::const_iterator it = mList.begin();
        it != mList.end(); ++it)
    {
        RegisterBitmap *pField = *it;

        if(RegisterBitmap::WriteOnly == pField->getType() &&
            !pField->isReserved())
        {
            isWriteOnly = true;
        }
    }

    return isWriteOnly;
}

bool Register::hasWrite() const
{
    bool isWrite = false;

    for(std::list<RegisterBitmap*>::const_iterator it = mList.begin();
        it != mList.end(); ++it)
    {
        RegisterBitmap *pField = *it;

        if(RegisterBitmap::ReadOnly != pField->getType() &&
            !pField->isReserved())
        {
            isWrite = true;
        }
    }

    return isWrite;
}

bool compare_bitmaps(const RegisterBitmap* first, const RegisterBitmap* second)
{
    if(!first || !second) return false;
    return (first->getStart() < second->getStart());
}

void Register::sort()
{
    mList.sort(compare_bitmaps);
}


//////

Component::Component(const std::string& name) : Container<Register>(name)
{
    mName = name;
    mDescription = "";
    mRange = 0;
    mAddressUnitBits = 8;
}
Component::~Component()
{

}

bool compare_regs(const Register* first, const Register* second)
{
    if(!first || !second) return false;
    return (first->getAddr() < second->getAddr());
}

void Component::sort()
{
    mList.sort(compare_regs);
}


Register* Component::get(uint64_t address)
{
    const std::list<Register*>& regs = Container<Register>::get();
    std::list<Register*>::const_iterator it;

    for(it = regs.begin(); it != regs.end(); it++)
    {
        Register* reg = *it;
        if(reg && reg->getAddr() == address) return reg;
    }
    return NULL;
}

Register* Component::getElementWithTypeID(std::string &typeID)
{
    Register* element = NULL;
    if(!typeID.empty())
    {
        // See if any existing components exist with the given type, and if so, copy it.
        auto it = mMap.begin();
        for(; it != mMap.end(); it++)
        {
            element = it->second;
            if(element)
            {
                if(element->getTypeID() == typeID)
                {
                    return element;
                }
            }
        }
    }
    return NULL;
}



//////

RegisterBitmap::RegisterBitmap(const std::string& name) :
    Container<Enumeration>(name)
{
    mStartBit = 0;
    mStopBit = 0;
    mDefault = 0;
    mType = RegisterBitmap::ReadWrite;
    mHasResetValue = false;
    mResetValue = 0;
    mReserved = false;
    mConstantValue = false;
}

RegisterBitmap::RegisterBitmap(const std::string& name,
    const std::string& description, int start, int stop, int defval,
    RegisterBitmap::Type type) : Container<Enumeration>(name, description)
{
    mStartBit = start;
    mStopBit = stop;
    mDefault = defval;
    mType = type;
    mHasResetValue = false;
    mResetValue = 0;
    mReserved = false;
    mConstantValue = false;
}

bool compare_enums(const Enumeration* first, const Enumeration* second)
{
    if(!first || !second) return false;
    return (first->getValue() < second->getValue());
}

void RegisterBitmap::sort()
{
    mList.sort(compare_enums);
}

void RegisterBitmap::setResetValue(unsigned int resetValue)
{
    mHasResetValue = true;
    mResetValue = resetValue;
}

void RegisterBitmap::setReserved(bool reserved)
{
    mReserved = reserved;
}

void RegisterBitmap::setConstantValue(bool constant)
{
    mConstantValue = constant;
}

unsigned int RegisterBitmap::getMask() const
{
    unsigned int mask = 0;

    for(int i = getStop(); i <= getStart(); ++i)
    {
        mask |= 1 << i;
    }

    return mask;
}

//////

Enumeration::Enumeration(const std::string& name) : Container<int>(name),
    mValue(0)
{
    mName = name;
    mDescription = "";
}

void Enumeration::sort()
{
}


//////

Components::Components() : Container<Component>("all")
{
    mName = "";
    mDescription = "";
}

Components::~Components()
{

}

Component* Components::getElementWithTypeID(std::string &typeID)
{
    Component* element = NULL;
    if(!typeID.empty())
    {
        // See if any existing components exist with the given type, and if so, copy it.
        auto it = mMap.begin();
        for(; it != mMap.end(); it++)
        {
            element = it->second;
            if(element)
            {
                if(element->getTypeID() == typeID)
                {
                    return element;
                }
            }
        }
    }
    return NULL;
}

