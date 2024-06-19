#pragma once
#include <queue>
#include <string>
#include "core.hpp"

std::pair<std::queue<std::string>, std::vector<struct site_info>> run_queue(std::queue<std::string> url);