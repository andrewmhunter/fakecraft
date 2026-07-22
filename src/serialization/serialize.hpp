#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include "util/types.hpp"
#include <concepts>
#include <cstddef>
#include <filesystem>
#include <format>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <engine/logger.hpp>
#include <istream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace ser {

enum class DataTag : u8 {
    nil,

    //dynamic,

    // tag value
    boolean,
    i8,
    u8,
    i16,
    u16,
    i32,
    u32,
    i64,
    u64,

    f32,
    f64,

    vec3,
    ivec3,

    // tag size character...
    string,
    // tag element_type (can be dynamic) count element...
    list,
    // tag count [key_size character... element_tag element]...
    object, 
};

struct Error : std::runtime_error {
    Error(const std::string& what);
};

struct ParseError : Error {
    ParseError(const std::string& what);
};

struct WriteError : Error {
    WriteError(const std::string& what);
};

struct DecodeError : Error {
    DecodeError(const std::string& what);
};


class Object;
class List;
class Dynamic;

class Reader {
private:
    std::istream& stream;

public:
    explicit Reader(std::istream& stream);

    u8 readByte();
    DataTag readTag();

    template<std::integral T>
    T readInt() {
        T value = 0;
        for (std::size_t i = 0; i < sizeof(value); ++i) {
            value |= readByte() << (i * 8);
        }
        return value;
    }

    float readF32();
    double readF64();

    template<std::unsigned_integral LengthT>
    std::string readString() {
        LengthT size = readInt<LengthT>();
        std::string string{};
        for (u32 i = 0; i < size; ++i) {
            string.push_back(readByte());
        }
        return string;
    }
};

class Writer {
private:
    std::ostream& stream;

public:
    explicit Writer(std::ostream& stream);

    void writeByte(u8 byte);
    void writeTag(DataTag tag);

    void writeInt(std::integral auto value) {
        for (std::size_t i = 0; i < sizeof(value); ++i) {
            writeByte(value & 0xff);
            value >>= 8;
        }
    }

    void writeF32(float value);
    void writeF64(double value);

    template<std::unsigned_integral LengthT>
    void writeString(std::string_view value) {
        Logger::assertion(std::numeric_limits<LengthT>::max() >= value.size(), std::format("String too long"));
        writeInt<LengthT>(value.size());
        for (char ch : value) {
            writeByte(ch);
        }
    }
};

enum class Operation {
    serialize,
    deserialize,
};




template<typename>
struct Serializer {
    Serializer() = delete;
    Serializer(const Serializer&) = delete;
    Serializer& operator=(const Serializer&) = delete;
};

