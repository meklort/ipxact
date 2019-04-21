////////////////////////////////////////////////////////////////////////////////
///
/// @file       includes/Register.hpp
///
/// @project    ipxact
///
/// @brief      Generic Container Class.
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

#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <map>
#include <list>
#include <string>


template <class T> class Container {
public:
    Container(const std::string& name) {
        mName = name;
        mDescription = "";
    }

    Container(const std::string& name, const std::string& desc) {
        mName = name;
        mDescription = desc;
    }
    ~Container() {
        clear();
    };

    const std::string& getName() const { return mName; };
    const std::string& getDescription() const { return mDescription; };

    void setDescription(const std::string& desc) { mDescription = desc; };
    void setName(const std::string& name) { mName = name; };


    virtual const std::list<T*>& get() {
        return mList;
    }

    virtual T* get(const std::string& name) {
        return mMap[name];
    }

    virtual void set(const std::string& name, T* element) {
        mMap[name] = element;
        mList.push_back(element);
        sort();
    }

    virtual void setTypeID(const std::string& type, const std::string &copy) {
        mTypeID = type;
        mTypeIDCopy = copy;
    }

    virtual T* getElementWithTypeID(std::string &typeID) {
        return NULL;
    }

    virtual const std::string& getTypeID(void) {
        return mTypeID;
    }

    virtual const std::string& getTypeIDCopy(void) {
        return mTypeIDCopy;
    }

    virtual bool isTypeIDCopy(void) {
        if(mTypeIDCopy.empty())
        {
            return false;
        }
        else
        {
            return mTypeIDCopy != mName;
        }
    }



    virtual void remove(const std::string& name, T* element)
    {
        mList.remove(element);
        mMap[name] = NULL;
    }


    virtual size_t size() {
        return mList.size();
    }


    virtual void clear() {
        for(typename std::list<T*>::const_iterator it = mList.begin();
            it != mList.end(); ++it)
        {
            // delete *it;
        }

        // mList.clear();
        // mMap.clear();
    }


    virtual void sort() = 0;

protected:
    std::string mName;
    std::string mTypeID;
    std::string mTypeIDCopy;
    std::string mDescription;

    std::map<std::string, T*>   mMap;
    std::list<T*>               mList;
};

class Enumeration : public Container<int>
{
public:
    Enumeration(const std::string& name);
    virtual ~Enumeration() { }

    unsigned int getValue() const { return mValue; };
    void setValue(unsigned int value) { mValue = value; };

    virtual void sort();

private:
    unsigned int mValue;
};

class RegisterBitmap : public Container<Enumeration>
{
public:
    enum Type {
        ReadOnly,
        WriteOnly,
        ReadWrite,
        ReadWriteOnce,
        WriteOnce,
        Reserved,
    };

    RegisterBitmap(const std::string& name);
    RegisterBitmap(const std::string& name, const std::string& description,
        int start, int stop, int defval, RegisterBitmap::Type type);
    virtual ~RegisterBitmap() { }

    int getStart() const { return mStartBit; };
    int getStop() const { return mStopBit; };
    RegisterBitmap::Type getType() const { return mType; };

    unsigned int getMask() const;

    void setStart(int start) { mStartBit = start; };
    void setStop(int stop) { mStopBit = stop; };
    void setType(RegisterBitmap::Type type) { mType = type; };
    int getWidth() const { return mStartBit - mStopBit; };

    void setResetValue(unsigned int resetValue);
    unsigned int getResetValue() const { return mResetValue; }
    bool hasResetValue() const { return mHasResetValue; }

    void setReserved(bool reserved);
    bool isReserved() const { return mReserved; }

    void setConstantValue(bool constant);
    bool isConstantValue() const { return mConstantValue; }

    virtual void sort();

private:
    int mStartBit;
    int mStopBit;

    int mDefault;
    RegisterBitmap::Type mType;

    bool mHasResetValue;
    unsigned int mResetValue;

    bool mReserved;
    bool mConstantValue;
};

class Register : public Container<RegisterBitmap>
{
public:
    Register(const std::string& name);
    virtual ~Register();

    void setWidth(int width);
    int getWidth() const { return mWidth; };

    void setAddr(uint64_t addr);
    uint64_t getAddr(void) const { return mAddress; };

    void setDimensions(unsigned int dim);
    unsigned int getDimensions(void) const { return mDimensions; };

    unsigned int getResetValue(void) const;
    unsigned int getWriteMask(void) const;
    unsigned int getMask(void) const;

    bool hasReadOnly() const;
    bool hasWriteOnly() const;
    bool hasWrite() const;

    virtual void sort();
private:
    uint64_t mAddress;
    int mWidth;
    int mDimensions;
};

class Component : public Container<Register>
{
public:
    Component(const std::string& name);
    virtual ~Component();

    virtual void sort();


    uint64_t getBase() const { return mBase; };
    void setBase(uint64_t base) { mBase = base; };

    std::string getModuleName() const { return mModuleName; }
    void setModuleName(const std::string& name) { mModuleName = name; }

    int getRange() const { return mRange; }
    void setRange(int range) { mRange = range; }

    int getAddressUnitBits() const { return mAddressUnitBits; }
    void setAddressUnitBits(int bits) { mAddressUnitBits = bits; }

    Register* getElementWithTypeID(std::string &typeID);

    virtual Register* get(uint64_t address);

    virtual const std::list<Register*>& get() {
        return Container<Register>::get();
    }
    virtual Register* get(const std::string& name) {
        return Container<Register>::get(name);
    }

private:
    uint64_t mBase;
    std::string mModuleName;
    int mRange;
    int mAddressUnitBits;
};

class Components : public Container<Component>
{
public:
    Components(const std::string& name);
    Components();
    virtual ~Components();

    virtual void sort() {
    }

    Component* getElementWithTypeID(std::string &typeID);

private:
};


#endif /* !REGISTER_HPP */
