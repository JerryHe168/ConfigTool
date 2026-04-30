#include <iostream>
#include <cassert>
#include "IniConfig.h"

void testBasicParsing()
{
    std::cout << "Testing basic INI parsing..." << std::endl;
    
    INI::IniConfig config;
    
    std::string iniContent = R"(
[section1]
key1 = value1
key2=value2
  key3  =  value3  

[section2]
; This is a comment
# Another comment
key = test value
)";
    
    assert(config.loadString(iniContent));
    
    assert(config.hasSection("section1"));
    assert(config.hasSection("section2"));
    assert(!config.hasSection("nonexistent"));
    
    assert(config.hasKey("section1", "key1"));
    assert(config.hasKey("section1", "key2"));
    assert(config.hasKey("section1", "key3"));
    assert(!config.hasKey("section1", "nonexistent"));
    
    assert(config.getString("section1", "key1") == "value1");
    assert(config.getString("section1", "key2") == "value2");
    assert(config.getString("section1", "key3") == "value3");
    assert(config.getString("section2", "key") == "test value");
    
    std::cout << "  Basic parsing: PASSED" << std::endl;
}

void testSetAndSave()
{
    std::cout << "Testing set and save operations..." << std::endl;
    
    INI::IniConfig config;
    
    config.set("test", "string_key", "test value");
    config.setInt("test", "int_key", 42);
    config.setDouble("test", "double_key", 3.14159);
    config.setBool("test", "bool_key", true);
    config.setLong("test", "long_key", 123456789L);
    
    assert(config.hasSection("test"));
    assert(config.hasKey("test", "string_key"));
    assert(config.hasKey("test", "int_key"));
    assert(config.hasKey("test", "double_key"));
    assert(config.hasKey("test", "bool_key"));
    assert(config.hasKey("test", "long_key"));
    
    assert(config.getString("test", "string_key") == "test value");
    assert(config.getInt("test", "int_key") == 42);
    
    bool doubleMatch = (config.getDouble("test", "double_key") - 3.14159) < 0.0001;
    assert(doubleMatch);
    
    assert(config.getBool("test", "bool_key") == true);
    assert(config.getLong("test", "long_key") == 123456789L);
    
    std::string saved = config.saveToString();
    assert(!saved.empty());
    
    INI::IniConfig reloaded;
    assert(reloaded.loadString(saved));
    
    assert(reloaded.getString("test", "string_key") == "test value");
    assert(reloaded.getInt("test", "int_key") == 42);
    
    std::cout << "  Set and save: PASSED" << std::endl;
}

void testRemoval()
{
    std::cout << "Testing removal operations..." << std::endl;
    
    INI::IniConfig config;
    
    config.set("section1", "key1", "value1");
    config.set("section1", "key2", "value2");
    config.set("section2", "key1", "value1");
    
    assert(config.hasSection("section1"));
    assert(config.hasKey("section1", "key1"));
    
    assert(config.removeKey("section1", "key1"));
    assert(!config.hasKey("section1", "key1"));
    assert(config.hasKey("section1", "key2"));
    assert(!config.removeKey("section1", "nonexistent"));
    
    assert(config.removeSection("section1"));
    assert(!config.hasSection("section1"));
    assert(config.hasSection("section2"));
    assert(!config.removeSection("nonexistent"));
    
    std::cout << "  Removal operations: PASSED" << std::endl;
}

void testDefaultValues()
{
    std::cout << "Testing default values..." << std::endl;
    
    INI::IniConfig config;
    
    assert(config.getString("nonexistent", "key", "default") == "default");
    assert(config.getInt("nonexistent", "key", 999) == 999);
    assert(config.getDouble("nonexistent", "key", 3.14) == 3.14);
    assert(config.getBool("nonexistent", "key", true) == true);
    assert(config.getLong("nonexistent", "key", 12345L) == 12345L);
    
    config.set("test", "key", "not_a_number");
    assert(config.getInt("test", "key", 999) == 999);
    assert(config.getDouble("test", "key", 3.14) == 3.14);
    assert(config.getBool("test", "key", true) == true);
    
    std::cout << "  Default values: PASSED" << std::endl;
}

void testQuotedValues()
{
    std::cout << "Testing quoted values..." << std::endl;
    
    INI::IniConfig config;
    
    std::string iniContent = R"(
[test]
quoted = "quoted value with spaces"
single = no quotes
mixed = "start" end
)";
    
    assert(config.loadString(iniContent));
    
    assert(config.getString("test", "quoted") == "quoted value with spaces");
    assert(config.getString("test", "single") == "no quotes");
    
    std::cout << "  Quoted values: PASSED" << std::endl;
}

void testEmptyValues()
{
    std::cout << "Testing empty values..." << std::endl;
    
    INI::IniConfig config;
    
    std::string iniContent = R"(
[test]
empty_key =
key_without_equals
)";
    
    assert(config.loadString(iniContent));
    
    assert(config.getString("test", "empty_key") == "");
    assert(config.getString("test", "key_without_equals") == "");
    
    std::cout << "  Empty values: PASSED" << std::endl;
}

int main()
{
    std::cout << "=== Basic Tests ===" << std::endl << std::endl;
    
    testBasicParsing();
    testSetAndSave();
    testRemoval();
    testDefaultValues();
    testQuotedValues();
    testEmptyValues();
    
    std::cout << std::endl << "=== All Basic Tests PASSED ===" << std::endl;
    
    return 0;
}
