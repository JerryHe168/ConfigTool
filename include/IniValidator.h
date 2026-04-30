#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>

namespace INI {

struct ValidationRule {
    std::string sectionName;
    std::string keyName;
    bool required = false;
    std::string defaultValue;
    
    enum class Type {
        Any,
        String,
        Integer,
        FloatingPoint,
        Boolean,
        Custom
    } type = Type::Any;
    
    std::function<bool(const std::string&)> customValidator;
    std::string errorMessage;
    
    int minInt = 0;
    int maxInt = 0;
    bool intRange = false;
    
    double minDouble = 0.0;
    double maxDouble = 0.0;
    bool doubleRange = false;
    
    size_t minLength = 0;
    size_t maxLength = 0;
    bool lengthRange = false;
    
    std::vector<std::string> allowedValues;
    bool enumCheck = false;
};

class IniValidator {
public:
    void addRule(const ValidationRule& rule);
    
    void addRequired(const std::string& sectionName, const std::string& keyName,
                     const std::string& defaultValue = "");
    
    void addIntegerRule(const std::string& sectionName, const std::string& keyName,
                        bool required = false, int min = 0, int max = 0,
                        bool useRange = false);
    
    void addDoubleRule(const std::string& sectionName, const std::string& keyName,
                       bool required = false, double min = 0.0, double max = 0.0,
                       bool useRange = false);
    
    void addBooleanRule(const std::string& sectionName, const std::string& keyName,
                        bool required = false);
    
    void addStringRule(const std::string& sectionName, const std::string& keyName,
                       bool required = false, size_t minLength = 0, size_t maxLength = 0,
                       bool useLength = false);
    
    void addEnumRule(const std::string& sectionName, const std::string& keyName,
                     const std::vector<std::string>& allowedValues, bool required = false);
    
    void addCustomRule(const std::string& sectionName, const std::string& keyName,
                       std::function<bool(const std::string&)> validator,
                       const std::string& errorMessage, bool required = false);
    
    bool validate(const std::map<std::string, std::map<std::string, std::string>>& data,
                  std::vector<std::string>& errors) const;
    
    bool validateValue(const std::string& sectionName, const std::string& keyName,
                       const std::string& value, std::string& error) const;
    
    void clearRules();
    
    const std::vector<ValidationRule>& getRules() const noexcept { return m_rules; }
    
private:
    std::vector<ValidationRule> m_rules;
    
    static bool validateRuleValue(const ValidationRule& rule, const std::string& value, std::string& error);
    
    static bool isInteger(const std::string& str);
    static bool isFloatingPoint(const std::string& str);
    static bool isBoolean(const std::string& str);
};

} // namespace INI
