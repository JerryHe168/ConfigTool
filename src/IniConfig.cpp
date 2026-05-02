#include "IniConfig.h"
#include "IniExceptions.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>

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

void IniConfig::loadFileOrThrow(const std::string& filepath)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    IniParser parser(m_options);
    ParseResult result = parser.parseFile(filepath);
    
    m_data = result.sections;
    m_includedFiles = result.includedFiles;
    m_warnings = result.warnings;
    m_loadedFile = filepath;
}

void IniConfig::loadStringOrThrow(const std::string& content)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    IniParser parser(m_options);
    ParseResult result = parser.parseString(content);
    
    m_data = result.sections;
    m_includedFiles = result.includedFiles;
    m_warnings = result.warnings;
    m_loadedFile.clear();
}

void IniConfig::loadStreamOrThrow(std::istream& stream)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    IniParser parser(m_options);
    ParseResult result = parser.parseStream(stream);
    
    m_data = result.sections;
    m_includedFiles = result.includedFiles;
    m_warnings = result.warnings;
    m_loadedFile.clear();
}

bool IniConfig::loadFile(const std::string& filepath) noexcept
{
    try {
        loadFileOrThrow(filepath);
        return true;
    } catch (...) {
        return false;
    }
}

bool IniConfig::loadString(const std::string& content) noexcept
{
    try {
        loadStringOrThrow(content);
        return true;
    } catch (...) {
        return false;
    }
}

bool IniConfig::loadStream(std::istream& stream) noexcept
{
    try {
        loadStreamOrThrow(stream);
        return true;
    } catch (...) {
        return false;
    }
}

void IniConfig::saveFileOrThrow(const std::string& filepath) const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw FileException(filepath, "open");
    }
    
    saveStreamOrThrow(file);
}

void IniConfig::saveStreamOrThrow(std::ostream& stream) const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    bool isFirstSection = true;
    
    for (const auto& [sectionName, keyValues] : m_data) {
        if (keyValues.empty() && sectionName != m_options.defaultSectionName) {
            if (!m_options.saveEmptySections) {
                continue;
            }
        }
        
        if (!isFirstSection) {
            stream << std::endl;
        }
        isFirstSection = false;
        
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
    }
    
    if (!stream.good()) {
        throw FileException("", "write");
    }
}

bool IniConfig::saveFile(const std::string& filepath) const noexcept
{
    try {
        saveFileOrThrow(filepath);
        return true;
    } catch (...) {
        return false;
    }
}

bool IniConfig::saveStream(std::ostream& stream) const noexcept
{
    try {
        saveStreamOrThrow(stream);
        return true;
    } catch (...) {
        return false;
    }
}

std::string IniConfig::saveToString() const noexcept
{
    std::ostringstream oss;
    saveStream(oss);
    return oss.str();
}

bool IniConfig::hasSection(const std::string& sectionName) const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_data.find(normalizeSection(sectionName)) != m_data.end();
}

bool IniConfig::hasKey(const std::string& sectionName, const std::string& keyName) const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    auto sectionIt = m_data.find(normalizeSection(sectionName));
    if (sectionIt == m_data.end()) {
        return false;
    }
    
    return sectionIt->second.find(normalizeKey(keyName)) != sectionIt->second.end();
}

std::vector<std::string> IniConfig::getSectionNames() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    std::vector<std::string> names;
    for (const auto& [sectionName, _] : m_data) {
        names.push_back(sectionName);
    }
    return names;
}

std::vector<std::string> IniConfig::getKeys(const std::string& sectionName) const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
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
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
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
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
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
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
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
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
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
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
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
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
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
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    auto it = m_data.find(normalizeSection(sectionName));
    if (it == m_data.end()) {
        return false;
    }
    
    m_data.erase(it);
    return true;
}

bool IniConfig::removeKey(const std::string& sectionName, const std::string& keyName)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
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
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    m_data.clear();
    m_includedFiles.clear();
    m_warnings.clear();
    m_validationErrors.clear();
    m_loadedFile.clear();
}

void IniConfig::setValidator(std::shared_ptr<IniValidator> validator)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_validator = validator;
}

std::vector<std::string> IniConfig::validateAndGetErrors() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    std::vector<std::string> errors;
    
    if (!m_validator) {
        return errors;
    }
    
    m_validator->validate(m_data, errors);
    return errors;
}

bool IniConfig::validate() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    m_validationErrors = validateAndGetErrors();
    return m_validationErrors.empty();
}

void IniConfig::clearValidationErrors()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_validationErrors.clear();
}

std::vector<std::string> IniConfig::getIncludedFiles() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_includedFiles;
}

std::vector<std::string> IniConfig::getWarnings() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_warnings;
}

std::vector<std::string> IniConfig::getValidationErrors() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_validationErrors;
}

ParseOptions IniConfig::getOptions() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_options;
}

void IniConfig::setOptions(const ParseOptions& options)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_options = options;
}

std::string IniConfig::getLoadedFile() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_loadedFile;
}

