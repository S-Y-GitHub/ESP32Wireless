#include "Data.hpp"

Data::Data(const String& str)
  : _type(DataType::String), _data{ ._str = new String(str) } {}

Data::Data(String&& str)
  : _type(DataType::String), _data{ ._str = new String(str) } {}

Data::Data(const char* str)
  : _type(str == nullptr ? DataType::Null : DataType::String), _data{ ._str = (str == nullptr ? nullptr : new String(str)) } {}

Data::Data(const std::vector<Data>& array)
  : _type(DataType::Array), _data{ ._array = new std::vector<Data>(array) } {}

Data::Data(std::vector<Data>&& array)
  : _type(DataType::Array), _data{ ._array = new std::vector<Data>(array) } {}

Data::Data(const Data& other)
  : _type(other._type) {
  if (other._type == DataType::String)
    _data._str = new String(*other._data._str);
  else if (other._type == DataType::Array)
    _data._array = new std::vector<Data>(*other._data._array);
  else
    _data = other._data;
}

Data::Data(Data&& other)
  : _type(other._type) {
  if (other._type == DataType::String)
    _data._str = new String(std::move(*other._data._str));
  else if (other._type == DataType::Array)
    _data._array = new std::vector<Data>(std::move(*other._data._array));
  else
    _data = other._data;
}

Data::~Data() {
  if (_type == DataType::String)      // 現在のデータが文字列型のとき
    delete _data._str;                // 文字列のメモリを解放
  else if (_type == DataType::Array)  // 現在のデータが配列のとき
    delete _data._array;              // 配列のメモリを解放
}

Data& Data::operator=(const Data& other) {
  if (_type == DataType::String)                                // 現在のデータが文字列型のとき
    delete _data._str;                                          // 文字列のメモリを解放
  else if (_type == DataType::Array)                            // 現在のデータが配列のとき
    delete _data._array;                                        // 配列のメモリを解放
  _type = other._type;                                          // データ型を変更
  if (other._type == DataType::String)                          // 新しいデータが文字列型の場合
    _data._str = new String(*other._data._str);                 // メモリを確保してコピー
  else if (other._type == DataType::Array)                      // 新しいデータが配列型の場合
    _data._array = new std::vector<Data>(*other._data._array);  // メモリを確保してコピー
  else                                                          // 新しいデータがそれ以外の型の場合
    _data = other._data;                                        // 値をコピー
  return *this;                                                 // このオブジェクトの参照を返す
}

Data& Data::operator=(Data&& other) {
  if (_type == DataType::String)                                           // 現在のデータが文字列型のとき
    delete _data._str;                                                     // 文字列のメモリを解放
  else if (_type == DataType::Array)                                       // 現在のデータが配列のとき
    delete _data._array;                                                   // 配列のメモリを解放
  _type = other._type;                                                     // データ型を変更
  if (other._type == DataType::String)                                     // 新しいデータが文字列型の場合
    _data._str = new String(std::move(*other._data._str));                 // メモリを確保してコピー
  else if (other._type == DataType::Array)                                 // 新しいデータが配列型の場合
    _data._array = new std::vector<Data>(std::move(*other._data._array));  // メモリを確保してコピー
  else                                                                     // 新しいデータがそれ以外の型の場合
    _data = other._data;                                                   // 値をコピー
  return *this;                                                            // このオブジェクトの参照を返す
}

bool Data::operator==(const Data& other) const {
  bool b;
  if (other._type == _type)
    switch (_type) {
      case DataType::Null:  // Null
        b = true;
        break;
      case DataType::Bool:
        b = other._data._bool == _data._bool;
        break;
      case DataType::String:
        b = *other._data._str == *_data._str;
        break;
      case DataType::Array:
        b = *other._data._array == *_data._array;
        break;
      case DataType::Int8:
        b = other._data._int8 == _data._int8;
        break;
      case DataType::Int16:
        b = other._data._int16 == _data._int16;
        break;
      case DataType::Int32:
        b = other._data._int32 == _data._int32;
        break;
      case DataType::Int64:
        b = other._data._int64 == _data._int64;
        break;
      case DataType::UInt8:
        b = other._data._uint8 == _data._uint8;
        break;
      case DataType::UInt16:
        b = other._data._uint16 == _data._uint16;
        break;
      case DataType::UInt32:
        b = other._data._uint32 == _data._uint32;
        break;
      case DataType::UInt64:
        b = other._data._uint64 == _data._uint64;
        break;
      default:
        b = false;
        break;
    }
  else
    b = false;
  return b;
}

template<class IntType>
static void serialize_int(uint8_t* buf, size_t& off, IntType val) {
  for (size_t i = 0; i < sizeof(IntType); ++i)
    buf[off++] = (val >> (i << 3)) & 0xFF;
}

