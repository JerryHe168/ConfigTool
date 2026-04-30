#pragma once

#include "IniParser.h"
#include "IniValidator.h"

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>

namespace INI {

class IniConfig {
public:
    explicit IniConfig(const ParseOptions& options = ParseOptions());
    
    bool loadFile(const std::string& filepath);
    bool loadString(const std::string& content);
    bool loadStream(std::istream& stream);
    
    bool saveFile(const std::string& filepath) const;
    bool saveStream(std::ostream& stream) const;
    std::string saveToString() const;
    
    bool hasSection(const std::string& sectionName) const;
    bool hasKey(const std::string& sectionName, const std::string& keyName) const;
    
    std::vector<std::string> getSectionNames() const;
    std::vector<std::string> getKeys(const std::string& sectionName) const;
    
    std::string get(const std::string& sectionName, const std::string& keyName,
                    const std::string& defaultValue = "") const;
    
    template<typename T>
    T get(const std::string& sectionName, const std::string& keyName,
          const T& defaultValue = T()) const;
    
    bool getBool(const std::string& sectionName, const std::string& keyName,
                 bool defaultValue = false) const;
    bool getBool(const std::string& sectionName, const std::string& keyName,
                 bool defaultValue, bool* success) const;
    
    int getInt(const std::string& sectionName, const std::string& keyName,
               int defaultValue = 0) const;
    int getInt(const std::string& sectionName, const std::string& keyName,
               int defaultValue, bool* success) const;
    
    long getLong(const std::string& sectionName, const std::string& keyName,
                 long defaultValue = 0L) const;
    long getLong(const std::string& sectionName, const std::string& keyName,
                 long defaultValue, bool* success) const;
    
    double getDouble(const std::string& sectionName, const std::string& keyName,
                     double defaultValue = 0.0) const;
    double getDouble(const std::string& sectionName, const std::string& keyName,
                     double defaultValue, bool* success) const;
    
    std::string getString(const std::string& sectionName, const std::string& keyName,
                          const std::string& defaultValue = "") const;
    
    void set(const std::string& sectionName, const std::string& keyName,
             const std::string& value);
    void setBool(const std::string& sectionName, const std::string& keyName, bool value);
    void setInt(const std::string& sectionName, const std::string& keyName, int value);
    void setLong(const std::string& sectionName, const std::string& keyName, long value);
    void setDouble(const std::string& sectionName, const std::string& keyName, double value);
    
    bool removeSection(const std::string& sectionName);
    bool removeKey(const std::string& sectionName, const std::string& keyName);
    
    void clear();
    
    void setValidator(std::shared_ptr<IniValidator> validator);
    bool validate() const;
    
    const std::vector<std::string>& getIncludedFiles() const noexcept { return m_includedFiles; }
    const std::vector<std::string>& getWarnings() const noexcept { return m_warnings; }
    const std::vector<std::string>& getValidationErrors() const noexcept { return m_validationErrors; }
    
    const ParseOptions& getOptions() const noexcept { return m_options; }
    void setOptions(const ParseOptions& options) { m_options = options; }
    
    const std::string& getLoadedFile() const noexcept { return m_loadedFile; }
    
private:
    ParseOptions m_options;
    std::map<std::string, std::map<std::string, std::string>> m_data;
    std::vector<std::string> m_includedFiles;
    std::vector<std::string> m_warnings;
    mutable std::vector<std::string> m_validationErrors;
    std::shared_ptr<IniValidator> m_validator;
    std::string m_loadedFile;
    
    std::string normalizeSection(const std::string& name) const;
    std::string normalizeKey(const std::string& name) const;
    
    static std::string boolToString(bool value);
    static bool stringToBool(const std::string& str, bool& success);
    static int stringToInt(const std::string& str, bool& success);
    static long stringToLong(const std::string& str, bool& success);
    static double stringToDouble(const std::string& str, bool& success);
    
    static bool isTruthy(const std::string& str);
    static bool isFalsy(const std::string& str);
};

} // namespace INI
