#pragma once
#include <string>
#include <unordered_set>
#include <initializer_list>

enum class PolicyMode { ALLOW_ALL, DENY_ALL, ALLOWLIST, DENYLIST };

class ToolPolicy {
private:
    PolicyMode mode;
    std::unordered_set<std::string> toolList;
public:
    ToolPolicy();  // defaults to ALLOW_ALL

    // Setters
    void setMode(PolicyMode m);
    void addTool(const std::string& toolName);
    void removeTool(const std::string& toolName);

    // Core check
    bool isAllowed(const std::string& toolName) const;

    // Factory helpers
    static ToolPolicy allowAll();
    static ToolPolicy denyAll();
    static ToolPolicy allowOnly(std::initializer_list<std::string> tools);
    static ToolPolicy denyOnly(std::initializer_list<std::string> tools);
};