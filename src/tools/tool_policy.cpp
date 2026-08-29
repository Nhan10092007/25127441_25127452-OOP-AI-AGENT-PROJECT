#include "tool_policy.h"

ToolPolicy::ToolPolicy() : mode(PolicyMode::ALLOW_ALL) {}

void ToolPolicy::setMode(PolicyMode m) {
    mode = m;
}

void ToolPolicy::addTool(const std::string& toolName) {
    toolList.insert(toolName);
}

void ToolPolicy::removeTool(const std::string& toolName) {
    toolList.erase(toolName);
}

bool ToolPolicy::isAllowed(const std::string& toolName) const {
    switch (mode) {
        case PolicyMode::ALLOW_ALL:
            return true;
        case PolicyMode::DENY_ALL:
            return false;
        case PolicyMode::ALLOWLIST:
            return toolList.contains(toolName);
        case PolicyMode::DENYLIST:
            return !toolList.contains(toolName);
    }
    return true; // fallback
}

ToolPolicy ToolPolicy::allowAll() {
    ToolPolicy p;
    p.mode = PolicyMode::ALLOW_ALL;
    return p;
}

ToolPolicy ToolPolicy::denyAll() {
    ToolPolicy p;
    p.mode = PolicyMode::DENY_ALL;
    return p;
}

ToolPolicy ToolPolicy::allowOnly(std::initializer_list<std::string> tools) {
    ToolPolicy p;
    p.mode = PolicyMode::ALLOWLIST;
    for (const auto& t : tools) {
        p.toolList.insert(t);
    }
    return p;
}

ToolPolicy ToolPolicy::denyOnly(std::initializer_list<std::string> tools) {
    ToolPolicy p;
    p.mode = PolicyMode::DENYLIST;
    for (const auto& t : tools) {
        p.toolList.insert(t);
    }
    return p;
}
