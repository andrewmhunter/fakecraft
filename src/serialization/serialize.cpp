#include "serialize.hpp"
#include "engine/logger.hpp"
#include "util/types.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <glm/fwd.hpp>
#include <memory>
#include <print>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace ser {

Error::Error(const std::string& what)
    : std::runtime_error{what}
{}

ParseError::ParseError(const std::string& what)
    : Error{what}
{}

WriteError::WriteError(const std::string& what)
    : Error{what}
{}

DecodeError::DecodeError(const std::string& what)
    : Error{what}
{}

constexpr Variants::TypeContainers dataTagGetType(DataTag tag) {
    std::variant<std::monostate, std::unique_ptr<int>> v;
    v = std::make_unique<int>(32);

    switch (tag) {
        /*case DataTag::dynamic:
            return TypeContainer<Dynamic>{};*/
        case DataTag::nil:
            return TypeContainer<std::monostate>{};
        case DataTag::boolean:
            return TypeContainer<bool>{};
        case DataTag::i8:
            return TypeContainer<i8>{};
        case DataTag::u8:
            return TypeContainer<u8>{};
        case DataTag::i16:
            return TypeContainer<i16>{};
        case DataTag::u16:
            return TypeContainer<u16>{};
        case DataTag::i32:
            return TypeContainer<i32>{};
        case DataTag::u32:
            return TypeContainer<u32>{};
        case DataTag::i64:
            return TypeContainer<i64>{};
        case DataTag::u64:
            return TypeContainer<u64>{};
        case DataTag::f32:
            return TypeContainer<float>{};
        case DataTag::f64:
            return TypeContainer<double>{};
        case DataTag::vec3:
            return TypeContainer<glm::vec3>{};
        case DataTag::ivec3:
            return TypeContainer<glm::ivec3>{};
        case DataTag::string:
            return TypeContainer<std::string>{};
        case DataTag::object:
            return TypeContainer<Object>{};
        case DataTag::list:
            return TypeContainer<List>{};
    }
    throw ParseError{std::format("Invalid data tag {}", static_cast<int>(tag))};
}


std::monostate Serializer<std::monostate>::read(Reader& reader) {
    (void)reader;
    return std::monostate{};
}

void Serializer<std::monostate>::write(Writer& writer, const std::monostate& value) {
    // Ignore unused parameter warnings
    (void)writer;
    (void)value;
}


bool Serializer<bool>::read(Reader& reader) {
    return reader.readByte();
}

void Serializer<bool>::write(Writer& writer, const bool& value) {
    writer.writeByte(value);
}


i8 Serializer<i8>::read(Reader& reader) {
    return reader.readByte();
}

void Serializer<i8>::write(Writer& writer, const i8& value) {
    writer.writeByte(value);
}


u8 Serializer<u8>::read(Reader& reader) {
    return reader.readByte();
}

void Serializer<u8>::write(Writer& writer, const u8& value) {
    writer.writeByte(value);
}


i16 Serializer<i16>::read(Reader& reader) {
    return reader.readInt<i16>();
}

void Serializer<i16>::write(Writer& writer, const i16& value) {
    writer.writeInt<i16>(value);
}


u16 Serializer<u16>::read(Reader& reader) {
    return reader.readInt<u16>();
}

void Serializer<u16>::write(Writer& writer, const u16& value) {
    writer.writeInt<u16>(value);
}


i32 Serializer<i32>::read(Reader& reader) {
    return reader.readInt<i32>();
}

void Serializer<i32>::write(Writer& writer, const i32& value) {
    writer.writeInt(value);
}


u32 Serializer<u32>::read(Reader& reader) {
    return reader.readInt<u32>();
}

void Serializer<u32>::write(Writer& writer, const u32& value) {
    writer.writeInt(value);
}


i64 Serializer<i64>::read(Reader& reader) {
    return reader.readInt<i64>();
}

void Serializer<i64>::write(Writer& writer, const i64& value) {
    writer.writeInt(value);
}


u64 Serializer<u64>::read(Reader& reader) {
    return reader.readInt<u64>();
}

void Serializer<u64>::write(Writer& writer, const u64& value) {
    writer.writeInt(value);
}


float Serializer<float>::read(Reader& reader) {
    return reader.readF32();
}

void Serializer<float>::write(Writer& writer, const float& value) {
    writer.writeF32(value);
}


double Serializer<double>::read(Reader& reader) {
    return reader.readF64();
}

void Serializer<double>::write(Writer& writer, const double& value) {
    writer.writeF64(value);
}


glm::vec3 Serializer<glm::vec3>::read(Reader& reader) {
    glm::vec3 value{};
    value.x = reader.readF32();
    value.y = reader.readF32();
    value.z = reader.readF32();
    return value;
}

void Serializer<glm::vec3>::write(Writer& writer, const glm::vec3& value) {
    writer.writeF32(value.x);
    writer.writeF32(value.y);
    writer.writeF32(value.z);
}


glm::ivec3 Serializer<glm::ivec3>::read(Reader& reader) {
    glm::vec3 value{};
    value.x = reader.readInt<i32>();
    value.y = reader.readInt<i32>();
    value.z = reader.readInt<i32>();
    return value;
}

void Serializer<glm::ivec3>::write(Writer& writer, const glm::ivec3& value) {
    writer.writeInt<i32>(value.x);
    writer.writeInt<i32>(value.y);
    writer.writeInt<i32>(value.z);
}


std::string Serializer<std::string>::read(Reader& reader) {
    return reader.readString<LengthType>();
}

