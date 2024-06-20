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
#include <iostream>
#include <string>
#include <queue>
#include "core.hpp"
#include "visit.hpp"
#include "config.hpp"

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
