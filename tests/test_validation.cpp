#include <iostream>
#include <cassert>
#include <memory>
#include "IniConfig.h"
#include "IniValidator.h"

void testRequiredValidation()
{
    std::cout << "Testing required field validation..." << std::endl;
    
    INI::IniConfig config;
    auto validator = std::make_shared<INI::IniValidator>();
    
    validator->addRequired("section1", "key1");
    validator->addRequired("section1", "key2", "default_value");
    
    config.setValidator(validator);
    
    assert(!config.validate());
    assert(!config.getValidationErrors().empty());
    
    config.set("section1", "key1", "value1");
    
    assert(config.validate());
    assert(config.getValidationErrors().empty());
    
    std::cout << "  Required field validation: PASSED" << std::endl;
}

void testTypeValidation()
{
    std::cout << "Testing type validation..." << std::endl;
    
    INI::IniConfig config;
    auto validator = std::make_shared<INI::IniValidator>();
    
    validator->addIntegerRule("test", "int_val", false);
    validator->addDoubleRule("test", "double_val", false);
    validator->addBooleanRule("test", "bool_val", false);
    validator->addStringRule("test", "string_val", false);
    
    config.setValidator(validator);
    
    config.set("test", "int_val", "42");
    config.set("test", "double_val", "3.14");
    config.set("test", "bool_val", "true");
    config.set("test", "string_val", "hello");
    
    assert(config.validate());
    
    config.set("test", "int_val", "not_an_int");
    
    assert(!config.validate());
    assert(!config.getValidationErrors().empty());
    
    std::cout << "  Type validation: PASSED" << std::endl;
}

void testRangeValidation()
{
    std::cout << "Testing range validation..." << std::endl;
    
    INI::IniConfig config;
    auto validator = std::make_shared<INI::IniValidator>();
    
    validator->addIntegerRule("test", "int_range", false, 0, 100, true);
    validator->addDoubleRule("test", "double_range", false, 0.0, 1.0, true);
    validator->addStringRule("test", "string_len", false, 3, 10, true);
    
    config.setValidator(validator);
    
    config.set("test", "int_range", "50");
    config.set("test", "double_range", "0.5");
    config.set("test", "string_len", "hello");
    
    assert(config.validate());
    
    config.set("test", "int_range", "150");
    assert(!config.validate());
    
    config.set("test", "int_range", "50");
    config.set("test", "double_range", "2.0");
    assert(!config.validate());
    
    config.set("test", "double_range", "0.5");
    config.set("test", "string_len", "hi");
    assert(!config.validate());
    
    config.set("test", "string_len", "this_is_a_very_long_string");
    assert(!config.validate());
    
    std::cout << "  Range validation: PASSED" << std::endl;
}

void testEnumValidation()
{
    std::cout << "Testing enum validation..." << std::endl;
    
    INI::IniConfig config;
    auto validator = std::make_shared<INI::IniValidator>();
    
    std::vector<std::string> allowed = {"red", "green", "blue"};
    validator->addEnumRule("test", "color", allowed, false);
    
    config.setValidator(validator);
    
    config.set("test", "color", "red");
    assert(config.validate());
    
    config.set("test", "color", "green");
    assert(config.validate());
    
    config.set("test", "color", "yellow");
    assert(!config.validate());
    
    std::cout << "  Enum validation: PASSED" << std::endl;
}

void testCustomValidation()
{
    std::cout << "Testing custom validation..." << std::endl;
    
    INI::IniConfig config;
    auto validator = std::make_shared<INI::IniValidator>();
    
    validator->addCustomRule("test", "email",
        [](const std::string& val) {
            return val.find('@') != std::string::npos;
        },
        "Value must be a valid email address",
        false
    );
    
    config.setValidator(validator);
    
    config.set("test", "email", "user@example.com");
    assert(config.validate());
    
    config.set("test", "email", "invalid_email");
    assert(!config.validate());
    
    std::cout << "  Custom validation: PASSED" << std::endl;
}

void testValidationRuleDirect()
{
    std::cout << "Testing direct validation rule usage..." << std::endl;
    
    INI::IniValidator validator;
    
    INI::ValidationRule rule;
    rule.sectionName = "test";
    rule.keyName = "custom";
    rule.required = true;
    rule.type = INI::ValidationRule::Type::Integer;
    rule.minInt = 10;
    rule.maxInt = 20;
    rule.intRange = true;
    
    validator.addRule(rule);
    
    INI::IniConfig config;
    auto validatorPtr = std::make_shared<INI::IniValidator>(validator);
    config.setValidator(validatorPtr);
    
    config.set("test", "custom", "15");
    assert(config.validate());
    
    config.set("test", "custom", "5");
    assert(!config.validate());
    
    config.set("test", "custom", "25");
    assert(!config.validate());
    
    config.set("test", "custom", "not_an_int");
    assert(!config.validate());
    
    std::cout << "  Direct validation rule: PASSED" << std::endl;
}

void testValidateValue()
{
    std::cout << "Testing validateValue method..." << std::endl;
    
    INI::IniValidator validator;
    validator.addIntegerRule("section", "key", false, 0, 100, true);
    
    std::string error;
    
    assert(validator.validateValue("section", "key", "50", error));
    assert(error.empty());
    
    assert(!validator.validateValue("section", "key", "150", error));
    assert(!error.empty());
    
    assert(!validator.validateValue("section", "key", "not_int", error));
    assert(!error.empty());
    
    assert(validator.validateValue("nonexistent", "key", "anything", error));
    assert(error.empty());
    
    std::cout << "  validateValue method: PASSED" << std::endl;
}

void testClearRules()
{
    std::cout << "Testing clearRules method..." << std::endl;
    
    INI::IniValidator validator;
    validator.addRequired("section", "key");
    
    assert(!validator.getRules().empty());
    
    validator.clearRules();
    
    assert(validator.getRules().empty());
    
    std::cout << "  clearRules method: PASSED" << std::endl;
}

int main()
{
    std::cout << "=== Validation Tests ===" << std::endl << std::endl;
    
    testRequiredValidation();
    testTypeValidation();
    testRangeValidation();
    testEnumValidation();
    testCustomValidation();
    testValidationRuleDirect();
    testValidateValue();
    testClearRules();
    
    std::cout << std::endl << "=== All Validation Tests PASSED ===" << std::endl;
    
    return 0;
}
