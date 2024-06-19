#include <iostream>
#include <string>
#include <queue>
#include "core.hpp"
#include "visit.hpp"

#pragma region run queue
std::pair<std::queue<std::string>, std::vector<struct site_info>> run_queue(std::queue<std::string> queue)
{
    std::queue<std::string> newqueue;
    std::vector<struct site_info> pages;
    while(!queue.empty())
    {
        // TODO: check if it's OK to visit (last visit time etc.)
        struct site_info currpage = visit_page(queue.front());
        queue.pop();
        for(auto url : currpage.links)
        {
            newqueue.push(url);
        }
        pages.push_back(currpage);
    }
    return make_pair(newqueue, pages);
}
#pragma endregion
