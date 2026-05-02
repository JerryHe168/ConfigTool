#include "IniConfig.h"
#include "JsonParser.h"

#include <iostream>
#include <cassert>
#include <sstream>
#include <fstream>

using namespace INI;

void testJsonParserBasic()
{
    std::cout << "Testing basic JSON parsing..." << std::endl;
    
    JsonParser parser;
    
    std::string jsonStr = R"({
        "section1": {
            "key1": "value1",
            "key2": 123,
            "key3": 3.14159,
            "key4": true
        },
        "section2": {
            "key5": "hello world"
        }
    })";
    
    auto result = parser.parseString(jsonStr);
    assert(result.root != nullptr);
    assert(result.root->isObject());
    
    assert(result.root->has("section1"));
    auto section1 = result.root->get("section1");
    assert(section1 != nullptr);
    assert(section1->isObject());
    
    assert(section1->has("key1"));
    assert(section1->get("key1")->getString() == "value1");
    
    assert(section1->has("key2"));
    assert(section1->get("key2")->getInteger() == 123);
    
    assert(section1->has("key3"));
    assert(section1->get("key3")->getDouble() == 3.14159);
    
    assert(section1->has("key4"));
    assert(section1->get("key4")->getBoolean() == true);
    
    assert(result.root->has("section2"));
    auto section2 = result.root->get("section2");
    assert(section2->has("key5"));
    assert(section2->get("key5")->getString() == "hello world");
    
    std::cout << "  Basic JSON parsing: PASSED" << std::endl;
}

void testJsonParserTypes()
{
    std::cout << "Testing JSON type parsing..." << std::endl;
    
    JsonParser parser;
    
    std::string jsonStr = R"({
        "null_value": null,
        "bool_true": true,
        "bool_false": false,
        "int_pos": 42,
        "int_neg": -123,
        "double_val": 3.14159,
        "double_sci": 1.23e10,
        "empty_string": "",
        "special_string": "line1\nline2\twith\ttab"
    })";
    
    auto result = parser.parseString(jsonStr);
    assert(result.root != nullptr);
    
    assert(result.root->get("null_value")->isNull());
    assert(result.root->get("bool_true")->getBoolean() == true);
    assert(result.root->get("bool_false")->getBoolean() == false);
    assert(result.root->get("int_pos")->getInteger() == 42);
    assert(result.root->get("int_neg")->getInteger() == -123);
    assert(result.root->get("double_val")->getDouble() == 3.14159);
    
    std::cout << "  JSON type parsing: PASSED" << std::endl;
}

void testJsonParserArrays()
{
    std::cout << "Testing JSON array parsing..." << std::endl;
    
    JsonParser parser;
    
    std::string jsonStr = R"({
        "numbers": [1, 2, 3, 4, 5],
        "strings": ["a", "b", "c"],
        "mixed": [1, "two", 3.0, true],
        "empty": [],
        "nested": [[1, 2], [3, 4]]
    })";
    
    auto result = parser.parseString(jsonStr);
    assert(result.root != nullptr);
    
    auto numbers = result.root->get("numbers");
    assert(numbers->isArray());
    assert(numbers->size() == 5);
    assert(numbers->get(0)->getInteger() == 1);
    assert(numbers->get(4)->getInteger() == 5);
    
    auto strings = result.root->get("strings");
    assert(strings->isArray());
    assert(strings->size() == 3);
    assert(strings->get(1)->getString() == "b");
    
    auto empty = result.root->get("empty");
    assert(empty->isArray());
    assert(empty->size() == 0);
    
    std::cout << "  JSON array parsing: PASSED" << std::endl;
}