void Serializer<std::string>::write(Writer& writer, const std::string& value) {
    writer.writeString<LengthType>(value);
}


Object Serializer<Object>::read(Reader& reader) {
    return Object{reader};
}

void Serializer<Object>::write(Writer& writer, const Object& value) {
    value.write(writer);
}


List Serializer<List>::read(Reader& reader) {
    return List{reader};
}

void Serializer<List>::write(Writer& writer, const List& value) {
    value.write(writer);
}



Dynamic::Dynamic(Reader reader)
    : Dynamic{Operation::deserialize}
{
    DataTag tag = reader.readTag();

    Variants::TypeContainers typeContainer = dataTagGetType(tag);
    variant = std::visit([&reader](auto&& typeContainer) {
        using Type = std::remove_cvref_t<decltype(typeContainer)>::Type;
        return Variants::Variant{Serializer<Type>::read(reader)};
    }, typeContainer);
}

Dynamic::Dynamic(Operation operation)
    : Dynamic{std::monostate{}, operation}
{}

Dynamic::Dynamic(Variants::Variant value, Operation operation)
    : variant{value}, operation{operation}
{}

DataTag Dynamic::getDataTag() const {
    return std::visit([](auto&& arg) {
        return Serializer<std::remove_cvref_t<decltype(arg)>>::tag;
    }, variant);
}

void Dynamic::write(Writer& writer, bool skipTag) const {
    if (!skipTag) {
        writer.writeTag(getDataTag());
    }

    std::visit([&writer](auto&& value) {
        Serializer<std::remove_cvref_t<decltype(value)>>::write(writer, value);
    }, variant);
}



Reader::Reader(std::istream& stream) : stream{stream} {}

u8 Reader::readByte() {
    int ch = stream.get();
    if (ch == std::ios::traits_type::eof()) {
        throw ParseError{"Reached end of file"};
    }
    if (stream.fail()) {
        throw ParseError{"Failed to read byte due to IO error"};
    }
    return ch;
}

float Reader::readF32() {
    float value = 0.f;
    u32 from = readInt<u32>();
    static_assert(sizeof(value) == sizeof(from));
    std::memcpy(&value, &from, sizeof(value));
    return value;
}

double Reader::readF64() {
    double value = 0.f;
    u64 from = readInt<u64>();
    static_assert(sizeof(value) == sizeof(from));
    std::memcpy(&value, &from, sizeof(value));
    return value;   
}

DataTag Reader::readTag() {
    return static_cast<DataTag>(readByte());
}


Writer::Writer(std::ostream& stream) : stream{stream} {}

void Writer::writeByte(u8 byte) {
    stream.put(byte);
    if (stream.fail()) {
        throw WriteError{"Failed to write byte due to IO error"};
    }
}

void Writer::writeTag(DataTag tag) {
    writeByte(static_cast<u8>(tag));
}

void Writer::writeF32(float value) {
    u32 into = 0;
    static_assert(sizeof(value) == sizeof(into));
    std::memcpy(&into, &value, sizeof(value));
    writeInt(into);
}

void Writer::writeF64(double value) {
    u64 into = 0;
    static_assert(sizeof(value) == sizeof(into));
    std::memcpy(&into, &value, sizeof(value));
    writeInt(into);
}


Object::Object(Reader& reader)
    : Object{Operation::deserialize}
{
    u32 count = reader.readInt<u32>();
    for (u32 i = 0; i < count; ++i) {
        std::string key = reader.readString<KeyLengthType>();
        fields.emplace(key, reader);
    }
}

Object::Object(Operation operation)
    : operation{operation}
{}

void Object::write(Writer& writer) const {
    writer.writeInt<u32>(fields.size());
    for (auto& field : fields) {
        writer.writeString<KeyLengthType>(field.first);
        field.second.write(writer);
    }
}

bool Object::hasField(const std::string& key) const {
    return fields.contains(key);
}


List::List(Reader& reader)
    : List{Operation::deserialize}
{
    u32 len = reader.readInt<u32>();
    DataTag tag = reader.readTag();

    Variants::TypeContainers typeContainer = dataTagGetType(tag);
    elements = std::visit([&reader, len](auto&& typeContainer) {
        using Type = std::remove_cvref_t<decltype(typeContainer)>::Type;
        std::vector<Type> vector;
        for (u32 i = 0; i < len; ++i) {
            Type value = Serializer<Type>::read(reader);
            vector.push_back(value);
        }
        return Variants::Lists{vector};
    }, typeContainer);
}

List::List(Operation operation)
    : operation{operation}
{}

List::List(Variants::Lists elements, Operation operation)
    : elements{elements}, operation{operation}
{}

u32 List::length() const {
    return std::visit([](auto&& arg) { return arg.size(); }, elements);
}

void List::write(Writer& writer) const {
    writer.writeInt<u32>(length());

    std::visit([&writer](auto&& elements) {
        using Type = std::remove_cvref_t<decltype(elements)>::value_type;
        writer.writeTag(Serializer<Type>::tag);
        for (const auto& element : elements) {
            Serializer<Type>::write(writer, element);
        }
    }, elements);
}


void serialize(std::filesystem::path path, const Dynamic& dynamic) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file{path};
    Writer writer{file};
    dynamic.write(writer);
}

Dynamic deserialize(std::filesystem::path path) {
    std::ifstream file{path};
    Reader reader{file};
    Dynamic dynamic{reader};
    return dynamic;
}

}
