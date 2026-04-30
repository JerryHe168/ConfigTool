#include <iostream>
#include <cassert>
#include "IniConfig.h"

void testBooleanConversion()
{
    std::cout << "Testing boolean type conversion..." << std::endl;
    
    INI::IniConfig config;
    
    std::string iniContent = R"(
[booleans]
true_val = true
false_val = false
yes_val = yes
no_val = no
on_val = on
off_val = off
one_val = 1
zero_val = 0
mixed_case = TrUe
)";
    
    assert(config.loadString(iniContent));
    
    assert(config.getBool("booleans", "true_val") == true);
    assert(config.getBool("booleans", "false_val") == false);
    assert(config.getBool("booleans", "yes_val") == true);
    assert(config.getBool("booleans", "no_val") == false);
    assert(config.getBool("booleans", "on_val") == true);
    assert(config.getBool("booleans", "off_val") == false);
    assert(config.getBool("booleans", "one_val") == true);
    assert(config.getBool("booleans", "zero_val") == false);
    assert(config.getBool("booleans", "mixed_case") == true);
    
    bool success = false;
    config.getBool("booleans", "true_val", false, &success);
    assert(success == true);
    
    config.getBool("booleans", "invalid", false, &success);
    assert(success == false);
    
    config.set("test", "invalid_bool", "not_a_bool");
    assert(config.getBool("test", "invalid_bool", true) == true);
    
    config.setBool("test", "new_true", true);
    config.setBool("test", "new_false", false);
    assert(config.getString("test", "new_true") == "true");
    assert(config.getString("test", "new_false") == "false");
    
    std::cout << "  Boolean conversion: PASSED" << std::endl;
}

void testIntegerConversion()
{
    std::cout << "Testing integer type conversion..." << std::endl;
    
    INI::IniConfig config;
    
    std::string iniContent = R"(
[integers]
positive = 42
negative = -123
zero = 0
with_plus = +456
large = 2147483647
)";
    
    assert(config.loadString(iniContent));
    
    assert(config.getInt("integers", "positive") == 42);
    assert(config.getInt("integers", "negative") == -123);
    assert(config.getInt("integers", "zero") == 0);
    assert(config.getInt("integers", "with_plus") == 456);
    
    bool success = false;
    config.getInt("integers", "positive", 0, &success);
    assert(success == true);
    
    config.set("test", "not_int", "abc");
    config.getInt("test", "not_int", 999, &success);
    assert(success == false);
    
    config.setInt("test", "new_int", 12345);
    assert(config.getString("test", "new_int") == "12345");
    
    std::cout << "  Integer conversion: PASSED" << std::endl;
}

void testLongConversion()
{
    std::cout << "Testing long integer type conversion..." << std::endl;
    
    INI::IniConfig config;
    
    config.setLong("test", "small", 42L);
    config.setLong("test", "large", 9223372036854775807LL);
    config.setLong("test", "negative", -123456789L);
    
    assert(config.getLong("test", "small") == 42L);
    assert(config.getLong("test", "negative") == -123456789L);
    
    bool success = false;
    config.getLong("test", "small", 0L, &success);
    assert(success == true);
    
    config.set("test", "not_long", "xyz");
    config.getLong("test", "not_long", 999L, &success);
    assert(success == false);
    
    std::cout << "  Long conversion: PASSED" << std::endl;
}

void testDoubleConversion()
{
    std::cout << "Testing floating-point type conversion..." << std::endl;
    
    INI::IniConfig config;
    
    std::string iniContent = R"(
[floats]
simple = 3.14159
negative = -2.718
integer_like = 42.0
scientific = 1.5e3
with_plus = +1.23
)";
    
    assert(config.loadString(iniContent));
    
    bool closeEnough;
    
    closeEnough = (config.getDouble("floats", "simple") - 3.14159) < 0.0001;
    assert(closeEnough);
    
    closeEnough = (config.getDouble("floats", "negative") - (-2.718)) < 0.0001;
    assert(closeEnough);
    
    closeEnough = (config.getDouble("floats", "integer_like") - 42.0) < 0.0001;
    assert(closeEnough);
    
    closeEnough = (config.getDouble("floats", "scientific") - 1500.0) < 0.0001;
    assert(closeEnough);
    
    closeEnough = (config.getDouble("floats", "with_plus") - 1.23) < 0.0001;
    assert(closeEnough);
    
    bool success = false;
    config.getDouble("floats", "simple", 0.0, &success);
    assert(success == true);
    
    config.set("test", "not_double", "abc");
    config.getDouble("test", "not_double", 999.9, &success);
    assert(success == false);
    
    config.setDouble("test", "new_double", 0.12345);
    
    std::cout << "  Floating-point conversion: PASSED" << std::endl;
}

void testEscapeSequences()
{
    std::cout << "Testing escape sequences..." << std::endl;
    
    INI::IniConfig config;
    
    std::string iniContent = R"(
[escapes]
newline = Hello\nWorld
tab = Hello\tWorld
backslash = path\\to\\file
semicolon = value\;comment
)";
    
    assert(config.loadString(iniContent));
    
    std::string newlineVal = config.getString("escapes", "newline");
    assert(newlineVal.find('\n') != std::string::npos);
    
    std::string tabVal = config.getString("escapes", "tab");
    assert(tabVal.find('\t') != std::string::npos);
    
    std::string backslashVal = config.getString("escapes", "backslash");
    assert(backslashVal == "path\\to\\file");
    
    std::string semicolonVal = config.getString("escapes", "semicolon");
    assert(semicolonVal == "value;comment");
    
    std::cout << "  Escape sequences: PASSED" << std::endl;
}

void testSuccessFlag()
{
    std::cout << "Testing success flag parameter..." << std::endl;
    
    INI::IniConfig config;
    
    config.set("test", "int_val", "42");
    config.set("test", "invalid", "not_a_number");
    
    bool success = false;
    
    int intVal = config.getInt("test", "int_val", 0, &success);
    assert(success == true);
    assert(intVal == 42);
    
    int invalidVal = config.getInt("test", "invalid", 999, &success);
    assert(success == false);
    assert(invalidVal == 999);
    
    int missingVal = config.getInt("test", "missing", 111, &success);
    assert(success == false);
    assert(missingVal == 111);
    
    config.setBool("test", "bool_val", true);
    bool boolVal = config.getBool("test", "bool_val", false, &success);
    assert(success == true);
    assert(boolVal == true);
    
    config.setDouble("test", "double_val", 3.14);
    double doubleVal = config.getDouble("test", "double_val", 0.0, &success);
    assert(success == true);
    
    std::cout << "  Success flag: PASSED" << std::endl;
}

int main()
{
    std::cout << "=== Type Conversion Tests ===" << std::endl << std::endl;
    
    testBooleanConversion();
    testIntegerConversion();
    testLongConversion();
    testDoubleConversion();
    testEscapeSequences();
    testSuccessFlag();
    
    std::cout << std::endl << "=== All Type Conversion Tests PASSED ===" << std::endl;
    
    return 0;
}