size_t Data::_serialize(uint8_t* buf, size_t& off, size_t size) const {
  switch (_type) {
    case DataType::Null:
      if (off + 1 > size) return 0;
      buf[off++] = TYPE_NULL;
      break;
    case DataType::Bool:
      if (off + 1 > size) return 0;
      buf[off++] = _data._bool ? TYPE_TRUE : TYPE_FALSE;
      break;
    case DataType::String:
      {
        const String& str = *_data._str;
        uint16_t len = str.length();
        if (off + 1 + 4 + len > size) return 0;
        buf[off++] = TYPE_STRING;
        serialize_int(buf, off, len);
        for (uint16_t i = 0; i < len; ++i)
          buf[off++] = str[i];
      }
      break;
    case DataType::Array:
      {
        const std::vector<Data>& array = *_data._array;
        uint16_t len = array.size();
        if (off + 1 + 4 > size)
          return 0;
        buf[off++] = TYPE_ARRAY;
        serialize_int(buf, off, len);
        for (uint16_t i = 0; i < len; ++i)
          if (array[i]._serialize(buf, off, size) == 0)
            return 0;
      }
      break;
    case DataType::Int8:
      if (off + 1 + 1 > size) return 0;
      buf[off++] = TYPE_INT8;
      serialize_int(buf, off, _data._int8);
      break;
    case DataType::Int16:
      if (off + 1 + 2 > size) return 0;
      buf[off++] = TYPE_INT16;
      serialize_int(buf, off, _data._int16);
      break;
    case DataType::Int32:
      if (off + 1 + 4 > size) return 0;
      buf[off++] = TYPE_INT32;
      serialize_int(buf, off, _data._int32);
      break;
    case DataType::Int64:
      if (off + 1 + 8 > size) return 0;
      buf[off++] = TYPE_INT64;
      serialize_int(buf, off, _data._int64);
      break;
    case DataType::UInt8:
      if (off + 1 + 1 > size) return 0;
      buf[off++] = TYPE_UINT8;
      serialize_int(buf, off, _data._uint8);
      break;
    case DataType::UInt16:
      if (off + 1 + 2 > size) return 0;
      buf[off++] = TYPE_UINT16;
      serialize_int(buf, off, _data._uint16);
      break;
    case DataType::UInt32:
      if (off + 1 + 4 > size) return 0;
      buf[off++] = TYPE_UINT32;
      serialize_int(buf, off, _data._uint32);
      break;
    case DataType::UInt64:
      if (off + 1 + 8 > size) return 0;
      buf[off++] = TYPE_UINT64;
      serialize_int(buf, off, _data._uint64);
      break;
  }
  return off;
}

template<class IntType>
static IntType deserialize_int(uint8_t* buf, size_t& off) {
  IntType val = 0;
  for (size_t i = 0; i < sizeof(IntType); ++i)
    val |= static_cast<IntType>(buf[off++]) << (i << 3);
  return val;
}

size_t Data::_deserialize(uint8_t* buf, size_t& off, const size_t size, Data* const data_p) {
  if (off >= size) return false;
  switch (buf[off++]) {
    case TYPE_NULL:
      *data_p = nullptr;
      break;
    case TYPE_TRUE:
      *data_p = true;
      break;
    case TYPE_FALSE:
      *data_p = false;
      break;
    case TYPE_STRING:
      if (off + 4 > size) return 0;
      {
        uint16_t len = deserialize_int<uint16_t>(buf, off);
        String str;
        if (!str.reserve(len)) return false;
        for (uint16_t i = 0; i < len; ++i)
          str.concat(static_cast<char>(buf[off++]));
        *data_p = str;
      }
      break;
    case TYPE_ARRAY:
      if (off + 4 > size) return 0;
      {
        uint16_t len = deserialize_int<uint16_t>(buf, off);
        std::vector<Data> array(len);
        for (uint16_t i = 0; i < len; ++i)
          if (_deserialize(buf, off, size, &array[i]) == 0) return 0;
        *data_p = array;
      }
      break;
    case TYPE_INT8:
      if (off + 1 > size) return 0;
      *data_p = deserialize_int<int8_t>(buf, off);
      break;
    case TYPE_INT16:
      if (off + 2 > size) return 0;
      *data_p = deserialize_int<int16_t>(buf, off);
      break;
    case TYPE_INT32:
      if (off + 4 > size) return 0;
      *data_p = deserialize_int<int32_t>(buf, off);
      break;
    case TYPE_INT64:
      if (off + 8 > size) return 0;
      *data_p = deserialize_int<int64_t>(buf, off);
      break;
    case TYPE_UINT8:
      if (off + 1 > size) return 0;
      *data_p = deserialize_int<uint8_t>(buf, off);
      break;
    case TYPE_UINT16:
      if (off + 2 > size) return 0;
      *data_p = deserialize_int<uint16_t>(buf, off);
      break;
    case TYPE_UINT32:
      if (off + 4 > size) return 0;
      *data_p = deserialize_int<uint32_t>(buf, off);
      break;
    case TYPE_UINT64:
      if (off + 8 > size) return 0;
      *data_p = deserialize_int<uint64_t>(buf, off);
      break;
    default:
      return 0;
  }
  return off;
}

Data::operator bool() const noexcept {
  return _type == DataType::Bool ? _data._bool : false;
}

Data::operator String() const {
  return _type == DataType::String ? *_data._str : String();
}

Data::operator std::vector<Data>() const {
  return _type == DataType::Array ? *_data._array : std::vector<Data>();
}

Data::operator int8_t() const noexcept {
  return _type == DataType::Int8 ? _data._int8 : static_cast<int8_t>(-1);
}

Data::operator int16_t() const noexcept {
  return _type == DataType::Int16 ? _data._int16 : static_cast<int16_t>(-1);
}

Data::operator int32_t() const noexcept {
  return _type == DataType::Int32 ? _data._int32 : static_cast<int32_t>(-1);
}

Data::operator int64_t() const noexcept {
  return _type == DataType::Int64 ? _data._int64 : static_cast<int64_t>(-1);
}

Data::operator uint8_t() const noexcept {
  return _type == DataType::UInt8 ? _data._uint8 : static_cast<uint8_t>(0xFF);
}

Data::operator uint16_t() const noexcept {
  return _type == DataType::UInt16 ? _data._uint16 : static_cast<uint16_t>(0xFFFF);
}

Data::operator uint32_t() const noexcept {
  return _type == DataType::UInt32 ? _data._uint32 : static_cast<uint32_t>(0xFFFFFFFF);
}

Data::operator uint64_t() const noexcept {
  return _type == DataType::UInt64 ? _data._uint64 : static_cast<uint64_t>(0xFFFFFFFFFFFFFFFF);
}