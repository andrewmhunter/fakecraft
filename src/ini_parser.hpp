#ifndef INI_PARSER_HPP
#define INI_PARSER_HPP

#include <filesystem>
#include <format>
#include <istream>
#include <map>
#include <source_location>
#include <string>
#include <optional>

class IniFile {
private:
    std::map<std::string, std::map<std::string, std::string, std::less<>>, std::less<>> contents;

    void parse(std::istream& stream, std::string_view fileName, std::source_location location);

public:
    IniFile(std::istream& stream, std::string_view fileName, std::source_location location = std::source_location::current());
    IniFile(std::filesystem::path filePath, std::source_location location = std::source_location::current());

    std::optional<std::reference_wrapper<const std::string>> getString(std::string_view section, std::string_view key) const;
    const std::string& getString(std::string_view section, std::string_view key, std::string defaultValue) const;

    std::optional<int> getInt(std::string_view section, std::string_view key) const;
    int getInt(std::string_view section, std::string_view key, int defaultValue) const;

    std::optional<bool> getBool(std::string_view section, std::string_view key) const;
    bool getBool(std::string_view section, std::string_view key, int defaultValue) const;

    std::optional<float> getFloat(std::string_view section, std::string_view key) const;
    float getFloat(std::string_view section, std::string_view key, float defaultValue) const;

    std::format_context::iterator format_to(std::format_context::iterator it) const;
};

template<>
struct std::formatter<IniFile> {
    constexpr auto parse(std::format_parse_context& ctx) const {
        return ctx.begin();
    }

    auto format(const IniFile& iniFile, std::format_context& ctx) const {
        return iniFile.format_to(ctx.out());
    }
};

#endif
