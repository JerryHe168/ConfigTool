#include "IniValidator.h"
#include "IniExceptions.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <limits>

namespace INI {

void IniValidator::addRule(const ValidationRule& rule)
{
    m_rules.push_back(rule);
}

void IniValidator::addRequired(const std::string& sectionName, const std::string& keyName,
                                const std::string& defaultValue)
{
    ValidationRule rule;
    rule.sectionName = sectionName;
    rule.keyName = keyName;
    rule.required = true;
    rule.defaultValue = defaultValue;
    rule.type = ValidationRule::Type::Any;
    rule.intRange = false;
    rule.doubleRange = false;
    rule.lengthRange = false;
    rule.enumCheck = false;
    addRule(rule);
}

void IniValidator::addIntegerRule(const std::string& sectionName, const std::string& keyName,
                                   bool required, int min, int max, bool useRange)
{
    ValidationRule rule;
    rule.sectionName = sectionName;
    rule.keyName = keyName;
    rule.required = required;
    rule.type = ValidationRule::Type::Integer;
    rule.minInt = min;
    rule.maxInt = max;
    rule.intRange = useRange;
    addRule(rule);
}

void IniValidator::addDoubleRule(const std::string& sectionName, const std::string& keyName,
                                  bool required, double min, double max, bool useRange)
{
    ValidationRule rule;
    rule.sectionName = sectionName;
    rule.keyName = keyName;
    rule.required = required;
    rule.type = ValidationRule::Type::FloatingPoint;
    rule.minDouble = min;
    rule.maxDouble = max;
    rule.doubleRange = useRange;
    addRule(rule);
}

void IniValidator::addBooleanRule(const std::string& sectionName, const std::string& keyName,
                                   bool required)
{
    ValidationRule rule;
    rule.sectionName = sectionName;
    rule.keyName = keyName;
    rule.required = required;
    rule.type = ValidationRule::Type::Boolean;
    addRule(rule);
}

void IniValidator::addStringRule(const std::string& sectionName, const std::string& keyName,
                                  bool required, size_t minLength, size_t maxLength, bool useLength)
{
    ValidationRule rule;
    rule.sectionName = sectionName;
    rule.keyName = keyName;
    rule.required = required;
    rule.type = ValidationRule::Type::String;
    rule.minLength = minLength;
    rule.maxLength = maxLength;
    rule.lengthRange = useLength;
    addRule(rule);
}

void IniValidator::addEnumRule(const std::string& sectionName, const std::string& keyName,
                                const std::vector<std::string>& allowedValues, bool required)
{
    ValidationRule rule;
    rule.sectionName = sectionName;
    rule.keyName = keyName;
    rule.required = required;
    rule.allowedValues = allowedValues;
    rule.enumCheck = true;
    addRule(rule);
}

void IniValidator::addCustomRule(const std::string& sectionName, const std::string& keyName,
                                  std::function<bool(const std::string&)> validator,
                                  const std::string& errorMessage, bool required)
{
    ValidationRule rule;
    rule.sectionName = sectionName;
    rule.keyName = keyName;
    rule.required = required;
    rule.type = ValidationRule::Type::Custom;
    rule.customValidator = validator;
    rule.errorMessage = errorMessage;
    addRule(rule);
}

bool IniValidator::isInteger(const std::string& str)
{
    if (str.empty()) {
        return false;
    }
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') {
        start = 1;
    }
    
    if (start >= str.size()) {
        return false;
    }
    
    for (size_t i = start; i < str.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(str[i]))) {
            return false;
        }
    }
    
    return true;
}

bool IniValidator::isFloatingPoint(const std::string& str)
{
    if (str.empty()) {
        return false;
    }
    
    bool hasDot = false;
    bool hasExp = false;
    size_t start = 0;
    
    if (str[0] == '-' || str[0] == '+') {
        start = 1;
    }
    
    for (size_t i = start; i < str.size(); ++i) {
        char c = str[i];
        
        if (std::isdigit(static_cast<unsigned char>(c))) {
            continue;
        } else if (c == '.') {
            if (hasDot || hasExp) {
                return false;
            }
            hasDot = true;
        } else if (c == 'e' || c == 'E') {
            if (hasExp) {
                return false;
            }
            hasExp = true;
            
            if (i + 1 < str.size() && (str[i + 1] == '+' || str[i + 1] == '-')) {
                ++i;
            }
        } else {
            return false;
        }
    }
    
    return true;
}

