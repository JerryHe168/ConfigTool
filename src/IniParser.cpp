#include "IniParser.h"
#include "IniExceptions.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

namespace INI {

IniParser::IniParser(const ParseOptions& options)
    : m_options(options)
    , m_currentLine(0)
{
}

void IniParser::setIncludeResolver(std::function<std::string(const std::string&, const std::string&)> resolver)
{
    m_includeResolver = resolver;
}

std::string IniParser::trim(const std::string& str)
{
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }
    
    size_t end = str.size();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }
    
    return str.substr(start, end - start);
}

std::string IniParser::unescape(const std::string& str)
{
    std::string result;
    result.reserve(str.size());
    
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\\' && i + 1 < str.size()) {
            switch (str[i + 1]) {
                case 'n': result += '\n'; ++i; break;
                case 't': result += '\t'; ++i; break;
                case '\\': result += '\\'; ++i; break;
                case ';': result += ';'; ++i; break;
                case '#': result += '#'; ++i; break;
                case 'r': result += '\r'; ++i; break;
                case '0': result += '\0'; ++i; break;
                default: result += str[i]; break;
            }
        } else {
            result += str[i];
        }
    }
    
    return result;
}

std::string IniParser::escape(const std::string& str)
{
    std::string result;
    result.reserve(str.size() * 2);
    
    for (char c : str) {
        switch (c) {
            case '\n': result += "\\n"; break;
            case '\t': result += "\\t"; break;
            case '\\': result += "\\\\"; break;
            case ';': result += "\\;"; break;
            case '#': result += "\\#"; break;
            case '\r': result += "\\r"; break;
            case '\0': result += "\\0"; break;
            default: result += c; break;
        }
    }
    
    return result;
}

std::string IniParser::normalizeSectionName(const std::string& name) const
{
    if (m_options.caseSensitiveSections) {
        return name;
    }
    
    std::string result = name;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

std::string IniParser::normalizeKeyName(const std::string& name) const
{
    if (m_options.caseSensitiveKeys) {
        return name;
    }
    
    std::string result = name;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

bool IniParser::isCommentLine(const std::string& line) const
{
    if (line.empty()) {
        return false;
    }
    
    size_t firstNonSpace = 0;
    while (firstNonSpace < line.size() && std::isspace(static_cast<unsigned char>(line[firstNonSpace]))) {
        ++firstNonSpace;
    }
    
    if (firstNonSpace >= line.size()) {
        return false;
    }
    
    return m_options.commentChars.find(line[firstNonSpace]) != std::string::npos;
}

bool IniParser::isEmptyLine(const std::string& line) const
{
    for (char c : line) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

bool IniParser::isSectionLine(const std::string& line) const
{
    size_t start = 0;
    while (start < line.size() && std::isspace(static_cast<unsigned char>(line[start]))) {
        ++start;
    }
    
    if (start >= line.size() || line[start] != '[') {
        return false;
    }
    
    size_t end = start + 1;
    while (end < line.size() && line[end] != ']') {
        ++end;
    }
    
    return end < line.size() && line[end] == ']';
}

std::string IniParser::extractSectionName(const std::string& line) const
{
    size_t start = line.find('[');
    if (start == std::string::npos) {
        throw ParseException("Invalid section line", m_currentLine);
    }
    
    size_t end = line.find(']', start + 1);
    if (end == std::string::npos) {
        throw ParseException("Missing closing bracket for section", m_currentLine);
    }
    
    std::string name = line.substr(start + 1, end - start - 1);
    
    for (char c : name) {
        if (c == '[' || c == ']' || c == '\r' || c == '\n' || c == '\\') {
            throw ParseException("Invalid character in section name: " + std::string(1, c), m_currentLine);
        }
    }
    
    if (m_options.trimWhitespace) {
        name = trim(name);
    }
    
    if (name.empty()) {
        throw ParseException("Section name cannot be empty", m_currentLine);
    }
    
    return name;
}

std::pair<std::string, std::string> IniParser::extractKeyValue(const std::string& line) const
{
    std::string processedLine = line;
    
    if (m_options.allowInlineComments) {
        bool inQuotes = false;
        bool lastWasBackslash = false;
        
        for (size_t i = 0; i < processedLine.size(); ++i) {
            char c = processedLine[i];
            
            if (c == '\\') {
                lastWasBackslash = !lastWasBackslash;
                continue;
            }
            
            if (c == '"' && !lastWasBackslash) {
                inQuotes = !inQuotes;
                lastWasBackslash = false;
                continue;
            }
            
            if (!inQuotes && !lastWasBackslash && 
                m_options.commentChars.find(c) != std::string::npos) {
                processedLine = processedLine.substr(0, i);
                break;
            }
            
            lastWasBackslash = false;
        }
    }
    
    size_t equalsPos = processedLine.find('=');
    if (equalsPos == std::string::npos) {
        std::string key = m_options.trimWhitespace ? trim(processedLine) : processedLine;
        if (key.empty()) {
            throw ParseException("Invalid key-value pair", m_currentLine);
        }
        return { key, "" };
    }
    
    std::string key = processedLine.substr(0, equalsPos);
    std::string value = processedLine.substr(equalsPos + 1);
    
    if (m_options.trimWhitespace) {
        key = trim(key);
        value = trim(value);
    }
    
    if (key.empty()) {
        throw ParseException("Key cannot be empty", m_currentLine);
    }
    
    for (char c : key) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '.') {
            throw ParseException("Invalid character in key: " + std::string(1, c), m_currentLine);
        }
    }
    
    return { key, value };
}

std::string IniParser::processValue(const std::string& value) const
{
    if (value.empty()) {
        return value;
    }
    
    std::string result = value;
    
    if (m_options.allowQuotedValues && result.size() >= 2 && result.front() == '"' && result.back() == '"') {
        result = result.substr(1, result.size() - 2);
    }
    
    if (m_options.processEscapes) {
        result = unescape(result);
    }
    
    return result;
}

std::string IniParser::getDirectoryPath(const std::string& filepath)
{
#ifdef _WIN32
    size_t lastSep = filepath.find_last_of("\\/");
#else
    size_t lastSep = filepath.find_last_of('/');
#endif
    
    if (lastSep == std::string::npos) {
        return "";
    }
    
    return filepath.substr(0, lastSep);
}

std::string IniParser::combinePaths(const std::string& base, const std::string& relative)
{
    if (base.empty()) {
        return relative;
    }
    
#ifdef _WIN32
    bool isAbsolute = (relative.size() >= 2 && relative[1] == ':') || 
                      (relative.size() >= 1 && (relative[0] == '\\' || relative[0] == '/'));
#else
    bool isAbsolute = !relative.empty() && relative[0] == '/';
#endif
    
    if (isAbsolute) {
        return relative;
    }
    
#ifdef _WIN32
    if (base.back() != '\\' && base.back() != '/') {
        return base + "\\" + relative;
    }
#else
    if (base.back() != '/') {
        return base + "/" + relative;
    }
#endif
    
    return base + relative;
}

void IniParser::processInclude(const std::string& includePath, ParseResult& result,
                               std::vector<std::string>& includeStack)
{
    std::string resolvedPath = includePath;
    
    if (m_includeResolver) {
        resolvedPath = m_includeResolver(m_currentFile, includePath);
    } else if (!m_currentFile.empty()) {
        std::string dir = getDirectoryPath(m_currentFile);
        resolvedPath = combinePaths(dir, includePath);
    }
    
    for (const auto& stackItem : includeStack) {
        if (stackItem == resolvedPath) {
            std::stringstream ss;
            ss << "Circular include detected: ";
            for (const auto& item : includeStack) {
                ss << item << " -> ";
            }
            ss << resolvedPath;
            throw IncludeException(m_currentFile, resolvedPath, ss.str());
        }
    }
    
    includeStack.push_back(resolvedPath);
    
    try {
        IniParser subParser(m_options);
        subParser.setIncludeResolver(m_includeResolver);
        ParseResult subResult = subParser.parseFile(resolvedPath);
        
        for (const auto& included : subResult.includedFiles) {
            result.includedFiles.push_back(included);
        }
        result.includedFiles.push_back(resolvedPath);
        
        for (const auto& warning : subResult.warnings) {
            result.warnings.push_back(warning);
        }
        
        for (const auto& [sectionName, keyValues] : subResult.sections) {
            auto& targetSection = result.sections[normalizeSectionName(sectionName)];
            for (const auto& [key, value] : keyValues) {
                targetSection[normalizeKeyName(key)] = value;
            }
        }
    } catch (const FileException& e) {
        throw IncludeException(m_currentFile, resolvedPath, "Cannot open include file: " + std::string(e.what()));
    }
    
    includeStack.pop_back();
}

ParseResult IniParser::parseFile(const std::string& filepath)
{
    m_currentFile = filepath;
    m_currentLine = 0;
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw FileException(filepath, "open");
    }
    
    return parseStream(file);
}

