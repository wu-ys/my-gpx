#pragma once
#include "config.hpp"

namespace mygpx {

    struct POI {
        std::string name;
        double lat;
        double lon;
        double distance;
    };

    std::string queryAddress(double latitude, double longitude);

    std::string queryAddressFromAmap(double latitude, double longitude, std::vector<int> poitypes = {}, double poi_radius = 1000.0);
    POI queryPOIFromAmap(double latitude, double longitude, std::vector<int> poitypes = {}, double poi_radius = 1000.0);
    std::string queryFromOSM(double latitude, double longitude, std::vector<int> poitypes = {}, double poi_radius = 1000.0);

    POI queryRailwayStation(double latitude, double longitude, double radius = 1000.0);
    POI querySubwayStation(double latitude, double longitude, double radius = 1000.0);
    POI queryBusStation(double latitude, double longitude, double radius = 1000.0);
    POI queryAirport(double latitude, double longitude, double radius = 1000.0);

}
