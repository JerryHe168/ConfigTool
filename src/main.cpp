#include <iostream>
#include <memory>
#include "IniConfig.h"
#include "IniExceptions.h"

void printUsage()
{
    std::cout << "INI Configuration Manager" << std::endl;
    std::cout << "Usage: ConfigTool1 [options] <config_file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help     Show this help message" << std::endl;
    std::cout << "  -v, --version  Show version information" << std::endl;
    std::cout << "  -d, --demo     Run demo mode" << std::endl;
    std::cout << std::endl;
}

void printVersion()
{
    std::cout << "INI Configuration Manager v1.0.0" << std::endl;
    std::cout << "Cross-platform INI parser with advanced features" << std::endl;
}

void runDemo()
{
    std::cout << "=== INI Configuration Manager Demo ===" << std::endl << std::endl;
    
    std::cout << "1. Creating and populating a configuration..." << std::endl;
    INI::IniConfig config;
    
    config.set("section1", "key1", "value1");
    config.setInt("section1", "key2", 123);
    config.setDouble("section1", "key3", 3.14159);
    config.setBool("section1", "key4", true);
    config.set("section1", "key5", "quoted string");
    config.set("section1", "key6", "Hello\\nWorld");
    
    config.set("section2", "key", "multiple words");
    
    config.setInt("DEFAULT", "timeout", 30);
    
    std::cout << "   Configuration created successfully." << std::endl << std::endl;
    
    std::cout << "2. Reading configuration values..." << std::endl;
    std::cout << "   [section1].key1 = " << config.getString("section1", "key1") << std::endl;
    std::cout << "   [section1].key2 (int) = " << config.getInt("section1", "key2") << std::endl;
    std::cout << "   [section1].key3 (double) = " << config.getDouble("section1", "key3") << std::endl;
    std::cout << "   [section1].key4 (bool) = " << (config.getBool("section1", "key4") ? "true" : "false") << std::endl;
    std::cout << "   [section1].key5 = " << config.getString("section1", "key5") << std::endl;
    std::cout << "   [DEFAULT].timeout = " << config.getInt("DEFAULT", "timeout") << std::endl;
    std::cout << std::endl;
    
    std::cout << "3. Configuration as INI string:" << std::endl;
    std::cout << config.saveToString() << std::endl;
    
    std::cout << "4. Testing section and key existence..." << std::endl;
    std::cout << "   Has section 'section1'? " << (config.hasSection("section1") ? "Yes" : "No") << std::endl;
    std::cout << "   Has section 'nonexistent'? " << (config.hasSection("nonexistent") ? "Yes" : "No") << std::endl;
    std::cout << "   Has key 'section1.key1'? " << (config.hasKey("section1", "key1") ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
    
    std::cout << "5. Listing all sections:" << std::endl;
    for (const auto& section : config.getSectionNames())
    {
        std::cout << "   [" << section << "]" << std::endl;
        for (const auto& key : config.getKeys(section))
        {
            std::cout << "      " << key << " = " << config.getString(section, key) << std::endl;
        }
    }
    std::cout << std::endl;
    
    std::cout << "6. Testing validation rules..." << std::endl;
    auto validator = std::make_shared<INI::IniValidator>();
    
    validator->addIntegerRule("section1", "key2", true, 0, 1000, true);
    validator->addBooleanRule("section1", "key4", true);
    validator->addEnumRule("section2", "status", {"active", "inactive", "pending"}, false);
    
    config.setValidator(validator);
    
    if (config.validate())
    {
        std::cout << "   Validation passed!" << std::endl;
    }
    else
    {
        std::cout << "   Validation failed:" << std::endl;
        for (const auto& error : config.getValidationErrors())
        {
            std::cout << "      - " << error << std::endl;
        }
    }
    std::cout << std::endl;
    
    std::cout << "7. Testing default values for missing keys..." << std::endl;
    std::cout << "   [section1].nonexistent = '" 
              << config.getString("section1", "nonexistent", "default_value") 
              << "'" << std::endl;
    std::cout << "   [nonexistent].key = " 
              << config.getInt("nonexistent", "key", 999) 
              << std::endl;
    std::cout << std::endl;
    
    std::cout << "8. Testing removal operations..." << std::endl;
    std::cout << "   Removing [section1].key1..." << std::endl;
    config.removeKey("section1", "key1");
    std::cout << "   Has [section1].key1? " << (config.hasKey("section1", "key1") ? "Yes" : "No") << std::endl;
    
    std::cout << "   Removing [section2]..." << std::endl;
    config.removeSection("section2");
    std::cout << "   Has [section2]? " << (config.hasSection("section2") ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== Demo Complete ===" << std::endl;
}

int main(int argc, char* argv[])
{
    std::cout << std::boolalpha;
    
    if (argc < 2)
    {
        runDemo();
        return 0;
    }
    
    std::string arg1 = argv[1];
    
    if (arg1 == "-h" || arg1 == "--help")
    {
        printUsage();
        return 0;
    }
    
    if (arg1 == "-v" || arg1 == "--version")
    {
        printVersion();
        return 0;
    }
    
    if (arg1 == "-d" || arg1 == "--demo")
    {
        runDemo();
        return 0;
    }
    
    std::string configFile = arg1;
    
    try
    {
        std::cout << "Loading configuration from: " << configFile << std::endl;
        
        INI::IniConfig config;
        
        if (!config.loadFile(configFile))
        {
            std::cerr << "Error: Failed to load configuration file: " << configFile << std::endl;
            return 1;
        }
        
        std::cout << "Configuration loaded successfully!" << std::endl << std::endl;
        
        std::cout << "=== Configuration Content ===" << std::endl;
        for (const auto& section : config.getSectionNames())
        {
            std::cout << "[" << section << "]" << std::endl;
            for (const auto& key : config.getKeys(section))
            {
                std::cout << "  " << key << " = " << config.getString(section, key) << std::endl;
            }
            std::cout << std::endl;
        }
        
        if (!config.getIncludedFiles().empty())
        {
            std::cout << "=== Included Files ===" << std::endl;
            for (const auto& file : config.getIncludedFiles())
            {
                std::cout << "  - " << file << std::endl;
            }
            std::cout << std::endl;
        }
        
        std::cout << "=== Parsed Values (with type conversion) ===" << std::endl;
        for (const auto& section : config.getSectionNames())
        {
            for (const auto& key : config.getKeys(section))
            {
                std::string value = config.getString(section, key);
                
                std::cout << "[" << section << "]." << key << ": " << value;
                
                bool success = false;
                int intVal = config.getInt(section, key, 0, &success);
                if (success)
                {
                    std::cout << " (int: " << intVal << ")";
                }
                
                double doubleVal = config.getDouble(section, key, 0.0, &success);
                if (success)
                {
                    std::cout << " (double: " << doubleVal << ")";
                }
                
                bool boolVal = config.getBool(section, key, false, &success);
                if (success)
                {
                    std::cout << " (bool: " << boolVal << ")";
                }
                
                std::cout << std::endl;
            }
        }
    }
    catch (const INI::ParseException& e)
    {
        std::cerr << "Parse error at line " << e.getLineNumber() << ": " << e.what() << std::endl;
        return 1;
    }
    catch (const INI::FileException& e)
    {
        std::cerr << "File error: " << e.what() << std::endl;
        return 1;
    }
    catch (const INI::IniException& e)
    {
        std::cerr << "Configuration error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