ParseResult IniParser::parseString(const std::string& content)
{
    m_currentFile = "";
    m_currentLine = 0;
    
    std::istringstream stream(content);
    return parseStream(stream);
}

ParseResult IniParser::parseStream(std::istream& stream)
{
    ParseResult result;
    std::string currentSection = normalizeSectionName(m_options.defaultSectionName);
    std::vector<std::string> includeStack;
    std::string normalizedIncludeSection = normalizeSectionName(m_options.includeSection);
    
    result.sections[currentSection] = {};
    
    std::string line;
    while (std::getline(stream, line)) {
        ++m_currentLine;
        
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (isEmptyLine(line) || isCommentLine(line)) {
            continue;
        }
        
        if (isSectionLine(line)) {
            std::string sectionName = extractSectionName(line);
            std::string normalizedName = normalizeSectionName(sectionName);
            
            if (result.sections.find(normalizedName) == result.sections.end() || 
                !m_options.mergeDuplicateSections) {
                result.sections[normalizedName] = {};
            }
            
            currentSection = normalizedName;
            continue;
        }
        
        try {
            auto [key, value] = extractKeyValue(line);
            std::string normalizedKey = normalizeKeyName(key);
            std::string processedValue = processValue(value);
            
            bool isIncludeKey = (normalizedKey == m_options.includeKey);
            bool isIncludeSection = m_options.includeSection.empty() || 
                                    (currentSection == normalizedIncludeSection);
            
            if (isIncludeKey && isIncludeSection) {
                processInclude(processedValue, result, includeStack);
            } else {
                result.sections[currentSection][normalizedKey] = processedValue;
            }
        } catch (const ParseException& e) {
            throw;
        }
    }
    
    return result;
}

} // namespace INI
