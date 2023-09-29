#pragma once

#ifndef DATA
#define DATA

#include <Esp.h>

#include <initializer_list>
#include <vector>

/*
  データ型を表す列挙型
*/
enum class DataType : uint8_t {
  Null,    // データなし
  Bool,    // bool型
  String,  // 文字列型
  Array,   // 配列型
  Int8,    // int8_t型
  Int16,   // int16_t型
  Int32,   // int32_t型
  Int64,   // int64_t型
  UInt8,   // uint8_t型
  UInt16,  // uint16_t型
  UInt32,  // uint32_t型
  UInt64   // uint64_t型
};

static const constexpr uint8_t TYPE_NULL = 0;
static const constexpr uint8_t TYPE_TRUE = 1;
static const constexpr uint8_t TYPE_FALSE = 2;
static const constexpr uint8_t TYPE_STRING = 3;
static const constexpr uint8_t TYPE_ARRAY = 4;
static const constexpr uint8_t TYPE_INT8 = 5;
static const constexpr uint8_t TYPE_INT16 = 6;
static const constexpr uint8_t TYPE_INT32 = 7;
static const constexpr uint8_t TYPE_INT64 = 8;
static const constexpr uint8_t TYPE_UINT8 = 9;
static const constexpr uint8_t TYPE_UINT16 = 10;
static const constexpr uint8_t TYPE_UINT32 = 11;
static const constexpr uint8_t TYPE_UINT64 = 12;

class DataArray;

/*
  データ型と値をもつクラスです。
*/
class Data {
private:
  DataType _type;
  union {
    std::nullptr_t _null;
    bool _bool;
    String* _str;
    std::vector<Data>* _array;
    int8_t _int8;
    int16_t _int16;
    int32_t _int32;
    int64_t _int64;

    uint8_t _uint8;
    uint16_t _uint16;
    uint32_t _uint32;
    uint64_t _uint64;
  } _data;

  size_t _serialize(uint8_t* /* buf */, size_t& /* off */, const size_t /* size */) const;
  static size_t _deserialize(const uint8_t* /* buf */, size_t& /* off */, const size_t /* size */, Data* const /* data_p */);
public:
  /*
    Nullを表すデータを構築します。
  */
  constexpr Data(std::nullptr_t n = nullptr) noexcept
    : _type(DataType::Null), _data{ ._null = n } {}

  /*
    文字列型の値をもつデータを構築します。
  */
  Data(const String&);

  /*
    文字列型の値をもつデータを構築します。
  */
  Data(String&&);

  /*
    ポインタがNULLでない場合は文字列型の値をもつデータを構築します。
    ポインタがNULLの場合はNullを表すデータを構築します。
  */
  Data(const char*);

  /*
    配列型の値を持つデータを構築します。
  */
  Data(const std::vector<Data>&);

  /*
    配列型の値を持つデータを構築します。
  */
  Data(std::vector<Data>&&);

  /*
    配列型の値を持つデータを構築します。
  */
  Data(std::initializer_list<Data>);

  /*
    bool型の値を持つデータを構築します。
  */
  constexpr Data(bool b) noexcept
    : _type(DataType::Bool), _data{ ._bool = b } {}

  /*
    int8_t型の値を持つデータを構築します。
  */
  constexpr Data(int8_t i) noexcept
    : _type(DataType::Int8), _data{ ._int8 = i } {}

  /*
    int16_t型の値を持つデータを構築します。
  */
  constexpr Data(int16_t i) noexcept
    : _type(DataType::Int16), _data{ ._int16 = i } {}

  /*
    int32_t型の値を持つデータを構築します。
  */
  constexpr Data(int32_t i) noexcept
    : _type(DataType::Int32), _data{ ._int32 = i } {}

  /*
    int64_t型の値を持つデータを構築します。
  */
  constexpr Data(int64_t i) noexcept
    : _type(DataType::Int64), _data{ ._int64 = i } {}

