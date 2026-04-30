#pragma once

#include <stdexcept>
#include <string>

namespace INI {

class IniException : public std::runtime_error {
public:
    explicit IniException(const std::string& message)
        : std::runtime_error(message) {}
    
    explicit IniException(const char* message)
        : std::runtime_error(message) {}
};

class ParseException : public IniException {
public:
    ParseException(const std::string& message, size_t lineNumber)
        : IniException("Parse error at line " + std::to_string(lineNumber) + ": " + message)
        , m_lineNumber(lineNumber) {}
    
    size_t getLineNumber() const noexcept { return m_lineNumber; }
    
private:
    size_t m_lineNumber;
};

class FileException : public IniException {
public:
    explicit FileException(const std::string& filename, const std::string& operation)
        : IniException("Failed to " + operation + " file: " + filename)
        , m_filename(filename) {}
    
    const std::string& getFilename() const noexcept { return m_filename; }
    
private:
    std::string m_filename;
};

class SectionNotFoundException : public IniException {
public:
    explicit SectionNotFoundException(const std::string& sectionName)
        : IniException("Section not found: " + sectionName)
        , m_sectionName(sectionName) {}
    
    const std::string& getSectionName() const noexcept { return m_sectionName; }
    
private:
    std::string m_sectionName;
};

class KeyNotFoundException : public IniException {
public:
    KeyNotFoundException(const std::string& sectionName, const std::string& keyName)
        : IniException("Key '" + keyName + "' not found in section '" + sectionName + "'")
        , m_sectionName(sectionName)
        , m_keyName(keyName) {}
    
    const std::string& getSectionName() const noexcept { return m_sectionName; }
    const std::string& getKeyName() const noexcept { return m_keyName; }
    
private:
    std::string m_sectionName;
    std::string m_keyName;
};

class TypeConversionException : public IniException {
public:
    TypeConversionException(const std::string& value, const std::string& targetType)
        : IniException("Cannot convert value '" + value + "' to type " + targetType)
        , m_value(value)
        , m_targetType(targetType) {}
    
    const std::string& getValue() const noexcept { return m_value; }
    const std::string& getTargetType() const noexcept { return m_targetType; }
    
private:
    std::string m_value;
    std::string m_targetType;
};

class ValidationException : public IniException {
public:
    ValidationException(const std::string& sectionName, const std::string& keyName, 
                        const std::string& message)
        : IniException("Validation failed for [" + sectionName + "]." + keyName + ": " + message)
        , m_sectionName(sectionName)
        , m_keyName(keyName) {}
    
    const std::string& getSectionName() const noexcept { return m_sectionName; }
    const std::string& getKeyName() const noexcept { return m_keyName; }
    
private:
    std::string m_sectionName;
    std::string m_keyName;
};

class IncludeException : public IniException {
public:
    IncludeException(const std::string& parentFile, const std::string& includeFile, 
                     const std::string& reason)
        : IniException("Include error: " + reason + " (parent: " + parentFile + ", include: " + includeFile + ")")
        , m_parentFile(parentFile)
        , m_includeFile(includeFile) {}
    
    const std::string& getParentFile() const noexcept { return m_parentFile; }
    const std::string& getIncludeFile() const noexcept { return m_includeFile; }
    
private:
    std::string m_parentFile;
    std::string m_includeFile;
};

} // namespace INI
