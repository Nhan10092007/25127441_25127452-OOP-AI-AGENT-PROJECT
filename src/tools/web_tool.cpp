#include "web_tool.h"
#include <curl/curl.h>
#include <sstream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

WebTool::WebTool() 
    : Tool("web_search", "Search the web via DuckDuckGo. Provide a plain string as the search query (example: 'How to be rich'). Do NOT wrap it in JSON.") {}
std::string WebTool::execute(const std::string& args){
    if (args.empty()){
    throw std::runtime_error("Missing required field: 'query'");
    }
    const bool SKIPDISAMBIG = true;
    const bool NOHTML = true;

    std::string url = buildUrl(args, SKIPDISAMBIG, NOHTML);
    std::string response = httpGet(url);
    json api;
    try{
        api = json::parse(response);
    }
    catch (const json::parse_error& e){
        return json{
            {"error", "Failed to parse API response JSON"},
            {"detail", e.what()}
        }.dump();
    }
    json result;
    result["query"] = args;
    result["abstract"] = api.value("Abstract", "");
    result["source"] = api.value("AbstractSource", "");
    result["url"] = api.value("AbstractURL", "");
    result["answer"] = api.value("Answer", "");
    result["heading"] = api.value("Heading", "");
    result["definition"] = api.value("Definition", "");
    json relatedTopics = json::array();
    for (const auto& topic : api["RelatedTopics"]){
        if (topic.contains("Text") && topic.contains("FirstURL")){
            relatedTopics.push_back({
                {"text", topic["Text"]},
                {"url", topic["FirstURL"]}
            });
        }
    }
    result["related_topics"] = relatedTopics;
    if(api.contains("Infobox") && !api["Infobox"].is_null()){
        result["infobox"] = api["Infobox"];
    }
    return result.dump(2);
}

std::string WebTool::buildUrl(const std::string& query, bool skipDisambig, bool noHtml){
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    char* encoded = curl_easy_escape(curl, query.c_str(), query.length());
    std::string encodedQuery(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    std::ostringstream url;
    url << "https://api.duckduckgo.com/"
        << "?q=" << encodedQuery
        << "&format=json"
        << "&pretty=0";
    if (skipDisambig) {
        url << "&skip_disambig=1";
    }
    if (noHtml) {
        url << "&no_html=1";
    }
    return url.str();
}

std::string WebTool::httpGet(const std::string& url) {
    CURL*curl=curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    std::string response{};
    //OPTION
    curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl,CURLOPT_TIMEOUT,10L);
    curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1L);
    curl_easy_setopt(curl,CURLOPT_USERAGENT,"WebTool-DuckDuckGo/1.0");

    //SEND REQUEST

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::string err = curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        throw std::runtime_error("HTTP request failed: " + err);
    }
    //CHECK
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (httpCode != 200) {
        throw std::runtime_error(
            "HTTP " + std::to_string(httpCode) + ": " + response);
    }
    return response;
}

size_t WebTool::writeCallback(void* contents, size_t size,
                               size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    static_cast<std::string*>(userp)->append(
        static_cast<char*>(contents), totalSize);
    return totalSize;
}