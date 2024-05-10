#include <iostream>
#include <string>
#include "core.hpp"
#include "visit.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <url>" << std::endl;
        return 1;
    }

    std::string url = argv[1];

    struct site_info site_data = visit_page(url);

    return 0;
}