void testJsonValueSet()
{
    std::cout << "Testing JsonValue set operations..." << std::endl;
    
    auto obj = JsonValue::createObject();
    
    obj->set("str_key", std::string("hello"));
    obj->set("int_key", 42LL);
    obj->set("double_key", 3.14);
    obj->set("bool_key", true);
    obj->setNull("null_key");
    
    auto nested = JsonValue::createObject();
    nested->set("nested_key", std::string("nested_value"));
    obj->set("nested_obj", nested);
    
    assert(obj->get("str_key")->getString() == "hello");
    assert(obj->get("int_key")->getInteger() == 42);
    assert(obj->get("double_key")->getDouble() == 3.14);
    assert(obj->get("bool_key")->getBoolean() == true);
    assert(obj->get("null_key")->isNull());
    assert(obj->get("nested_obj")->get("nested_key")->getString() == "nested_value");
    
    auto arr = JsonValue::createArray();
    arr->add(std::string("first"));
    arr->add(100LL);
    arr->add(true);
    arr->addNull();
    
    assert(arr->size() == 4);
    assert(arr->get(0)->getString() == "first");
    assert(arr->get(1)->getInteger() == 100);
    assert(arr->get(2)->getBoolean() == true);
    assert(arr->get(3)->isNull());
    
    std::cout << "  JsonValue set operations: PASSED" << std::endl;
}

void testJsonWrite()
{
    std::cout << "Testing JSON write..." << std::endl;
    
    auto obj = JsonValue::createObject();
    obj->set("name", std::string("test"));
    obj->set("value", 123LL);
    obj->set("enabled", true);
    
    auto nested = JsonValue::createObject();
    nested->set("key", std::string("nested_value"));
    obj->set("nested", nested);
    
    auto arr = JsonValue::createArray();
    arr->add(1LL);
    arr->add(2LL);
    arr->add(3LL);
    obj->set("array", arr);
    
    std::ostringstream oss;
    JsonParser::write(obj, oss, 2);
    std::string jsonOutput = oss.str();
    
    assert(jsonOutput.find("\"name\"") != std::string::npos);
    assert(jsonOutput.find("\"test\"") != std::string::npos);
    assert(jsonOutput.find("\"value\"") != std::string::npos);
    assert(jsonOutput.find("123") != std::string::npos);
    assert(jsonOutput.find("\"enabled\"") != std::string::npos);
    assert(jsonOutput.find("true") != std::string::npos);
    
    std::cout << "  JSON write: PASSED" << std::endl;
}

void testIniToJson()
{
    std::cout << "Testing INI to JSON conversion..." << std::endl;
    
    IniConfig config;
    config.set("database", "host", "localhost");
    config.set("database", "port", "3306");
    config.set("database", "enabled", "true");
    config.set("database", "timeout", "30.5");
    
    config.set("app", "name", "MyApp");
    config.set("app", "version", "1.0.0");
    
    auto json = config.toJson();
    assert(json != nullptr);
    assert(json->isObject());
    
    assert(json->has("database"));
    auto db = json->get("database");
    assert(db->isObject());
    assert(db->get("host")->getString() == "localhost");
    assert(db->get("port")->getInteger() == 3306);
    assert(db->get("enabled")->getBoolean() == true);
    assert(db->get("timeout")->getDouble() == 30.5);
    
    assert(json->has("app"));
    auto app = json->get("app");
    assert(app->get("name")->getString() == "MyApp");
    
    std::cout << "  INI to JSON conversion: PASSED" << std::endl;
}

void testJsonToIni()
{
    std::cout << "Testing JSON to INI conversion..." << std::endl;
    
    JsonParser parser;
    std::string jsonStr = R"({
        "database": {
            "host": "localhost",
            "port": 3306,
            "enabled": true,
            "timeout": 30.5
        },
        "app": {
            "name": "MyApp",
            "debug": false
        }
    })";
    
    auto result = parser.parseString(jsonStr);
    
    IniConfig config;
    config.fromJson(result.root);
    
    assert(config.hasSection("database"));
    assert(config.get("database", "host", "") == "localhost");
    
    bool success = false;
    int port = config.getInt("database", "port", 0, &success);
    assert(success == true);
    assert(port == 3306);
    
    bool enabled = config.getBool("database", "enabled", false, &success);
    assert(success == true);
    assert(enabled == true);
    
    double timeout = config.getDouble("database", "timeout", 0.0, &success);
    assert(success == true);
    assert(timeout == 30.5);
    
    assert(config.hasSection("app"));
    assert(config.get("app", "name", "") == "MyApp");
    
    bool debug = config.getBool("app", "debug", true, &success);
    assert(success == true);
    assert(debug == false);
    
    std::cout << "  JSON to INI conversion: PASSED" << std::endl;
}

