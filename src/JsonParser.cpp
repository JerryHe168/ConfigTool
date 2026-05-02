#include "JsonParser.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <iomanip>
#include <limits>
#include <charconv>
#include <system_error>

namespace INI {

bool JsonValue::getBoolean(bool defaultValue) const
{
    if (m_type == JsonType::Boolean) {
        return std::get<bool>(m_value);
    }
    return defaultValue;
}

long long JsonValue::getInteger(long long defaultValue) const
{
    if (m_type == JsonType::Integer) {
        return std::get<long long>(m_value);
    }
    if (m_type == JsonType::Double) {
        return static_cast<long long>(std::get<double>(m_value));
    }
    return defaultValue;
}

double JsonValue::getDouble(double defaultValue) const
{
    if (m_type == JsonType::Double) {
        return std::get<double>(m_value);
    }
    if (m_type == JsonType::Integer) {
        return static_cast<double>(std::get<long long>(m_value));
    }
    return defaultValue;
}

std::string JsonValue::getString(const std::string& defaultValue) const
{
    if (m_type == JsonType::String) {
        return std::get<std::string>(m_value);
    }
    return defaultValue;
}

JsonObject JsonValue::getObject() const
{
    if (m_type == JsonType::Object) {
        return std::get<JsonObject>(m_value);
    }
    return JsonObject();
}

JsonArray JsonValue::getArray() const
{
    if (m_type == JsonType::Array) {
        return std::get<JsonArray>(m_value);
    }
    return JsonArray();
}

bool JsonValue::has(const std::string& key) const
{
    if (m_type != JsonType::Object) {
        return false;
    }
    const auto& obj = std::get<JsonObject>(m_value);
    return obj.find(key) != obj.end();
}

std::shared_ptr<JsonValue> JsonValue::get(const std::string& key) const
{
    if (m_type != JsonType::Object) {
        return nullptr;
    }
    const auto& obj = std::get<JsonObject>(m_value);
    auto it = obj.find(key);
    if (it != obj.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<JsonValue> JsonValue::operator[](const std::string& key) const
{
    return get(key);
}

size_t JsonValue::size() const
{
    if (m_type == JsonType::Array) {
        return std::get<JsonArray>(m_value).size();
    }
    if (m_type == JsonType::Object) {
        return std::get<JsonObject>(m_value).size();
    }
    return 0;
}

std::shared_ptr<JsonValue> JsonValue::get(size_t index) const
{
    if (m_type != JsonType::Array) {
        return nullptr;
    }
    const auto& arr = std::get<JsonArray>(m_value);
    if (index >= arr.size()) {
        return nullptr;
    }
    return arr[index];
}

std::shared_ptr<JsonValue> JsonValue::operator[](size_t index) const
{
    return get(index);
}

std::string JsonValue::escapeString(const std::string& str)
{
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '\"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            default:
                if (std::iscntrl(static_cast<unsigned char>(c))) {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') 
                        << static_cast<int>(static_cast<unsigned char>(c));
                } else {
                    oss << c;
                }
                break;
        }
    }
    return oss.str();
}

void JsonValue::writeInternal(std::ostream& stream, int indent, int currentIndent) const
{
    switch (m_type) {
        case JsonType::Null:
            stream << "null";
            break;
        case JsonType::Boolean:
            stream << (std::get<bool>(m_value) ? "true" : "false");
            break;
        case JsonType::Integer:
            stream << std::get<long long>(m_value);
            break;
        case JsonType::Double: {
            double val = std::get<double>(m_value);
            if (val != val) {
                stream << "null";
            } else if (val == std::numeric_limits<double>::infinity()) {
                stream << "1e1000";
            } else if (val == -std::numeric_limits<double>::infinity()) {
                stream << "-1e1000";
            } else {
                char buf[64];
                auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), val,
                    std::chars_format::general, std::numeric_limits<double>::max_digits10);
                if (ec == std::errc()) {
                    stream.write(buf, ptr - buf);
                } else {
                    stream << std::setprecision(std::numeric_limits<double>::max_digits10) << val;
                }
            }
            break;
        }
        case JsonType::String:
            stream << "\"" << escapeString(std::get<std::string>(m_value)) << "\"";
            break;
        case JsonType::Object: {
            const auto& obj = std::get<JsonObject>(m_value);
            stream << "{";
            if (indent > 0) {
                stream << std::endl;
            }
            bool first = true;
            for (const auto& [key, value] : obj) {
                if (!first) {
                    stream << ",";
                    if (indent > 0) {
                        stream << std::endl;
                    }
                }
                first = false;
                if (indent > 0) {
                    stream << std::string(currentIndent + indent, ' ');
                }
                stream << "\"" << escapeString(key) << "\":";
                if (indent > 0) {
                    stream << " ";
                }
                if (value) {
                    value->writeInternal(stream, indent, currentIndent + indent);
                } else {
                    stream << "null";
                }
            }
            if (indent > 0 && !obj.empty()) {
                stream << std::endl << std::string(currentIndent, ' ');
            }
            stream << "}";
            break;
        }
        case JsonType::Array: {
            const auto& arr = std::get<JsonArray>(m_value);
            stream << "[";
            if (indent > 0) {
                stream << std::endl;
            }
            bool first = true;
            for (const auto& value : arr) {
                if (!first) {
                    stream << ",";
                    if (indent > 0) {
                        stream << std::endl;
                    }
                }
                first = false;
                if (indent > 0) {
                    stream << std::string(currentIndent + indent, ' ');
                }
                if (value) {
                    value->writeInternal(stream, indent, currentIndent + indent);
                } else {
                    stream << "null";
                }
            }
            if (indent > 0 && !arr.empty()) {
                stream << std::endl << std::string(currentIndent, ' ');
            }
            stream << "]";
            break;
        }
    }
}

