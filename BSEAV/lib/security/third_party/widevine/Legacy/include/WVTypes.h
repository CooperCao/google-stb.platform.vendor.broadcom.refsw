/****************************************************************************************************
 * WVTypes.h
 *
 * (c) Copyright 2011-2012 Google, Inc.
 *
 * Widevine API types
 ***************************************************************************************************/

#ifndef __WVTYPES_H__
#define __WVTYPES_H__

#include <string>
#include <vector>
#include <map>
#include <stdint.h>

// Unsigned and signed integer types (32 bits)
typedef uint32_t WVUnsignedInt;
typedef int32_t WVSignedInt;
typedef uint64_t WVUnsignedLong;
typedef int64_t WVSignedLong;
typedef float WVFloat;
typedef bool WVBoolean;

// String type
typedef std::string WVString;

// Array of strings
typedef std::vector<WVString> WVStringArray;
typedef std::vector<uint8_t> WVDataBlob;

// Forward declarations
class WVDictionary;
class WVTypedValueRep;
class WVTypedValueArray;


// Classs to hold a typed value to be stored in a WVTypedValueArray or a WVDictionary
class WVTypedValue
{
 public:
    enum Type {
	Type_Null 		= 0,
	Type_UnsignedInt 	= 1,
	Type_SignedInt 		= 2,
	Type_UnsignedLong 	= 3,
	Type_SignedLong 	= 4,
	Type_Float	 	= 5,
	Type_WVBoolean 		= 6,
	Type_String		= 7,
	Type_Array		= 8,
	Type_Dictionary		= 9,
	Type_DataBlob		= 10
    };

    // Constructs Type_Null value
    WVTypedValue();

    // Copy constructor and assignment operator
    WVTypedValue(WVTypedValue const& original);
    WVTypedValue const& operator=(WVTypedValue const& rhs);

    // Constructors for various typed values
    WVTypedValue(WVUnsignedInt value);
    WVTypedValue(WVSignedInt value);
    WVTypedValue(WVUnsignedLong value);
    WVTypedValue(WVSignedLong value);
    WVTypedValue(WVFloat value);
    WVTypedValue(WVBoolean value);
    WVTypedValue(WVString const& value);
    WVTypedValue(WVTypedValueArray const& value);
    WVTypedValue(WVDictionary const& value);
    WVTypedValue(WVDataBlob const& value);

    ~WVTypedValue();

    // Returns the type of the value held in the object
    Type GetType() const { return(mType); }

    // Accessors to retrieve the value.  Returns false if asked for a value of the wrong type
    WVBoolean GetValue(WVUnsignedInt& value) const;
    WVBoolean GetValue(WVSignedInt& value) const;
    WVBoolean GetValue(WVUnsignedLong& value) const;
    WVBoolean GetValue(WVSignedLong& value) const;
    WVBoolean GetValue(WVFloat& value) const;
    WVBoolean GetValue(WVBoolean& value) const;
    WVBoolean GetValue(WVString const*& value) const;
    WVBoolean GetValue(WVTypedValueArray const*& value) const;
    WVBoolean GetValue(WVDictionary const*& value) const;
    WVBoolean GetValue(WVDataBlob const*& value) const;

    // Accessors to set the value.  Can be used to change both the value and the type
    void SetUnsignedIntValue(WVUnsignedInt value);
    void SetSignedIntValue(WVSignedInt value);
    void SetUnsignedLongValue(WVUnsignedLong value);
    void SetSignedLongValue(WVSignedLong value);
    void SetFloatValue(WVFloat value);
    void SetBooleanValue(WVBoolean value);
    void SetStringValue(WVString const& value);
    void SetArrayValue(WVTypedValueArray const& value);
    void SetDictionaryValue(WVDictionary const& value);
    void SetDataBlobValue(WVDataBlob const& value);

    // Serialize into WVDataBlob
    void Serialize(WVDataBlob& intoBlob) const;

    // Deserialize from WVDataBlob.  Returns bytes used, 0 on failure
    WVUnsignedInt Deserialize(WVDataBlob const& fromBlob, WVUnsignedInt fromIndex = 0);

    // Dump value in human-readable format to toString
    void Dump(WVString const& prefix, WVString& toString) const;

 private:
    void Reset();

    union Representation {
	WVUnsignedInt uUnsignedIntValue;
	WVSignedInt uSignedIntValue;
	WVUnsignedLong uUnsignedLongValue;
	WVSignedLong uSignedLongValue;
	WVFloat uFloatValue;
	WVBoolean uBooleanValue;
	WVTypedValueRep* uOtherValue;
    };
    Type mType;
    Representation mValue;
};


// Class which holds an array of WVTypedValue.  Types of values stored may vary
class WVTypedValueArray
{
 public:
    // Returns the number of entries in the array
    WVUnsignedInt Size() const { return mArray.size(); }
    bool Empty() const { return mArray.empty(); }

    // Disposes of all entries in the array
    void Clear() { mArray.clear(); }

    // Returns a pointer to an element in an array.  NULL if the index is out of range
    WVTypedValue const* operator[](WVUnsignedInt index) const;

    // Iterator type and boundary methods for typed value array
    typedef std::vector<WVTypedValue>::const_iterator Iterator;
    Iterator Begin() const { return(mArray.begin()); }
    Iterator End() const { return(mArray.end()); }