  /*
    uint8_t型の値を持つデータを構築します。
  */
  constexpr Data(uint8_t i) noexcept
    : _type(DataType::UInt8), _data{ ._uint8 = i } {}

  /*
    uint16_t型の値を持つデータを構築します。
  */
  constexpr Data(uint16_t i) noexcept
    : _type(DataType::UInt16), _data{ ._uint16 = i } {}

  /*
    uint32_t型の値を持つデータを構築します。
  */
  constexpr Data(uint32_t i) noexcept
    : _type(DataType::UInt32), _data{ ._uint32 = i } {}

  /*
    uint64_t型の値を持つデータを構築します。
  */
  constexpr Data(uint64_t i) noexcept
    : _type(DataType::UInt64), _data{ ._uint64 = i } {}

  /*
    コピーコンストラクタ
    データを複製します。
  */
  Data(const Data&);

  /*
    ムーブコンストラクタ
    データを移動します。
  */
  Data(Data&&);

  /*
    デストラクタ
    メモリを解放します。
  */
  ~Data();

  /*
    データをコピー代入します。
  */
  Data& operator=(const Data&);

  /*
    データをムーブ代入します。
  */
  Data& operator=(Data&&);

  /*
    データをシリアライズします。
  */
  size_t serialize(uint8_t* buf, size_t off, size_t size) const {
    return _serialize(buf, off, size);
  }

  /*
    データをシリアライズします。
  */
  size_t serialize(uint8_t* buf, size_t size) const {
    return serialize(buf, 0, size);
  }

  /*
    データをデシリアライズします。
  */
  static bool deserialize(const uint8_t* buf, const size_t size, Data* const data_p) {
    size_t off = 0;
    return _deserialize(buf, off, size, data_p) == size;
  }

  /*
    データ型と値が共に等しければtrue、そうでなければfalseを返します。
    データ型が異なる場合、値が同じでもfalseを返します。
  */
  bool operator==(const Data&) const;

  /*
    データ型と値のどちらかが異なればtrue、そうでなければfalseを返します。
    データ型が異なる場合、値が同じでもtrueを返します。
  */
  bool operator!=(const Data& other) const {
    return !operator==(other);
  }

  /*
    データ型を取得します。
  */
  DataType type() const noexcept {
    return _type;
  }

  /*
    String型の値に変換します。
    データがString型でない場合は空文字列を返します。
  */
  operator String() const;

  /*
    配列型の値に変換します。
    データが配列型ではない場合は空の配列を返します。
  */
  operator std::vector<Data>() const;

  /*
    bool型の値に変換します。
    データがbool型ではない場合はfalseを返します。
  */
  operator bool() const noexcept;

  /*
    int8_t型の値に変換します。
    データがint8_t型ではない場合は-1を返します。
  */
  operator int8_t() const noexcept;

  /*
    int16_t型の値に変換します。
    データがint16_t型ではない場合は-1を返します。
  */
  operator int16_t() const noexcept;

  /*
    int32_t型の値に変換します。
    データがin32_t型ではない場合は-1を返します。
  */
  operator int32_t() const noexcept;

  /*
    int64_t型の値に変換します。
    データがint64_t型ではない場合は-1を返します。
  */
  operator int64_t() const noexcept;

  /*
    uint8_t型の値に変換します。
    データがuint8_t型でない場合は255を返します。
  */
  operator uint8_t() const noexcept;

  /*
    uint16_t型の値に変換します。
    データがuint16_t型でない場合は65535を返します。
  */
  operator uint16_t() const noexcept;

  /*
    uint32_t型の値に変換します。
    データがuint32_t型でない場合は4294967295を返します。
  */
  operator uint32_t() const noexcept;

  /*
    uint64_t型の値に変換します。
    データがuint64_t型でない場合は18446744073709551615を返します。
  */
  operator uint64_t() const noexcept;
};

#endif
