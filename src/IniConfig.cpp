#include "IniConfig.h"
#include "IniExceptions.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace INI {

IniConfig::IniConfig(const ParseOptions& options)
    : m_options(options)
{
}

std::string IniConfig::normalizeSection(const std::string& name) const
{
    if (m_options.caseSensitiveSections) {
        return name;
    }
    
    std::string result = name;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

std::string IniConfig::normalizeKey(const std::string& name) const
{
    if (m_options.caseSensitiveKeys) {
        return name;
    }
    
    std::string result = name;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

std::string IniConfig::boolToString(bool value)
{
    return value ? "true" : "false";
}

bool IniConfig::isTruthy(const std::string& str)
{
    std::string lower;
    for (char c : str) {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    
    return lower == "true" || lower == "yes" || lower == "1" || lower == "on";
}

bool IniConfig::isFalsy(const std::string& str)
{
    std::string lower;
    for (char c : str) {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    
    return lower == "false" || lower == "no" || lower == "0" || lower == "off";
}

bool IniConfig::stringToBool(const std::string& str, bool& success)
{
    success = true;
    
    if (isTruthy(str)) {
        return true;
    }
    
    if (isFalsy(str)) {
        return false;
    }
    
    success = false;
    return false;
}

int IniConfig::stringToInt(const std::string& str, bool& success)
{
    success = false;
    
    try {
        size_t pos = 0;
        int value = std::stoi(str, &pos);
        if (pos == str.size()) {
            success = true;
            return value;
        }
    } catch (...) {
    }
    
    return 0;
}

long IniConfig::stringToLong(const std::string& str, bool& success)
{
    success = false;
    
    try {
        size_t pos = 0;
        long value = std::stol(str, &pos);
        if (pos == str.size()) {
            success = true;
            return value;
        }
    } catch (...) {
    }
    
    return 0L;
}

double IniConfig::stringToDouble(const std::string& str, bool& success)
{
    success = false;
    
    try {
        size_t pos = 0;
        double value = std::stod(str, &pos);
        if (pos == str.size()) {
            success = true;
            return value;
        }
    } catch (...) {
    }
    
    return 0.0;
}

bool IniConfig::loadFile(const std::string& filepath)
{
    try {
        IniParser parser(m_options);
        ParseResult result = parser.parseFile(filepath);
        
        m_data = result.sections;
        m_includedFiles = result.includedFiles;
        m_warnings = result.warnings;
        m_loadedFile = filepath;
        
        return true;
    } catch (...) {
        return false;
    }
}

bool IniConfig::loadString(const std::string& content)
{
    try {
        IniParser parser(m_options);
        ParseResult result = parser.parseString(content);
        
        m_data = result.sections;
        m_includedFiles = result.includedFiles;
        m_warnings = result.warnings;
        m_loadedFile.clear();
        
        return true;
    } catch (...) {
        return false;
    }
}

bool IniConfig::loadStream(std::istream& stream)
{
    try {
        IniParser parser(m_options);
        ParseResult result = parser.parseStream(stream);
        
        m_data = result.sections;
        m_includedFiles = result.includedFiles;
        m_warnings = result.warnings;
        m_loadedFile.clear();
        
        return true;
    } catch (...) {
        return false;
    }
}

bool IniConfig::saveFile(const std::string& filepath) const
{
    try {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        return saveStream(file);
    } catch (...) {
        return false;
    }
}

bool IniConfig::saveStream(std::ostream& stream) const
{
    try {
        for (const auto& [sectionName, keyValues] : m_data) {
            if (keyValues.empty() && sectionName != m_options.defaultSectionName) {
                continue;
            }
            
            if (sectionName != m_options.defaultSectionName) {
                stream << "[" << sectionName << "]" << std::endl;
            }
            
            for (const auto& [key, value] : keyValues) {
                std::string escapedValue = IniParser::escape(value);
                bool needsQuotes = escapedValue.find_first_of(" \t\n\r\"") != std::string::npos;
                
                if (needsQuotes) {
                    stream << key << " = \"" << escapedValue << "\"" << std::endl;
                } else {
                    stream << key << " = " << escapedValue << std::endl;
                }
            }
            
            stream << std::endl;
        }
        
        return stream.good();
    } catch (...) {
        return false;
    }
}

std::string IniConfig::saveToString() const
{
    std::ostringstream oss;
    saveStream(oss);
    return oss.str();
}

bool IniConfig::hasSection(const std::string& sectionName) const
{
    return m_data.find(normalizeSection(sectionName)) != m_data.end();
}

bool IniConfig::hasKey(const std::string& sectionName, const std::string& keyName) const
{
    auto sectionIt = m_data.find(normalizeSection(sectionName));
    if (sectionIt == m_data.end()) {
        return false;
    }
    
    return sectionIt->second.find(normalizeKey(keyName)) != sectionIt->second.end();
}

std::vector<std::string> IniConfig::getSectionNames() const
{
    std::vector<std::string> names;
    for (const auto& [sectionName, _] : m_data) {
        names.push_back(sectionName);
    }
    return names;
}

std::vector<std::string> IniConfig::getKeys(const std::string& sectionName) const
{
    std::vector<std::string> keys;
    
    auto sectionIt = m_data.find(normalizeSection(sectionName));
    if (sectionIt == m_data.end()) {
        return keys;
    }
    
    for (const auto& [key, _] : sectionIt->second) {
        keys.push_back(key);
    }
    
    return keys;
}

std::string IniConfig::get(const std::string& sectionName, const std::string& keyName,
                            const std::string& defaultValue) const
{
    auto sectionIt = m_data.find(normalizeSection(sectionName));
    if (sectionIt == m_data.end()) {
        return defaultValue;
    }
    
    auto keyIt = sectionIt->second.find(normalizeKey(keyName));
    if (keyIt == sectionIt->second.end()) {
        return defaultValue;
    }
    
    return keyIt->second;
}

bool IniConfig::getBool(const std::string& sectionName, const std::string& keyName,
                         bool defaultValue) const
{
    bool success = false;
    return getBool(sectionName, keyName, defaultValue, &success);
}

bool IniConfig::getBool(const std::string& sectionName, const std::string& keyName,
                         bool defaultValue, bool* success) const
{
    if (success) *success = false;
    
    auto sectionIt = m_data.find(normalizeSection(sectionName));
    if (sectionIt == m_data.end()) {
        return defaultValue;
    }
    
    auto keyIt = sectionIt->second.find(normalizeKey(keyName));
    if (keyIt == sectionIt->second.end()) {
        return defaultValue;
    }
    
    bool localSuccess = false;
    bool value = stringToBool(keyIt->second, localSuccess);
    
    if (success) *success = localSuccess;
    return localSuccess ? value : defaultValue;
}

int IniConfig::getInt(const std::string& sectionName, const std::string& keyName,
                       int defaultValue) const
{
    bool success = false;
    return getInt(sectionName, keyName, defaultValue, &success);
}

int IniConfig::getInt(const std::string& sectionName, const std::string& keyName,
                       int defaultValue, bool* success) const
{
    if (success) *success = false;
    
    auto sectionIt = m_data.find(normalizeSection(sectionName));
    if (sectionIt == m_data.end()) {
        return defaultValue;
    }
    
    auto keyIt = sectionIt->second.find(normalizeKey(keyName));
    if (keyIt == sectionIt->second.end()) {
        return defaultValue;
    }
    
    bool localSuccess = false;
    int value = stringToInt(keyIt->second, localSuccess);
    
    if (success) *success = localSuccess;
    return localSuccess ? value : defaultValue;
}

long IniConfig::getLong(const std::string& sectionName, const std::string& keyName,
                         long defaultValue) const
{
    bool success = false;
    return getLong(sectionName, keyName, defaultValue, &success);
}

long IniConfig::getLong(const std::string& sectionName, const std::string& keyName,
                         long defaultValue, bool* success) const
{
    if (success) *success = false;
    
    auto sectionIt = m_data.find(normalizeSection(sectionName));
    if (sectionIt == m_data.end()) {
        return defaultValue;
    }
    
    auto keyIt = sectionIt->second.find(normalizeKey(keyName));
    if (keyIt == sectionIt->second.end()) {
        return defaultValue;
    }
    
    bool localSuccess = false;
    long value = stringToLong(keyIt->second, localSuccess);
    
    if (success) *success = localSuccess;
    return localSuccess ? value : defaultValue;
}

double IniConfig::getDouble(const std::string& sectionName, const std::string& keyName,
                             double defaultValue) const
{
    bool success = false;
    return getDouble(sectionName, keyName, defaultValue, &success);
}

double IniConfig::getDouble(const std::string& sectionName, const std::string& keyName,
                             double defaultValue, bool* success) const
{
    if (success) *success = false;
    
    auto sectionIt = m_data.find(normalizeSection(sectionName));
    if (sectionIt == m_data.end()) {
        return defaultValue;
    }
    
    auto keyIt = sectionIt->second.find(normalizeKey(keyName));
    if (keyIt == sectionIt->second.end()) {
        return defaultValue;
    }
    
    bool localSuccess = false;
    double value = stringToDouble(keyIt->second, localSuccess);
    
    if (success) *success = localSuccess;
    return localSuccess ? value : defaultValue;
}

std::string IniConfig::getString(const std::string& sectionName, const std::string& keyName,
                                  const std::string& defaultValue) const
{
    return get(sectionName, keyName, defaultValue);
}

void IniConfig::set(const std::string& sectionName, const std::string& keyName,
                     const std::string& value)
{
    m_data[normalizeSection(sectionName)][normalizeKey(keyName)] = value;
}

void IniConfig::setBool(const std::string& sectionName, const std::string& keyName, bool value)
{
    set(sectionName, keyName, boolToString(value));
}

void IniConfig::setInt(const std::string& sectionName, const std::string& keyName, int value)
{
    set(sectionName, keyName, std::to_string(value));
}

void IniConfig::setLong(const std::string& sectionName, const std::string& keyName, long value)
{
    set(sectionName, keyName, std::to_string(value));
}

void IniConfig::setDouble(const std::string& sectionName, const std::string& keyName, double value)
{
    std::ostringstream oss;
    oss.precision(15);
    oss << value;
    set(sectionName, keyName, oss.str());
}

bool IniConfig::removeSection(const std::string& sectionName)
{
    auto it = m_data.find(normalizeSection(sectionName));
    if (it == m_data.end()) {
        return false;
    }
    
    m_data.erase(it);
    return true;
}

bool IniConfig::removeKey(const std::string& sectionName, const std::string& keyName)
{
    auto sectionIt = m_data.find(normalizeSection(sectionName));
    if (sectionIt == m_data.end()) {
        return false;
    }
    
    auto keyIt = sectionIt->second.find(normalizeKey(keyName));
    if (keyIt == sectionIt->second.end()) {
        return false;
    }
    
    sectionIt->second.erase(keyIt);
    return true;
}

void IniConfig::clear()
{
    m_data.clear();
    m_includedFiles.clear();
    m_warnings.clear();
    m_validationErrors.clear();
    m_loadedFile.clear();
}

void IniConfig::setValidator(std::shared_ptr<IniValidator> validator)
{
    m_validator = validator;
}

bool IniConfig::validate() const
{
    m_validationErrors.clear();
    
    if (!m_validator) {
        return true;
    }
    
    return m_validator->validate(m_data, m_validationErrors);
}

} // namespace INI
