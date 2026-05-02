#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <memory>
#include <istream>
#include <ostream>
#include <stdexcept>

namespace INI {

class JsonValue;
class JsonParser;

using JsonObject = std::map<std::string, std::shared_ptr<JsonValue>>;
using JsonArray = std::vector<std::shared_ptr<JsonValue>>;

enum class JsonType {
    Null,
    Boolean,
    Integer,
    Double,
    String,
    Object,
    Array
};

class JsonException : public std::runtime_error {
public:
    explicit JsonException(const std::string& message) : std::runtime_error(message) {}
};

class JsonParseException : public JsonException {
public:
    JsonParseException(size_t line, size_t column, const std::string& message)
        : JsonException("Line " + std::to_string(line) + ", Column " + std::to_string(column) + ": " + message),
          m_line(line), m_column(column) {}
    
    size_t getLine() const { return m_line; }
    size_t getColumn() const { return m_column; }
    
private:
    size_t m_line;
    size_t m_column;
};

class JsonValue {
public:
    JsonValue() : m_type(JsonType::Null) {}
    explicit JsonValue(bool value) : m_type(JsonType::Boolean), m_value(value) {}
    explicit JsonValue(int value) : m_type(JsonType::Integer), m_value(static_cast<long long>(value)) {}
    explicit JsonValue(long value) : m_type(JsonType::Integer), m_value(static_cast<long long>(value)) {}
    explicit JsonValue(long long value) : m_type(JsonType::Integer), m_value(value) {}
    explicit JsonValue(double value) : m_type(JsonType::Double), m_value(value) {}
    explicit JsonValue(const std::string& value) : m_type(JsonType::String), m_value(value) {}
    explicit JsonValue(const char* value) : m_type(JsonType::String), m_value(std::string(value)) {}
    explicit JsonValue(const JsonObject& value) : m_type(JsonType::Object), m_value(value) {}
    explicit JsonValue(const JsonArray& value) : m_type(JsonType::Array), m_value(value) {}
    
    JsonType getType() const { return m_type; }
    
    bool isNull() const { return m_type == JsonType::Null; }
    bool isBoolean() const { return m_type == JsonType::Boolean; }
    bool isInteger() const { return m_type == JsonType::Integer; }
    bool isDouble() const { return m_type == JsonType::Double; }
    bool isString() const { return m_type == JsonType::String; }
    bool isObject() const { return m_type == JsonType::Object; }
    bool isArray() const { return m_type == JsonType::Array; }
    bool isNumber() const { return m_type == JsonType::Integer || m_type == JsonType::Double; }
    
    bool getBoolean(bool defaultValue = false) const;
    long long getInteger(long long defaultValue = 0) const;
    double getDouble(double defaultValue = 0.0) const;
    std::string getString(const std::string& defaultValue = "") const;
    JsonObject getObject() const;
    JsonArray getArray() const;
    
    bool has(const std::string& key) const;
    std::shared_ptr<JsonValue> get(const std::string& key) const;
    std::shared_ptr<JsonValue> operator[](const std::string& key) const;
    
    void set(const std::string& key, const std::shared_ptr<JsonValue>& value);
    void set(const std::string& key, bool value);
    void set(const std::string& key, long long value);
    void set(const std::string& key, double value);
    void set(const std::string& key, const std::string& value);
    void set(const std::string& key, const char* value);
    void setNull(const std::string& key);
    
    bool remove(const std::string& key);
    
    size_t size() const;
    std::shared_ptr<JsonValue> get(size_t index) const;
    std::shared_ptr<JsonValue> operator[](size_t index) const;
    
    void add(const std::shared_ptr<JsonValue>& value);
    void add(bool value);
    void add(long long value);
    void add(double value);
    void add(const std::string& value);
    void add(const char* value);
    void addNull();
    
    std::string toString() const;
    void write(std::ostream& stream, int indent = 2) const;
    
    static std::shared_ptr<JsonValue> createNull();
    static std::shared_ptr<JsonValue> createBoolean(bool value);
    static std::shared_ptr<JsonValue> createInteger(long long value);
    static std::shared_ptr<JsonValue> createDouble(double value);
    static std::shared_ptr<JsonValue> createString(const std::string& value);
    static std::shared_ptr<JsonValue> createObject();
    static std::shared_ptr<JsonValue> createArray();
    
private:
    JsonType m_type;
    std::variant<
        std::nullptr_t,
        bool,
        long long,
        double,
        std::string,
        JsonObject,
        JsonArray
    > m_value;
    
    void writeInternal(std::ostream& stream, int indent, int currentIndent) const;
    static std::string escapeString(const std::string& str);
    
    friend class JsonParser;
};

struct JsonParseOptions {
    bool allowTrailingComma = false;
    bool allowSingleQuotes = false;
    bool allowUnquotedKeys = false;
};

struct JsonParseResult {
    std::shared_ptr<JsonValue> root;
    std::vector<std::string> warnings;
};

class JsonParser {
public:
    explicit JsonParser(const JsonParseOptions& options = JsonParseOptions());
    
    JsonParseResult parseFile(const std::string& filepath);
    JsonParseResult parseString(const std::string& content);
    JsonParseResult parseStream(std::istream& stream);
    
    static void write(const std::shared_ptr<JsonValue>& value, std::ostream& stream, int indent = 2);
    static std::string toString(const std::shared_ptr<JsonValue>& value, int indent = 2);
    
private:
    JsonParseOptions m_options;
    std::string m_content;
    size_t m_pos;
    size_t m_line;
    size_t m_column;
    
    void skipWhitespace();
    void skipComment();
    char peek() const;
    char get();
    void expect(char expected);
    
    std::shared_ptr<JsonValue> parseValue();
    std::shared_ptr<JsonValue> parseObject();
    std::shared_ptr<JsonValue> parseArray();
    std::shared_ptr<JsonValue> parseString();
    std::shared_ptr<JsonValue> parseNumber();
    std::shared_ptr<JsonValue> parseKeyword();
    
    JsonParseException createParseException(const std::string& message) const;
};

} // namespace INI
