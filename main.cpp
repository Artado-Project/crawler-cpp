#include <iostream>
#include <string>
#include <filesystem>
#include "core.hpp"
#include "queue.hpp"

int main(int argc, char *argv[]) {
    debug_level = 0;
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <url> <depth>" << std::endl;
        return 1;
    }

    std::vector<std::string> cache_directories = {
        ".artadobot_cache",
        ".artadobot_cache/robots",
    };
    for(auto path : cache_directories)
    {
        try {
            if (!std::filesystem::exists(path)) {
                if (std::filesystem::create_directories(path)) {
                    std::cout << "Created cache directory " << path << std::endl;
                } else {
                    std::cerr << "Failed to create cache directory: " << path << std::endl;
                    return 1;
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }

    std::string url = argv[1];
    int depth = std::stoi(argv[2]);

    std::queue<std::string> q;
    std::vector<struct site_info> visited;
    q.push(url);
    while(depth--)
    {
        std::pair<std::queue<std::string>, std::vector<struct site_info>> level_info = run_queue(q);
        q = level_info.first;
        visited.reserve(visited.size() + distance(level_info.second.begin(), level_info.second.end()));
        visited.insert(visited.end(), level_info.second.begin(), level_info.second.end());
    }
    if (debug_level & DBG_QUEUE)
    {
        std::cout << "New queue:" << std::endl;
        while(!q.empty())
        {
            std::cout << "* " << q.front() << std::endl;
            q.pop();
        }
    }
    for(auto page : visited)
    {
        std::cout << page.url << " - " << page.title << std::endl;
        if (debug_level & DBG_INFO)
        {
            std::cout << "Description: " << page.description << std::endl;
            std::cout << "Language: " << page.language << std::endl;
            std::cout << "Keywords: ";
            for(auto keyword : page.keywords)
            {
                std::cout << keyword << " ";
            }
            std::cout << std::endl;
            std::cout << "Icons: " << std::endl;
            for(auto icon : page.icons)
            {
                std::cout << "  Icon: " << std::endl;
                std::cout << "    Link: " << icon.src << std::endl;
                std::cout << "    Size: " << icon.size << std::endl;
                std::cout << "    Type: " << icon.type << std::endl;
            }
            std::cout << "Links: " << std::endl;
            for(auto link : page.links)
            {
                std::cout << "  " << link << std::endl;
            }
        }
    }

    return 0;
}