template<>
struct Serializer<std::monostate> {
    using Type = std::monostate;
    static constexpr DataTag tag = DataTag::nil;
    static constexpr std::string_view tagName = "nil";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<bool> {
    using Type = bool;
    static constexpr DataTag tag = DataTag::boolean;
    static constexpr std::string_view tagName = "boolean";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<i8> {
    using Type = i8;
    static constexpr DataTag tag = DataTag::boolean;
    static constexpr std::string_view tagName = "i8";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<u8> {
    using Type = u8;
    static constexpr DataTag tag = DataTag::u8;
    static constexpr std::string_view tagName = "u8";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<i16> {
    using Type = i16;
    static constexpr DataTag tag = DataTag::i16;
    static constexpr std::string_view tagName = "i16";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<u16> {
    using Type = u16;
    static constexpr DataTag tag = DataTag::u16;
    static constexpr std::string_view tagName = "i16";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<i32> {
    using Type = i32;
    static constexpr DataTag tag = DataTag::i32;
    static constexpr std::string_view tagName = "i32";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<u32> {
    using Type = u32;
    static constexpr DataTag tag = DataTag::u32;
    static constexpr std::string_view tagName = "u32";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<i64> {
    using Type = i64;
    static constexpr DataTag tag = DataTag::i64;
    static constexpr std::string_view tagName = "i64";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<u64> {
    using Type = u64;
    static constexpr DataTag tag = DataTag::u64;
    static constexpr std::string_view tagName = "u64";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<float> {
    using Type = float;
    static constexpr DataTag tag = DataTag::f32;
    static constexpr std::string_view tagName = "f32";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<double> {
    using Type = double;
    static constexpr DataTag tag = DataTag::f64;
    static constexpr std::string_view tagName = "f64";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<glm::vec3> {
    using Type = glm::vec3;
    static constexpr DataTag tag = DataTag::vec3;
    static constexpr std::string_view tagName = "vec3";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<glm::ivec3> {
    using Type = glm::ivec3;
    static constexpr DataTag tag = DataTag::ivec3;
    static constexpr std::string_view tagName = "ivec3";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<std::string> {
    using Type = std::string;
    using LengthType = u32;
    static constexpr DataTag tag = DataTag::string;
    static constexpr std::string_view tagName = "string";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<Object> {
    using Type = Object;
    static constexpr DataTag tag = DataTag::object;
    static constexpr std::string_view tagName = "object";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};

template<>
struct Serializer<List> {
    using Type = List;
    static constexpr DataTag tag = DataTag::list;
    static constexpr std::string_view tagName = "list";
    static Type read(Reader& reader);
    static void write(Writer& writer, const Type& value);
};




class Object {
private:
    std::map<std::string, Dynamic> fields;

    using KeyLengthType = u8;

public:
    Operation operation;

    explicit Object(Reader& reader);
    explicit Object(Operation operation = Operation::serialize);

    bool hasField(const std::string& key) const;

    template<typename T>
    const T& getField(const std::string& key) const;

    template<typename T>
    T& getField(const std::string& key);

    template<typename T>
    T getField(const std::string& key, T defaultValue) const;

    template<typename T>
    void setField(const std::string& key, const T& value);

    template<typename T>
    void field(const std::string& key, T& value);

    template<typename T>
    T& field(const std::string& key);

    void write(Writer& writer) const;
};


template<typename T>
struct TypeContainer {
    using Type = T;
};

template<typename... Ts>
class VaradicVariants {
public:
    using Lists = std::variant<std::vector<Ts>...>;
    using Variant = std::variant<Ts...>;
    using TypeContainers = std::variant<TypeContainer<Ts>...>;
};

class List;

using Variants = VaradicVariants<
    std::monostate,

    bool,
    i8,
    u8,
    i16,
    u16,
    i32,
    u32,
    i64,
    u64,

    float,
    double,

    glm::vec3,
    glm::ivec3,

    std::string,

    Object,
    List
>;

class List {
private:
    Variants::Lists elements;

    template<typename T>
    DecodeError wrongDataType() {
        std::string_view rightTag = std::visit([](auto&& contents) {
            return Serializer<typename std::remove_cvref_t<decltype(contents)>::value_type>::tagName;
        }, elements);
        std::string_view attemptedTag = Serializer<T>::tagName;
        return DecodeError{
            std::format("Tried to access list of type '{}' as '{}'", rightTag, attemptedTag)
        };
    }

public:
    Operation operation;
    DataTag tag;

    explicit List(Reader& reader);
    explicit List(Operation operation = Operation::serialize);
    explicit List(Variants::Lists elements, Operation operation = Operation::serialize);

    u32 length() const;
    
    template<typename T>
    std::vector<T>& getVector() {
        try {
            return std::get<std::vector<T>>(elements);
        } catch (std::bad_variant_access& ex) {
            throw wrongDataType<T>();
        }
    }

    template<typename T>
    const std::vector<T>& getVector() const {
        try {
            return std::get<std::vector<T>>(elements);
        } catch (std::bad_variant_access& ex) {
            throw wrongDataType<T>();
        }
    }

    template<typename T>
    void setVector(const std::vector<T>& vector) {
        elements = vector;
    }

    template<typename T>
    void vector(std::vector<T>& value) {
        if (operation == Operation::deserialize) {
            value = getVector<T>();
        } else {
            setVector(value);
        }
    }

    void write(Writer& writer) const;
};



class Dynamic {
private:
    Variants::Variant variant{std::monostate{}};

    template<typename T>
    DecodeError wrongDataType() {
        std::string_view rightTag = std::visit([](auto&& contents) {
            return Serializer<std::remove_cvref_t<decltype(contents)>>::tagName;
        }, variant);
        std::string_view attemptedTag = Serializer<T>::tagName;
        return DecodeError{
            std::format("Tried to access dynamic of type '{}' as '{}'", rightTag, attemptedTag)
        };
    }

public:
    Operation operation;

    explicit Dynamic(Reader reader);
    explicit Dynamic(Operation operation = Operation::deserialize);
    explicit Dynamic(Variants::Variant value, Operation operation = Operation::serialize);

    template<typename T>
    const T& get() const {
        try {
            return std::get<T>(variant);
        } catch (std::bad_variant_access& ex) {
            throw wrongDataType<T>();
        }
    }

    template<typename T>
    T& get() {
        try {
            return std::get<T>(variant);
        } catch (std::bad_variant_access& ex) {
            throw wrongDataType<T>();
        }
    }

    template<typename T>
    void set(const T& value) {
        variant = value;
    }

    template<typename T>
    void value(T& val) {
        if (operation == Operation::deserialize) {
            val = get<T>();
        } else {
            set(val);
        }
    }

    DataTag getDataTag() const;

    void write(Writer& writer, bool skipTag = false) const;

};

template<typename T>
const T& Object::getField(const std::string& key) const {
    if (!hasField(key)) {
        throw DecodeError{std::format("Object missing key '{}'", key)};
    }
    return fields.at(key).get<T>();
}

template<typename T>
T& Object::getField(const std::string& key) {
    if (!hasField(key)) {
        throw DecodeError{std::format("Object missing key '{}'", key)};
    }
    return fields.at(key).get<T>();
}

template<typename T>
T Object::getField(const std::string& key, T defaultValue) const {
    if (!hasField(key)) {
        return defaultValue;
    }
    return fields.at(key).get<T>(); 
}

template<typename T>
void Object::setField(const std::string& key, const T& value) {
    fields.insert({key, Dynamic{value, operation}});
}

template<typename T>
void Object::field(const std::string& key, T& value) {
    if (operation == Operation::deserialize) {
        value = getField<T>(key);
    } else {
        setField(key, value);
    }
}

template<typename T>
T& Object::field(const std::string& key) {
    if (operation == Operation::deserialize) {
        return getField<T>(key);
    } else {
        setField(key, T{operation});
        return getField<T>(key);
    }
}

void serialize(std::filesystem::path path, const Dynamic& dynamic);
Dynamic deserialize(std::filesystem::path path);


void deserializationTest();


}

#endif
