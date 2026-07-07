#include "ini_parser.hpp"
#include "util/util.hpp"
#include "logger.hpp"
#include <format>
#include <fstream>
#include <functional>
#include <optional>
#include <source_location>
#include <stdexcept>
#include <string>

void IniFile::iniParseError(int lineNumber, std::string_view message, std::source_location location) {
    Logger::error(std::format("While parsing {}:{}: {}", fileName, lineNumber, message), location);
}

IniFile::IniFile(std::istream& stream, std::string fileName, std::source_location location)
    : fileName{fileName}
{
    parse(stream, location);
}

IniFile::IniFile(std::filesystem::path filePath, std::source_location location)
    : fileName{filePath.filename().string()}
{
    std::ifstream stream{filePath};
    parse(stream, location);
}

void IniFile::parse(std::istream& stream, std::source_location location) {
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
                iniParseError(lineNumber, "Missing closing ]", location);
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
            iniParseError(lineNumber, "Expected section or key value pair", location);
            continue;
        }

        std::string key = line.substr(0, equalsSign);
        trimRight(key);
        std::string value = line.substr(equalsSign + 1);
        trimLeft(value);

        if (key.empty()) {
            iniParseError(lineNumber, std::format("Missing key for value \"{}\"", value), location);
            continue;
        }
        if (value.empty()) {
            iniParseError(lineNumber, std::format("Missing value for key \"{}\"", key), location);
            continue;
        }

        auto& sectionMap = contents.try_emplace(currentSection).first->second;
        bool inserted = sectionMap.insert_or_assign(key, value).second;

        if (!inserted) {
            iniParseError(lineNumber, std::format("Duplicate key \"{}\"", key), location);
            continue;
        }
    }
}

std::optional<std::reference_wrapper<const std::string>> IniFile::getString(std::string_view section, std::string_view key, LogLevel requirement, std::source_location location) const {
    if (contents.contains(section)) {
        const auto& sectionMap = contents.find(section);
        if (sectionMap->second.contains(key)) {
            return sectionMap->second.find(key)->second;
        }
    }
    Logger::log(requirement, std::format("\"{}\" missing field \"{}.{}\"", fileName, section, key), location);
    return std::nullopt;
}

const std::string& IniFile::getString(std::string_view section, std::string_view key, std::string defaultValue, LogLevel requirement, std::source_location location) const {
    return getString(section, key, requirement, location).value_or(defaultValue);
}

std::optional<int> IniFile::getInt(std::string_view section, std::string_view key, LogLevel requirement, std::source_location location) const {
    std::optional<std::string> value = getString(section, key, requirement, location);
    if (!value.has_value()) {
        return std::nullopt;
    }
    std::string& str = value.value();

    try {
        return std::optional{std::stoi(str)};
    } catch (std::invalid_argument& ex) {
        Logger::error(std::format("In \"{}\" field \"{}.{} = {}\" must be an integer", fileName, section, key, str), location);
    } catch (std::out_of_range& ex) {
        Logger::error(std::format("In \"{}\" field \"{}.{} = {}\" is to large or to small", fileName, section, key, str), location);
    }

    return std::nullopt;
}

int IniFile::getInt(std::string_view section, std::string_view key, int defaultValue, LogLevel requirement, std::source_location location) const {
    return getInt(section, key, requirement, location).value_or(defaultValue);
}

std::optional<bool> IniFile::getBool(std::string_view section, std::string_view key, LogLevel requirement, std::source_location location) const {
    std::optional<std::string> value = getString(section, key, requirement, location);
    
    std::string lowered = toLower(value.value());
    if (lowered == "true") {
        return std::optional{true};
    }
    if (lowered == "false") {
        return std::optional{false};
    }

    Logger::error(std::format("In \"{}\" field \"{}.{} = {}\" must be 'true' or 'false'", fileName, section, key, value.value()), location);
    return std::optional<bool>{std::nullopt};
}

bool IniFile::getBool(std::string_view section, std::string_view key, int defaultValue, LogLevel requirement, std::source_location location) const {
    return getBool(section, key, requirement, location).value_or(defaultValue);
}

std::optional<float> IniFile::getFloat(std::string_view section, std::string_view key, LogLevel requirement, std::source_location location) const {
    std::optional<std::string> value = getString(section, key, requirement, location);
    if (!value.has_value()) {
        return std::nullopt;
    }
    std::string& str = value.value();

    try {
        return std::optional{std::stof(str)};
    } catch (std::invalid_argument& ex) {
        Logger::error(std::format("In \"{}\" field \"{}.{} = {}\" must be an number", fileName, section, key, str), location);
    } catch (std::out_of_range& ex) {
        Logger::error(std::format("In \"{}\" field \"{}.{} = {}\" is to large or to small", fileName, section, key, str), location);
    }

    return std::nullopt;
}

float IniFile::getFloat(std::string_view section, std::string_view key, float defaultValue, LogLevel requirement, std::source_location location) const {
    return getFloat(section, key, requirement, location).value_or(defaultValue);
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