void JsonValue::write(std::ostream& stream, int indent) const
{
    writeInternal(stream, indent, 0);
    if (indent > 0) {
        stream << std::endl;
    }
}

std::string JsonValue::toString() const
{
    std::ostringstream oss;
    write(oss, 2);
    return oss.str();
}

std::shared_ptr<JsonValue> JsonValue::createNull()
{
    return std::make_shared<JsonValue>();
}

std::shared_ptr<JsonValue> JsonValue::createBoolean(bool value)
{
    return std::make_shared<JsonValue>(value);
}

std::shared_ptr<JsonValue> JsonValue::createInteger(long long value)
{
    return std::make_shared<JsonValue>(value);
}

std::shared_ptr<JsonValue> JsonValue::createDouble(double value)
{
    return std::make_shared<JsonValue>(value);
}

std::shared_ptr<JsonValue> JsonValue::createString(const std::string& value)
{
    return std::make_shared<JsonValue>(value);
}

std::shared_ptr<JsonValue> JsonValue::createObject()
{
    return std::make_shared<JsonValue>(JsonObject());
}

std::shared_ptr<JsonValue> JsonValue::createArray()
{
    return std::make_shared<JsonValue>(JsonArray());
}

void JsonValue::set(const std::string& key, const std::shared_ptr<JsonValue>& value)
{
    if (m_type != JsonType::Object) {
        return;
    }
    auto& map = std::get<JsonObject>(m_value);
    map[key] = value;
}

void JsonValue::set(const std::string& key, bool value)
{
    set(key, createBoolean(value));
}

void JsonValue::set(const std::string& key, long long value)
{
    set(key, createInteger(value));
}

void JsonValue::set(const std::string& key, double value)
{
    set(key, createDouble(value));
}

void JsonValue::set(const std::string& key, const std::string& value)
{
    set(key, createString(value));
}

void JsonValue::set(const std::string& key, const char* value)
{
    set(key, createString(value));
}

void JsonValue::setNull(const std::string& key)
{
    set(key, createNull());
}

bool JsonValue::remove(const std::string& key)
{
    if (m_type != JsonType::Object) {
        return false;
    }
    auto& map = std::get<JsonObject>(m_value);
    return map.erase(key) > 0;
}

void JsonValue::add(const std::shared_ptr<JsonValue>& value)
{
    if (m_type != JsonType::Array) {
        return;
    }
    auto& vec = std::get<JsonArray>(m_value);
    vec.push_back(value);
}

void JsonValue::add(bool value)
{
    add(createBoolean(value));
}

void JsonValue::add(long long value)
{
    add(createInteger(value));
}

void JsonValue::add(double value)
{
    add(createDouble(value));
}

void JsonValue::add(const std::string& value)
{
    add(createString(value));
}

void JsonValue::add(const char* value)
{
    add(createString(value));
}

void JsonValue::addNull()
{
    add(createNull());
}

JsonParser::JsonParser(const JsonParseOptions& options)
    : m_options(options), m_pos(0), m_line(1), m_column(1)
{
}

JsonParseResult JsonParser::parseFile(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw JsonException("Cannot open file: " + filepath);
    }
    return parseStream(file);
}

JsonParseResult JsonParser::parseString(const std::string& content)
{
    std::istringstream iss(content);
    return parseStream(iss);
}

JsonParseResult JsonParser::parseStream(std::istream& stream)
{
    m_content.assign((std::istreambuf_iterator<char>(stream)),
                     std::istreambuf_iterator<char>());
    m_pos = 0;
    m_line = 1;
    m_column = 1;
    
    JsonParseResult result;
    result.root = parseValue();
    
    skipWhitespace();
    if (m_pos < m_content.size()) {
        throw createParseException("Unexpected character after JSON value");
    }
    
    return result;
}

