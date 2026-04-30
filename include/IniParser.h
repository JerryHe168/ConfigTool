#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>

namespace INI {

struct ParseOptions {
    bool caseSensitiveSections = true;
    bool caseSensitiveKeys = true;
    bool allowInlineComments = true;
    bool mergeDuplicateSections = true;
    bool trimWhitespace = true;
    bool allowQuotedValues = true;
    bool processEscapes = true;
    std::string commentChars = ";#";
    std::string includeKey = "include";
    std::string defaultSectionName = "DEFAULT";
};

struct ParseResult {
    std::map<std::string, std::map<std::string, std::string>> sections;
    std::vector<std::string> includedFiles;
    std::vector<std::string> warnings;
};

class IniParser {
public:
    explicit IniParser(const ParseOptions& options = ParseOptions());
    
    ParseResult parseFile(const std::string& filepath);
    ParseResult parseString(const std::string& content);
    ParseResult parseStream(std::istream& stream);
    
    const ParseOptions& getOptions() const noexcept { return m_options; }
    void setOptions(const ParseOptions& options) { m_options = options; }
    
    void setIncludeResolver(std::function<std::string(const std::string&, const std::string&)> resolver);
    
    static std::string trim(const std::string& str);
    static std::string unescape(const std::string& str);
    static std::string escape(const std::string& str);
    
private:
    ParseOptions m_options;
    std::function<std::string(const std::string&, const std::string&)> m_includeResolver;
    std::string m_currentFile;
    size_t m_currentLine;
    
    std::string normalizeSectionName(const std::string& name) const;
    std::string normalizeKeyName(const std::string& name) const;
    
    bool isCommentLine(const std::string& line) const;
    bool isEmptyLine(const std::string& line) const;
    bool isSectionLine(const std::string& line) const;
    
    std::string extractSectionName(const std::string& line) const;
    std::pair<std::string, std::string> extractKeyValue(const std::string& line) const;
    std::string processValue(const std::string& value) const;
    
    void processInclude(const std::string& includePath, ParseResult& result, 
                       std::vector<std::string>& includeStack);
    
    static std::string getDirectoryPath(const std::string& filepath);
    static std::string combinePaths(const std::string& base, const std::string& relative);
};

} // namespace INI
