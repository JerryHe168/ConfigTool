#include <iostream>
#include <cassert>
#include <fstream>
#include "IniConfig.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#define PATH_SEP "\\"
#else
#include <limits.h>
#include <unistd.h>
#define PATH_SEP "/"
#endif

std::string getTestDirectory()
{
    static std::string dir;
    if (!dir.empty()) return dir;
    
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exePath(buffer);
    size_t lastSep = exePath.find_last_of("\\/");
    if (lastSep != std::string::npos) {
        dir = exePath.substr(0, lastSep);
    }
#else
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        std::string exePath(buffer);
        size_t lastSep = exePath.find_last_of('/');
        if (lastSep != std::string::npos) {
            dir = exePath.substr(0, lastSep);
        }
    }
#endif
    
    return dir;
}

void createTestFile(const std::string& filename, const std::string& content)
{
    std::ofstream file(filename);
    assert(file.is_open());
    file << content;
    file.close();
}

void testBasicInclude()
{
    std::cout << "Testing basic include functionality..." << std::endl;
    
    std::string testDir = getTestDirectory();
    std::string basePath = testDir + PATH_SEP;
    
    std::string config2Content = R"(
[included_section]
included_key = included_value
another_key = 42
)";
    
    createTestFile(basePath + "config2.ini", config2Content);
    
    std::string mainConfigContent = R"(
[main_section]
main_key = main_value

[path]
include = config2.ini
)";
    
    createTestFile(basePath + "config_main.ini", mainConfigContent);
    
    INI::IniConfig config;
    assert(config.loadFile(basePath + "config_main.ini"));
    
    assert(config.hasSection("main_section"));
    assert(config.hasSection("included_section"));
    
    assert(config.getString("main_section", "main_key") == "main_value");
    assert(config.getString("included_section", "included_key") == "included_value");
    assert(config.getInt("included_section", "another_key") == 42);
    
    std::cout << "  Basic include: PASSED" << std::endl;
}

void testMultipleIncludes()
{
    std::cout << "Testing multiple includes..." << std::endl;
    
    std::string testDir = getTestDirectory();
    std::string basePath = testDir + PATH_SEP;
    
    createTestFile(basePath + "inc_a.ini", R"(
[section_a]
key_a = value_a
)");
    
    createTestFile(basePath + "inc_b.ini", R"(
[section_b]
key_b = value_b
)");
    
    createTestFile(basePath + "inc_c.ini", R"(
[section_c]
key_c = value_c
)");
    
    std::string mainContent = R"(
[main]
test = test_value

[path]
include = inc_a.ini
include = inc_b.ini
)";
    
    createTestFile(basePath + "multi_main.ini", mainContent);
    
    INI::IniConfig config;
    assert(config.loadFile(basePath + "multi_main.ini"));
    
    assert(config.hasSection("section_a"));
    assert(config.hasSection("section_b"));
    assert(config.getString("section_a", "key_a") == "value_a");
    assert(config.getString("section_b", "key_b") == "value_b");
    
    std::cout << "  Multiple includes: PASSED" << std::endl;
}

void testIncludedFilesTracking()
{
    std::cout << "Testing included files tracking..." << std::endl;
    
    std::string testDir = getTestDirectory();
    std::string basePath = testDir + PATH_SEP;
    
    createTestFile(basePath + "tracked1.ini", R"(
[tracked1]
key = tracked1_value
)");
    
    createTestFile(basePath + "tracked2.ini", R"(
[tracked2]
key = tracked2_value
)");
    
    std::string mainContent = R"(
[main]
test = value

[path]
include = tracked1.ini
include = tracked2.ini
)";
    
    createTestFile(basePath + "tracking_main.ini", mainContent);
    
    INI::IniConfig config;
    assert(config.loadFile(basePath + "tracking_main.ini"));
    
    const auto& included = config.getIncludedFiles();
    assert(included.size() >= 2);
    
    bool found1 = false, found2 = false;
    for (const auto& file : included) {
        if (file.find("tracked1.ini") != std::string::npos) found1 = true;
        if (file.find("tracked2.ini") != std::string::npos) found2 = true;
    }
    
    assert(found1);
    assert(found2);
    
    std::cout << "  Included files tracking: PASSED" << std::endl;
}

void testSectionMerge()
{
    std::cout << "Testing section merge functionality..." << std::endl;
    
    std::string testDir = getTestDirectory();
    std::string basePath = testDir + PATH_SEP;
    
    createTestFile(basePath + "merge_inc.ini", R"(
[shared]
from_include = included
key2 = value2
)");
    
    std::string mainContent = R"(
[shared]
from_main = main_value
key1 = value1

[path]
include = merge_inc.ini
)";
    
    createTestFile(basePath + "merge_main.ini", mainContent);
    
    INI::IniConfig config;
    assert(config.loadFile(basePath + "merge_main.ini"));
    
    assert(config.hasSection("shared"));
    assert(config.hasKey("shared", "from_main"));
    assert(config.hasKey("shared", "from_include"));
    assert(config.hasKey("shared", "key1"));
    assert(config.hasKey("shared", "key2"));
    
    assert(config.getString("shared", "from_main") == "main_value");
    assert(config.getString("shared", "from_include") == "included");
    
    std::cout << "  Section merge: PASSED" << std::endl;
}

void testNestedInclude()
{
    std::cout << "Testing nested include functionality..." << std::endl;
    
    std::string testDir = getTestDirectory();
    std::string basePath = testDir + PATH_SEP;
    
    createTestFile(basePath + "deepest.ini", R"(
[deepest]
value = deepest_value
)");
    
    createTestFile(basePath + "middle.ini", R"(
[middle]
value = middle_value

[path]
include = deepest.ini
)");
    
    std::string mainContent = R"(
[main]
value = main_value

[path]
include = middle.ini
)";
    
    createTestFile(basePath + "nested_main.ini", mainContent);
    
    INI::IniConfig config;
    assert(config.loadFile(basePath + "nested_main.ini"));
    
    assert(config.hasSection("main"));
    assert(config.hasSection("middle"));
    assert(config.hasSection("deepest"));
    
    assert(config.getString("main", "value") == "main_value");
    assert(config.getString("middle", "value") == "middle_value");
    assert(config.getString("deepest", "value") == "deepest_value");
    
    std::cout << "  Nested include: PASSED" << std::endl;
}

int main()
{
    std::cout << "=== Include Tests ===" << std::endl << std::endl;
    
    testBasicInclude();
    testMultipleIncludes();
    testIncludedFilesTracking();
    testSectionMerge();
    testNestedInclude();
    
    std::cout << std::endl << "=== All Include Tests PASSED ===" << std::endl;
    
    return 0;
}