void JsonParser::skipWhitespace()
{
    while (m_pos < m_content.size()) {
        char c = m_content[m_pos];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (c == '\n') {
                m_line++;
                m_column = 1;
            } else if (c != '\r') {
                m_column++;
            }
            m_pos++;
        } else if (c == '/' && m_pos + 1 < m_content.size()) {
            if (m_content[m_pos + 1] == '/' || m_content[m_pos + 1] == '*') {
                skipComment();
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

void JsonParser::skipComment()
{
    if (m_pos + 1 >= m_content.size()) {
        return;
    }
    
    if (m_content[m_pos + 1] == '/') {
        m_pos += 2;
        m_column += 2;
        while (m_pos < m_content.size() && m_content[m_pos] != '\n') {
            m_pos++;
            m_column++;
        }
    } else if (m_content[m_pos + 1] == '*') {
        m_pos += 2;
        m_column += 2;
        while (m_pos + 1 < m_content.size()) {
            if (m_content[m_pos] == '*' && m_content[m_pos + 1] == '/') {
                m_pos += 2;
                m_column += 2;
                break;
            }
            if (m_content[m_pos] == '\n') {
                m_line++;
                m_column = 1;
            } else if (m_content[m_pos] != '\r') {
                m_column++;
            }
            m_pos++;
        }
    }
}

char JsonParser::peek() const
{
    if (m_pos >= m_content.size()) {
        return '\0';
    }
    return m_content[m_pos];
}

char JsonParser::get()
{
    if (m_pos >= m_content.size()) {
        return '\0';
    }
    char c = m_content[m_pos];
    if (c == '\n') {
        m_line++;
        m_column = 1;
    } else if (c != '\r') {
        m_column++;
    }
    m_pos++;
    return c;
}

void JsonParser::expect(char expected)
{
    char actual = get();
    if (actual != expected) {
        throw createParseException(std::string("Expected '") + expected + "' but got '" + actual + "'");
    }
}

std::shared_ptr<JsonValue> JsonParser::parseValue()
{
    skipWhitespace();
    
    char c = peek();
    if (c == '{') {
        return parseObject();
    } else if (c == '[') {
        return parseArray();
    } else if (c == '"' || (m_options.allowSingleQuotes && c == '\'')) {
        return parseString();
    } else if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
        return parseNumber();
    } else if (c == 't' || c == 'f' || c == 'n') {
        return parseKeyword();
    } else if (m_options.allowUnquotedKeys && (std::isalpha(static_cast<unsigned char>(c)) || c == '_')) {
        return parseString();
    }
    
    throw createParseException("Unexpected character");
}

std::shared_ptr<JsonValue> JsonParser::parseObject()
{
    expect('{');
    
    auto obj = JsonValue::createObject();
    
    while (true) {
        skipWhitespace();
        
        if (peek() == '}') {
            get();
            break;
        }
        
        std::string key;
        if (peek() == '"' || (m_options.allowSingleQuotes && peek() == '\'')) {
            auto keyValue = parseString();
            key = keyValue->getString();
        } else if (m_options.allowUnquotedKeys) {
            size_t startPos = m_pos;
            while (m_pos < m_content.size() && 
                   (std::isalnum(static_cast<unsigned char>(m_content[m_pos])) || 
                    m_content[m_pos] == '_' || m_content[m_pos] == '$')) {
                get();
            }
            key = m_content.substr(startPos, m_pos - startPos);
        } else {
            throw createParseException("Expected string key");
        }
        
        skipWhitespace();
        expect(':');
        skipWhitespace();
        
        obj->set(key, parseValue());
        
        skipWhitespace();
        if (peek() == '}') {
            get();
            break;
        } else if (peek() == ',') {
            get();
            skipWhitespace();
            if (peek() == '}' && !m_options.allowTrailingComma) {
                throw createParseException("Trailing comma not allowed");
            }
        } else {
            throw createParseException("Expected ',' or '}'");
        }
    }
    
    return obj;
}

std::shared_ptr<JsonValue> JsonParser::parseArray()
{
    expect('[');
    
    auto arr = JsonValue::createArray();
    
    while (true) {
        skipWhitespace();
        
        if (peek() == ']') {
            get();
            break;
        }
        
        arr->add(parseValue());
        
        skipWhitespace();
        if (peek() == ']') {
            get();
            break;
        } else if (peek() == ',') {
            get();
            skipWhitespace();
            if (peek() == ']' && !m_options.allowTrailingComma) {
                throw createParseException("Trailing comma not allowed");
            }
        } else {
            throw createParseException("Expected ',' or ']'");
        }
    }
    
    return arr;
}

std::shared_ptr<JsonValue> JsonParser::parseString()
{
    char quoteChar = get();
    if (quoteChar != '"' && quoteChar != '\'') {
        throw createParseException("Expected quote character");
    }
    
    std::ostringstream oss;
    
    while (m_pos < m_content.size()) {
        char c = get();
        if (c == quoteChar) {
            break;
        } else if (c == '\\') {
            char next = get();
            switch (next) {
                case '"': oss << '"'; break;
                case '\'': oss << '\''; break;
                case '\\': oss << '\\'; break;
                case '/': oss << '/'; break;
                case 'n': oss << '\n'; break;
                case 'r': oss << '\r'; break;
                case 't': oss << '\t'; break;
                case 'b': oss << '\b'; break;
                case 'f': oss << '\f'; break;
                case 'u': {
                    if (m_pos + 4 > m_content.size()) {
                        throw createParseException("Incomplete unicode escape");
                    }
                    std::string hexStr = m_content.substr(m_pos, 4);
                    m_pos += 4;
                    m_column += 4;
                    try {
                        int code = std::stoi(hexStr, nullptr, 16);
                        if (code <= 0x7F) {
                            oss << static_cast<char>(code);
                        } else {
                            oss << "\\u" << hexStr;
                        }
                    } catch (...) {
                        throw createParseException("Invalid unicode escape");
                    }
                    break;
                }
                default:
                    oss << '\\' << next;
                    break;
            }
        } else if (c == '\n' || c == '\r') {
            throw createParseException("Unescaped newline in string");
        } else {
            oss << c;
        }
    }
    
    return JsonValue::createString(oss.str());
}

std::shared_ptr<JsonValue> JsonParser::parseNumber()
{
    size_t startPos = m_pos;
    bool hasDot = false;
    bool hasExp = false;
    
    if (peek() == '-') {
        get();
    }
    
    if (peek() == '0') {
        get();
        if (std::isdigit(static_cast<unsigned char>(peek()))) {
            throw createParseException("Leading zeros not allowed");
        }
    } else {
        if (!std::isdigit(static_cast<unsigned char>(peek()))) {
            throw createParseException("Expected digit");
        }
        while (m_pos < m_content.size() && std::isdigit(static_cast<unsigned char>(peek()))) {
            get();
        }
    }
    
    if (peek() == '.') {
        hasDot = true;
        get();
        if (!std::isdigit(static_cast<unsigned char>(peek()))) {
            throw createParseException("Expected digit after decimal point");
        }
        while (m_pos < m_content.size() && std::isdigit(static_cast<unsigned char>(peek()))) {
            get();
        }
    }
    
    if (peek() == 'e' || peek() == 'E') {
        hasExp = true;
        get();
        if (peek() == '+' || peek() == '-') {
            get();
        }
        if (!std::isdigit(static_cast<unsigned char>(peek()))) {
            throw createParseException("Expected digit in exponent");
        }
        while (m_pos < m_content.size() && std::isdigit(static_cast<unsigned char>(peek()))) {
            get();
        }
    }
    
    std::string numStr = m_content.substr(startPos, m_pos - startPos);
    
    try {
        if (hasDot || hasExp) {
            double d = std::stod(numStr);
            return JsonValue::createDouble(d);
        } else {
            long long ll = std::stoll(numStr);
            return JsonValue::createInteger(ll);
        }
    } catch (const std::out_of_range&) {
        throw createParseException("Number out of range");
    } catch (const std::invalid_argument&) {
        throw createParseException("Invalid number format");
    }
}

std::shared_ptr<JsonValue> JsonParser::parseKeyword()
{
    if (m_pos + 4 <= m_content.size() && m_content.substr(m_pos, 4) == "true") {
        m_pos += 4;
        m_column += 4;
        return JsonValue::createBoolean(true);
    } else if (m_pos + 5 <= m_content.size() && m_content.substr(m_pos, 5) == "false") {
        m_pos += 5;
        m_column += 5;
        return JsonValue::createBoolean(false);
    } else if (m_pos + 4 <= m_content.size() && m_content.substr(m_pos, 4) == "null") {
        m_pos += 4;
        m_column += 4;
        return JsonValue::createNull();
    }
    
    throw createParseException("Invalid keyword");
}

JsonParseException JsonParser::createParseException(const std::string& message) const
{
    return JsonParseException(m_line, m_column, message);
}

void JsonParser::write(const std::shared_ptr<JsonValue>& value, std::ostream& stream, int indent)
{
    if (value) {
        value->write(stream, indent);
    } else {
        stream << "null";
    }
}

std::string JsonParser::toString(const std::shared_ptr<JsonValue>& value, int indent)
{
    std::ostringstream oss;
    write(value, oss, indent);
    return oss.str();
}

} // namespace INI