std::shared_ptr<JsonValue> IniConfig::toJson() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    auto root = JsonValue::createObject();
    
    for (const auto& [sectionName, keyValues] : m_data) {
        if (sectionName == m_options.defaultSectionName && keyValues.empty()) {
            continue;
        }
        
        auto sectionObj = JsonValue::createObject();
        
        for (const auto& [key, value] : keyValues) {
            bool parseSuccess = false;
            long long intVal = 0;
            double doubleVal = 0.0;
            bool boolVal = false;
            
            if (value == "true" || value == "false" || 
                value == "yes" || value == "no" ||
                value == "on" || value == "off") {
                boolVal = stringToBool(value, parseSuccess);
                if (parseSuccess) {
                    sectionObj->set(key, boolVal);
                    continue;
                }
            }
            
            try {
                intVal = std::stoll(value);
                if (std::to_string(intVal) == value) {
                    sectionObj->set(key, intVal);
                    continue;
                }
            } catch (...) {}
            
            try {
                doubleVal = std::stod(value);
                sectionObj->set(key, doubleVal);
                continue;
            } catch (...) {}
            
            sectionObj->set(key, value);
        }
        
        root->set(sectionName, sectionObj);
    }
    
    return root;
}

void IniConfig::fromJson(const std::shared_ptr<JsonValue>& json)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    if (!json || !json->isObject()) {
        throw JsonException("JSON root must be an object");
    }
    
    m_data.clear();
    m_includedFiles.clear();
    m_warnings.clear();
    m_validationErrors.clear();
    
    auto rootObj = json->getObject();
    auto defaultSection = normalizeSection(m_options.defaultSectionName);
    
    for (const auto& [sectionName, sectionValue] : rootObj) {
        if (!sectionValue) {
            continue;
        }
        
        if (!sectionValue->isObject()) {
            m_warnings.push_back("Skipping section '" + sectionName + "': value is not an object");
            continue;
        }
        
        auto normalizedSection = normalizeSection(sectionName);
        auto& targetSection = m_data[normalizedSection];
        
        auto keyValues = sectionValue->getObject();
        for (const auto& [key, value] : keyValues) {
            if (!value) {
                continue;
            }
            
            std::string strValue;
            switch (value->getType()) {
                case JsonType::Null:
                    strValue = "null";
                    break;
                case JsonType::Boolean:
                    strValue = value->getBoolean() ? "true" : "false";
                    break;
                case JsonType::Integer:
                    strValue = std::to_string(value->getInteger());
                    break;
                case JsonType::Double: {
                    std::ostringstream oss;
                    oss << std::setprecision(15) << value->getDouble();
                    strValue = oss.str();
                    break;
                }
                case JsonType::String:
                    strValue = value->getString();
                    break;
                case JsonType::Array:
                case JsonType::Object:
                    m_warnings.push_back("Skipping nested object/array at '" + sectionName + "." + key + "'");
                    continue;
            }
            
            targetSection[normalizeKey(key)] = strValue;
        }
    }
}

void IniConfig::loadJsonFileOrThrow(const std::string& filepath)
{
    JsonParser parser;
    auto result = parser.parseFile(filepath);
    fromJson(result.root);
}

void IniConfig::loadJsonStringOrThrow(const std::string& content)
{
    JsonParser parser;
    auto result = parser.parseString(content);
    fromJson(result.root);
}

void IniConfig::loadJsonStreamOrThrow(std::istream& stream)
{
    JsonParser parser;
    auto result = parser.parseStream(stream);
    fromJson(result.root);
}

bool IniConfig::loadJsonFile(const std::string& filepath) noexcept
{
    try {
        loadJsonFileOrThrow(filepath);
        return true;
    } catch (...) {
        return false;
    }
}

bool IniConfig::loadJsonString(const std::string& content) noexcept
{
    try {
        loadJsonStringOrThrow(content);
        return true;
    } catch (...) {
        return false;
    }
}

bool IniConfig::loadJsonStream(std::istream& stream) noexcept
{
    try {
        loadJsonStreamOrThrow(stream);
        return true;
    } catch (...) {
        return false;
    }
}

void IniConfig::saveJsonFileOrThrow(const std::string& filepath, int indent) const
{
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw FileException(filepath, "open");
    }
    
    saveJsonStreamOrThrow(file, indent);
}

void IniConfig::saveJsonStreamOrThrow(std::ostream& stream, int indent) const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    
    auto json = toJson();
    JsonParser::write(json, stream, indent);
    
    if (!stream.good()) {
        throw FileException("", "write");
    }
}

bool IniConfig::saveJsonFile(const std::string& filepath, int indent) const noexcept
{
    try {
        saveJsonFileOrThrow(filepath, indent);
        return true;
    } catch (...) {
        return false;
    }
}

bool IniConfig::saveJsonStream(std::ostream& stream, int indent) const noexcept
{
    try {
        saveJsonStreamOrThrow(stream, indent);
        return true;
    } catch (...) {
        return false;
    }
}

std::string IniConfig::saveToJsonString(int indent) const noexcept
{
    try {
        std::ostringstream oss;
        saveJsonStream(oss, indent);
        return oss.str();
    } catch (...) {
        return "";
    }
}

} // namespace INI
