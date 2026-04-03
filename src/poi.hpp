#pragma once
#include <string>

namespace mygpx {

  /* Encodings of POIType to 16-digit integer:
  
  - First 6 digits: 000000
  - 7-10 digit: 0001 for manual points, 0010 for (csv)single points, 0011 for {manual, csv}single points, 1000 for path points, 1111 for all
  - 11-16 digit: 000001 for railway, 000010 for subway, 001000 for airports, 111111 for all
  
  */
  enum POIType {
    POIType_All                 = 0b000000'1111'111111,
    POIType_Manual              = 0b000000'0001'111111,
    POIType_CSV                 = 0b000000'0010'111111,
    POIType_AllPoint            = 0b000000'0011'111111,
    POIType_AllPathPoint        = 0b000000'1000'111111,
    // POIType_TrafficStationPoint = 100,
    POIType_RailwayStation      = 0b000000'1111'000001,
    POIType_SubwayStation       = 0b000000'1111'000010,
    POIType_Airport             = 0b000000'1111'001000,
    // POIType_TrafficPathPoint    = 0b000000'1000'111111,
    POIType_Railway             = 0b000000'1000'000001,
    POIType_Subway              = 0b000000'1000'000010,
  };

  /* Encoding of POIs to POIKey(int64_t)

  - Ordinary POI:
      | --- 16 -- | --- 48 --- |
      |  PoiType  |   PoiIdx   |
  - Railway Waypoints:
      | --- 16 -- | --- 8 ---- | --- 18 --- | --- 22 --- |
      |  PoiType  | CountryIdx | RailwayIdx |  PointIdx  |
  - Subway Waypoints:
      | --- 16 -- | --- 8 ---- | -- 8 -- | -- 10 -- | --- 22 --- |
      |  PoiType  | CountryIdx | CityIdx | LineIdx  |  PointIdx  |

  */
  struct POI {
    std::string name;
    double lat;
    double lon;
    POIType poitype;
    double distance;
  };

}