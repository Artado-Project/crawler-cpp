#pragma once
#include <string>
#include <vector>

struct site_icon {
    std::string src = "unknown";
    std::string size = "unknown";
    std::string type = "unknown";
};

struct site_info {
    std::string url = "unknown";
    std::string title = "unknown";
    std::string description = "unknown";
    std::string language = "unknown";
    std::vector<std::string> keywords;
    std::vector<struct site_icon> icons;
    std::vector<std::string> links;
};
