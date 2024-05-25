#pragma once
#include <string>
#include <vector>

struct site_icon {
    std::string src = "unknown";
    std::string size = "unknown";
    std::string type = "unknown";
};

struct robots_txt {
    bool exists = false;
    std::vector<std::string> disallowed_urls;
};

struct site_info {
    int status; // special codes:
                // * -0xA97AD0 - artadobot is not allowed
    std::string url = "unknown";
    std::string title = "unknown";
    std::string description = "unknown";
    std::string language = "unknown";
    std::vector<std::string> keywords;
    std::vector<struct site_icon> icons;
    std::vector<std::string> links;
    struct robots_txt robots;
};