void testSaveLoadJson()
{
    std::cout << "Testing save and load JSON..." << std::endl;
    
    IniConfig original;
    original.set("section1", "key1", "value1");
    original.set("section1", "key2", "123");
    original.set("section1", "key3", "true");
    original.set("section2", "key4", "3.14");
    
    std::string jsonStr = original.saveToJsonString(2);
    assert(!jsonStr.empty());
    assert(jsonStr.find("\"section1\"") != std::string::npos);
    assert(jsonStr.find("\"section2\"") != std::string::npos);
    
    IniConfig loaded;
    bool loadSuccess = loaded.loadJsonString(jsonStr);
    assert(loadSuccess == true);
    
    assert(loaded.hasSection("section1"));
    assert(loaded.get("section1", "key1", "") == "value1");
    
    std::ostringstream oss;
    loaded.saveJsonStream(oss, 2);
    std::string savedStr = oss.str();
    assert(!savedStr.empty());
    
    std::cout << "  Save and load JSON: PASSED" << std::endl;
}

void testJsonRoundTrip()
{
    std::cout << "Testing JSON round-trip conversion..." << std::endl;
    
    IniConfig original;
    original.set("config", "string_val", "hello world");
    original.set("config", "int_val", "42");
    original.set("config", "double_val", "3.14159");
    original.set("config", "bool_true", "true");
    original.set("config", "bool_false", "false");
    
    std::string jsonStr = original.saveToJsonString(0);
    
    IniConfig restored;
    restored.loadJsonString(jsonStr);
    
    assert(restored.get("config", "string_val", "") == "hello world");
    
    bool success = false;
    assert(restored.getInt("config", "int_val", 0, &success) == 42);
    assert(success == true);
    
    assert(restored.getDouble("config", "double_val", 0.0, &success) == 3.14159);
    assert(success == true);
    
    assert(restored.getBool("config", "bool_true", false, &success) == true);
    assert(success == true);
    
    assert(restored.getBool("config", "bool_false", true, &success) == false);
    assert(success == true);
    
    std::cout << "  JSON round-trip: PASSED" << std::endl;
}

void testJsonExceptions()
{
    std::cout << "Testing JSON exception handling..." << std::endl;
    
    IniConfig config;
    
    bool success = config.loadJsonString("invalid json");
    assert(success == false);
    
    success = config.loadJsonString("{");
    assert(success == false);
    
    success = config.loadJsonString("{\"test\": 123}");
    assert(success == true);
    
    try {
        config.loadJsonStringOrThrow("invalid json");
        assert(false && "Should have thrown exception");
    } catch (const JsonException& e) {
        assert(std::string(e.what()).length() > 0);
    }
    
    std::cout << "  JSON exception handling: PASSED" << std::endl;
}

void testJsonParserEscape()
{
    std::cout << "Testing JSON escape sequences..." << std::endl;
    
    JsonParser parser;
    
    std::string jsonStr = R"({
        "escaped": "line1\nline2\ttabbed\"quoted\"",
        "path": "C:\\\\Users\\\\test",
        "unicode": "test\u0041\u0042"
    })";
    
    auto result = parser.parseString(jsonStr);
    assert(result.root != nullptr);
    
    auto escaped = result.root->get("escaped")->getString();
    assert(escaped.find('\n') != std::string::npos);
    assert(escaped.find('\t') != std::string::npos);
    assert(escaped.find('"') != std::string::npos);
    
    auto path = result.root->get("path")->getString();
    assert(path.find('\\') != std::string::npos);
    
    std::cout << "  JSON escape sequences: PASSED" << std::endl;
}

int main()
{
    std::cout << "=== JSON Tests ===" << std::endl << std::endl;
    
    try {
        testJsonParserBasic();
        testJsonParserTypes();
        testJsonParserArrays();
        testJsonValueSet();
        testJsonWrite();
        testIniToJson();
        testJsonToIni();
        testSaveLoadJson();
        testJsonRoundTrip();
        testJsonExceptions();
        testJsonParserEscape();
        
        std::cout << std::endl << "=== All JSON Tests PASSED ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << std::endl << "Test FAILED: " << e.what() << std::endl;
        return 1;
    }
}
