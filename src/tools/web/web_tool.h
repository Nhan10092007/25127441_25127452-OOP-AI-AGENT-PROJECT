#pragma once
<<<<<<<< HEAD:src/tools/webtool/web_tool.h
#include "./tool.h"
========
#include "tools/tool.h"
>>>>>>>> origin/develop:src/tools/web/web_tool.h
#include <string>

class WebTool : public Tool {
private:
    // Perform the HTTP GET to DuckDuckGo.
    static std::string httpGet(const std::string& url);

    // Callback for libcurl to write response data.
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);

    // Build the API URL from query parameters.
    static std::string buildUrl(const std::string& query, bool skipDisambig, bool noHtml);
public:
    WebTool();
    std::string execute(const std::string& args) override;
};