    // Adds a typed value to the end of the array
    void Push(WVTypedValue const& entry) { mArray.push_back(entry); }

    // Accessors for typed values stored in the array.  Returns false if index is out of range,
    // or if trying to retrieve a value of the incorrect type.
    WVBoolean GetValue(WVUnsignedInt index, WVUnsignedInt& value) const;
    WVBoolean GetValue(WVUnsignedInt index, WVSignedInt& value) const;
    WVBoolean GetValue(WVUnsignedInt index, WVUnsignedLong& value) const;
    WVBoolean GetValue(WVUnsignedInt index, WVSignedLong& value) const;
    WVBoolean GetValue(WVUnsignedInt index, WVFloat& value) const;
    WVBoolean GetValue(WVUnsignedInt index, WVBoolean& value) const;
    WVBoolean GetValue(WVUnsignedInt index, WVString const*& value) const;
    WVBoolean GetValue(WVUnsignedInt index, WVTypedValueArray const*& value) const;
    WVBoolean GetValue(WVUnsignedInt index, WVDictionary const*& value) const;
    WVBoolean GetValue(WVUnsignedInt index, WVDataBlob const*& value) const;

    // Serialize into WVDataBlob
    void Serialize(WVDataBlob& intoBlob) const;

    // Deserialize from WVDataBlob.  Returns bytes used, 0 on failure
    WVUnsignedInt Deserialize(WVDataBlob const& fromBlob, WVUnsignedInt fromIndex = 0);

    // Dump values in human-readable format to toString
    void Dump(WVString const& prefix, WVString& toString) const;

 private:
    std::vector<WVTypedValue> mArray;
};


// Object which holds named typed value pairs.  Types of values stored may vary
class WVDictionary
{
 public:
    // Returns the number of entries in the dictionary
    WVUnsignedInt Size() const { return mMap.size(); }
    bool Empty() const { return mMap.empty(); }

    // Disposes of all entries in the dictionary
    void Clear() { mMap.clear(); }

    // Iterator type and bounary methods for dictionary.
    typedef std::map<WVString, WVTypedValue>::const_iterator Iterator;
    Iterator Begin() const { return(mMap.begin()); }
    Iterator End() const { return(mMap.end()); }

    // Store a typed value for the specified key.  Replaces any previous value stored for the key
    void SetEntry(WVString const& key, WVTypedValue const& entry = WVTypedValue()) { mMap[key] = entry; }

    // Remove the typed value for the specified key.  Fails silently if value is not present
    void RemoveEntry(WVString const& key);

    // Copy an entry from another dictionary
    void CopyEntryFrom(WVString const& key, WVDictionary const& fromDict, bool removeIfNotFound = false);

    // Merge two dictionaries
    void Merge(WVDictionary const& fromDict, bool replaceDuplicates);

    // Returns a pointer to a stored value for a specified key.  Returns NULL if value not present
    WVTypedValue const* GetEntry(WVString const& key) const;

    // Accessor methods for typed values stored in the dictionary.  Returns false if the value
    // is not present, or if trying to retrieve a value of the incorrect type
    WVBoolean GetValue(WVString const& key, WVUnsignedInt& value) const;
    WVBoolean GetValue(WVString const& key, WVSignedInt& value) const;
    WVBoolean GetValue(WVString const& key, WVUnsignedLong& value) const;
    WVBoolean GetValue(WVString const& key, WVSignedLong& value) const;
    WVBoolean GetValue(WVString const& key, WVFloat& value) const;
    WVBoolean GetValue(WVString const& key, WVBoolean& value) const;
    WVBoolean GetValue(WVString const& key, WVString const*& value) const;
    WVBoolean GetValue(WVString const& key, WVTypedValueArray const*& value) const;
    WVBoolean GetValue(WVString const& key, WVDictionary const*& value) const;
    WVBoolean GetValue(WVString const& key, WVDataBlob const*& value) const;

    // Convenience accessors.  Return default values if not found or wrong type specified
    WVUnsignedInt GetUnsignedIntValue(WVString const& key, WVUnsignedInt defaultValue = 0) const;
    WVSignedInt GetSignedIntValue(WVString const& key, WVSignedInt defaultValue = 0) const;
    WVUnsignedLong GetUnsignedLongValue(WVString const& key, WVUnsignedLong defaultValue = 0) const;
    WVSignedLong GetSignedLongValue(WVString const& key, WVSignedLong defaultValue = 0) const;
    WVFloat GetFloatValue(WVString const& key, WVFloat defaultValue = 0) const;
    WVBoolean GetBooleanValue(WVString const& key, WVBoolean defaultValue = 0) const;
    WVString GetStringValue(WVString const& key, WVString const& defaultValue = "") const;

    // Serialize into WVDataBlob
    void Serialize(WVDataBlob& intoBlob) const;

    // Deserialize from WVDataBlob.  Returns bytes used, 0 on failure
    WVUnsignedInt Deserialize(WVDataBlob const& fromBlob, WVUnsignedInt fromIndex = 0);

    // Merge dictionary into current one
    WVDictionary const& operator+=(WVDictionary const& rhs);

    // Dumps values in human-readable format to toString
    void Dump(WVString const& prefix, WVString& toString) const;

 private:
    std::map<WVString, WVTypedValue> mMap;
};

#endif // __WVTYPES_H__
