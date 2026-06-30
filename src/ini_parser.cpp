#include "ini_parser.hpp"
#include "util.hpp"
#include "logger.hpp"
#include <format>
#include <fstream>
#include <functional>
#include <optional>
#include <source_location>
#include <string>

template<typename... Args>
static void iniParseError(std::string_view fileName, int lineNumber, std::string_view message, std::source_location location) {
    Logger::warning(std::format("While parsing {}:{}: {}", fileName, lineNumber, message), location);
}

IniFile::IniFile(std::istream& stream, std::string_view fileName, std::source_location location) {
    parse(stream, fileName, location);
}

IniFile::IniFile(std::filesystem::path filePath, std::source_location location) {
    std::ifstream stream{filePath};
    parse(stream, filePath.filename().string(), location);
}

void IniFile::parse(std::istream& stream, std::string_view fileName, std::source_location location) {
    std::string line{};
    std::string currentSection{};
    int lineNumber = 0;

    while (std::getline(stream, line)) {
        lineNumber += 1;
    
        trim(line);
        if (line.length() == 0 || line[0] == ';') {
            continue;
        }

        if (line.starts_with('[')) {
            if (!line.ends_with(']')) {
                iniParseError(fileName, lineNumber, "Missing closing ]", location);
                continue;
            }
            line.erase(line.begin());
            line.erase(line.end() - 1);
            trim(line);
            currentSection = line;
            trim(currentSection);
            continue;
        }

        std::size_t equalsSign = line.find('=');
        if (equalsSign == std::string::npos) {
            iniParseError(fileName, lineNumber, "Expected section or key value pair", location);
            continue;
        }

        std::string key = line.substr(0, equalsSign);
        trimRight(key);
        std::string value = line.substr(equalsSign + 1);
        trimLeft(value);

        if (key.empty()) {
            iniParseError(fileName, lineNumber, std::format("Missing key for value \"{}\"", value), location);
            continue;
        }
        if (value.empty()) {
            iniParseError(fileName, lineNumber, std::format("Missing value for key \"{}\"", key), location);
            continue;
        }

        auto& sectionMap = contents.try_emplace(currentSection).first->second;
        bool inserted = sectionMap.insert_or_assign(key, value).second;

        if (!inserted) {
            iniParseError(fileName, lineNumber, std::format("Duplicate key \"{}\"", key), location);
            continue;
        }
    }
}

std::optional<std::reference_wrapper<const std::string>> IniFile::getString(std::string_view section, std::string_view key) const {
    if (contents.contains(section)) {
        const auto& sectionMap = contents.find(section);
        if (sectionMap->second.contains(key)) {
            return sectionMap->second.find(key)->second;
        }
    }
    return std::nullopt;
}

const std::string& IniFile::getString(std::string_view section, std::string_view key, std::string defaultValue) const {
    return getString(section, key).value_or(defaultValue);
}

std::optional<int> IniFile::getInt(std::string_view section, std::string_view key) const {
    return getString(section, key).and_then([](auto str) {
        return std::optional{std::stoi(str)};
    });
}

int IniFile::getInt(std::string_view section, std::string_view key, int defaultValue) const {
    return getInt(section, key).value_or(defaultValue);
}

std::optional<bool> IniFile::getBool(std::string_view section, std::string_view key) const {
    return getString(section, key).and_then([](auto str) {
        std::string lowered = toLower(str.get());
        if (lowered == "true") {
            return std::optional{true};
        }
        if (lowered == "false") {
            return std::optional{false};
        }

        return std::optional<bool>{std::nullopt};
    });
}

bool IniFile::getBool(std::string_view section, std::string_view key, int defaultValue) const {
    return getBool(section, key).value_or(defaultValue);
}

std::optional<float> IniFile::getFloat(std::string_view section, std::string_view key) const {
    return getString(section, key).and_then([](auto str) {
        return std::optional{std::stof(str)};
    });
}

float IniFile::getFloat(std::string_view section, std::string_view key, float defaultValue) const {
    return getFloat(section, key).value_or(defaultValue);
}


std::format_context::iterator IniFile::format_to(std::format_context::iterator it) const {
    for (const auto& section : contents) {
        it = std::format_to(it, "[{}]\n", section.first);
        for (const auto& keyValue : section.second) {
            it = std::format_to(it, "{} = {}\n", keyValue.first, keyValue.second);
        }
        it = std::format_to(it, "\n");
    }
    return it;
}
