#include "gpxpoint.hpp"

int main() {

    mygpx::GPXPoint p(39.917904, 116.396956, 55.0, 1622548860);

    std::cout << p.QueryAddress() << "\n";

    return 0;

}