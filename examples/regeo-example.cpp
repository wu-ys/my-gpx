#include "regeo.hpp"

int main() {

    try {
        mygpx::POI poi = mygpx::queryRailwayStation(31.421331, 120.643926);
        std::cout << poi.name << ":(" << poi.lon << "," << poi.lat << ")" << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;

}