bool IniValidator::isBoolean(const std::string& str)
{
    std::string lower;
    for (char c : str) {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    
    return lower == "true" || lower == "false" ||
           lower == "yes" || lower == "no" ||
           lower == "1" || lower == "0" ||
           lower == "on" || lower == "off";
}

bool IniValidator::validateRuleValue(const ValidationRule& rule, const std::string& value, std::string& error)
{
    switch (rule.type) {
        case ValidationRule::Type::Integer:
            if (!isInteger(value)) {
                error = "Value '" + value + "' is not a valid integer";
                return false;
            }
            if (rule.intRange) {
                try {
                    int intValue = std::stoi(value);
                    if (intValue < rule.minInt || intValue > rule.maxInt) {
                        std::ostringstream oss;
                        oss << "Integer value " << intValue << " is out of range [" 
                            << rule.minInt << ", " << rule.maxInt << "]";
                        error = oss.str();
                        return false;
                    }
                } catch (...) {
                    error = "Value '" + value + "' cannot be converted to integer";
                    return false;
                }
            }
            break;
            
        case ValidationRule::Type::FloatingPoint:
            if (!isFloatingPoint(value)) {
                error = "Value '" + value + "' is not a valid floating-point number";
                return false;
            }
            if (rule.doubleRange) {
                try {
                    double doubleValue = std::stod(value);
                    if (doubleValue < rule.minDouble || doubleValue > rule.maxDouble) {
                        std::ostringstream oss;
                        oss << "Floating-point value " << doubleValue << " is out of range [" 
                            << rule.minDouble << ", " << rule.maxDouble << "]";
                        error = oss.str();
                        return false;
                    }
                } catch (...) {
                    error = "Value '" + value + "' cannot be converted to floating-point";
                    return false;
                }
            }
            break;
            
        case ValidationRule::Type::Boolean:
            if (!isBoolean(value)) {
                error = "Value '" + value + "' is not a valid boolean (try: true/false, yes/no, 1/0, on/off)";
                return false;
            }
            break;
            
        case ValidationRule::Type::String:
            if (rule.lengthRange) {
                if (value.length() < rule.minLength) {
                    error = "String length " + std::to_string(value.length()) + 
                            " is less than minimum " + std::to_string(rule.minLength);
                    return false;
                }
                if (value.length() > rule.maxLength) {
                    error = "String length " + std::to_string(value.length()) + 
                            " is greater than maximum " + std::to_string(rule.maxLength);
                    return false;
                }
            }
            break;
            
        case ValidationRule::Type::Custom:
            if (rule.customValidator) {
                try {
                    if (!rule.customValidator(value)) {
                        error = rule.errorMessage.empty() ? 
                            "Custom validation failed for value '" + value + "'" :
                            rule.errorMessage;
                        return false;
                    }
                } catch (const std::exception& e) {
                    error = "Custom validator threw exception: " + std::string(e.what());
                    return false;
                }
            }
            break;
            
        case ValidationRule::Type::Any:
        default:
            break;
    }
    
    if (rule.enumCheck) {
        bool found = false;
        for (const auto& allowed : rule.allowedValues) {
            if (allowed == value) {
                found = true;
                break;
            }
        }
        if (!found) {
            std::ostringstream oss;
            oss << "Value '" << value << "' is not in allowed set: [";
            for (size_t i = 0; i < rule.allowedValues.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << "'" << rule.allowedValues[i] << "'";
            }
            oss << "]";
            error = oss.str();
            return false;
        }
    }
    
    return true;
}

bool IniValidator::validateValue(const std::string& sectionName, const std::string& keyName,
                                  const std::string& value, std::string& error) const
{
    error.clear();
    for (const auto& rule : m_rules) {
        if (rule.sectionName == sectionName && rule.keyName == keyName) {
            return validateRuleValue(rule, value, error);
        }
    }
    return true;
}

bool IniValidator::validate(const std::map<std::string, std::map<std::string, std::string>>& data,
                             std::vector<std::string>& errors) const
{
    errors.clear();
    
    for (const auto& rule : m_rules) {
        auto sectionIt = data.find(rule.sectionName);
        bool hasKey = false;
        std::string value;
        
        if (sectionIt != data.end()) {
            auto keyIt = sectionIt->second.find(rule.keyName);
            if (keyIt != sectionIt->second.end()) {
                hasKey = true;
                value = keyIt->second;
            }
        }
        
        if (rule.required) {
            if (!hasKey && rule.defaultValue.empty()) {
                if (sectionIt == data.end()) {
                    errors.push_back("Required section '" + rule.sectionName + "' not found");
                } else {
                    errors.push_back("Required key '" + rule.keyName + "' not found in section '" + 
                                    rule.sectionName + "'");
                }
                continue;
            }
        }
        
        if (hasKey) {
            std::string error;
            if (!validateRuleValue(rule, value, error)) {
                errors.push_back("[" + rule.sectionName + "]." + rule.keyName + ": " + error);
            }
        }
    }
    
    return errors.empty();
}

void IniValidator::clearRules()
{
    m_rules.clear();
}

} // namespace INI
