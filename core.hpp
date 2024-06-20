// This file is part of ArtadoBot Crawler.
// 
// ArtadoBot Crawler is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
// 
// ArtadoBot Crawler is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
// 
// You should have received a copy of the GNU General Public License along with
// ArtadoBot Crawler. If not, see <https://www.gnu.org/licenses/>.
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