#pragma once
#include <string>
#include <vector>
#include <set>

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
    long status; // special codes:
                // * -0xA97AD0 - crawler is not allowed
    std::string url = "unknown";
    std::string title = "unknown";
    std::string description = "unknown";
    std::string language = "unknown";
    std::vector<std::string> keywords;
    std::vector<struct site_icon> icons;
    std::set<std::string> links;
    struct robots_txt robots;
    long long visit_time;
};

#define DBG_CONT 1
#define DBG_RECR 2
#define DBG_INFO 4
#define DBG_QUEUE 8

extern uint16_t debug_level;