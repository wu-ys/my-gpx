#include "utils.hpp"
#include <iostream>

int main() {
    std::string t = "2025-11-17T16:00:01Z";

    long long ts10 = mygpx::to_timestamp_seconds(t);
    long long ts13 = mygpx::to_timestamp_millis(t);

    std::cout << "10-digit timestamp: " << ts10 << "\n";
    std::cout << "13-digit timestamp: " << ts13 << "\n";
}