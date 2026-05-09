#pragma once
#include <cstdint>
#include <string>

namespace mygpx {

/* ============================================================================
   64-bit POI Key Layout:  [8-bit type | 56-bit payload]

   POI Types (bits 63..56):
     0x00  Frequently Visited Point
     0x99  Railway Station
     0x9A  Subway Station
     0xA0  Airport
     0xC9  Railway Waypoint
     0xCA  Subway Waypoint

   Payload formats (bits 55..0, packed from LSB upward):
   ─────────────────────────────────────────────────────────────
   Airport (0x01):       | 13b reserved | 8b country | 15b IATA | 20b ICAO |
     IATA: 3 letters × 5bit A1Z26 (1..26), 0 = empty
     ICAO: 4 letters × 5bit A1Z26 (1..26), 0 = empty

   RailwayStation (0x02):| 8b country | 4b class | 44b station_idx |
     class: 0=regular, 1=high-speed, 2=freight, 3=special, etc.

   SubwayStation (0x03): | 8b country | 8b city | 8b line | 8b station | 24b reserved |

   Frequent (0x04):      | 8b category | 48b index |

   RailwayWaypoint (0x10):| 8b country | 16b railway_enc | 32b distance_m |
     railway_enc: packed per country (flat line_id or operator+line; see railway_layouts.json)
     distance_m:  metres from line origin (monotonic → sequence-comparable)

   SubwayWaypoint (0x11):| 8b country | 8b city | 8b line | 32b distance_m |
   ============================================================================ */

// ---- POI Type enum (8-bit) ----
enum class POIType : uint8_t {
    Frequent        = 0x00, // 0000 0000
    RailwayStation  = 0x99, // 1001 1001
    SubwayStation   = 0x9A, // 1001 1010
    Airport         = 0xA0, // 1010 0000
    RailwayWaypoint = 0xC9, // 1100 1001
    SubwayWaypoint  = 0xCA, // 1100 1010

};

// Check if a type byte is a waypoint
inline bool isWaypoint(POIType t)    { return isWaypoint(static_cast<uint8_t>(t)); }
inline bool isRailway(POIType t)     { return isRailway(static_cast<uint8_t>(t)); }
inline bool isSubway(POIType t)      { return isSubway(static_cast<uint8_t>(t)); }
inline bool isAirport(POIType t)     { return isAirport(static_cast<uint8_t>(t)); }
inline bool isTrafficHub(POIType t)  { return isTrafficHub(static_cast<uint8_t>(t)); }
inline bool isWaypoint(uint8_t t)    { return (t & 0xC0) == 0xC0; }   // 11xx xxxx
inline bool isRailway (uint8_t t)    { return (t & 0x8F) == 0x89; }   // 1xxx 1001
inline bool isSubway  (uint8_t t)    { return (t & 0x8F) == 0x8A; }   // 1xxx 1010
inline bool isAirport (uint8_t t)    { return (t == 0xA0); }          // 1010 0000
inline bool isTrafficHub (uint8_t t) { return (t & 0xC0) == 0x80; }   // 10xx xxxx

// ---- Bit layout constants ----
namespace poi_bits {
    constexpr int TYPE_SHIFT = 56;
    constexpr uint64_t TYPE_MASK = 0xFFULL << 56;

    // Airport
    constexpr int AIRPORT_ICAO_SHIFT   = 0;
    constexpr int AIRPORT_IATA_SHIFT   = 20;
    constexpr int AIRPORT_COUNTRY_SHIFT = 35;

    // Railway Station
    constexpr int RSTATION_IDX_SHIFT    = 0;
    constexpr int RSTATION_CLASS_SHIFT  = 44;
    constexpr int RSTATION_COUNTRY_SHIFT = 48;

    // Subway Station
    constexpr int SSTATION_RESV_SHIFT     = 0;
    constexpr int SSTATION_STATION_SHIFT  = 24;
    constexpr int SSTATION_LINE_SHIFT     = 32;
    constexpr int SSTATION_CITY_SHIFT     = 40;
    constexpr int SSTATION_COUNTRY_SHIFT  = 48;

    // Frequent
    constexpr int FREQ_INDEX_SHIFT    = 0;
    constexpr int FREQ_CATEGORY_SHIFT = 48;

    // Railway Waypoint
    constexpr int RWP_DIST_SHIFT    = 0;
    constexpr int RWP_RAILWAY_SHIFT = 32;
    constexpr int RWP_COUNTRY_SHIFT = 48;

    // Subway Waypoint
    constexpr int SWP_DIST_SHIFT    = 0;
    constexpr int SWP_LINE_SHIFT    = 32;
    constexpr int SWP_CITY_SHIFT    = 40;
    constexpr int SWP_COUNTRY_SHIFT = 48;
}

// ---- Decoded POI key ----
struct DecodedKey {
    POIType type = POIType::Airport;
    uint8_t country = 0;

    // Airport
    uint8_t iata_a1z26[3] = {0, 0, 0};   // A1Z26 values (1..26), 0 = empty
    uint8_t icao_a1z26[4] = {0, 0, 0, 0};

    // Railway Station
    uint8_t  station_class = 0;
    uint64_t station_idx = 0;

    // Subway Station
    uint8_t subway_city = 0;
    uint8_t subway_line = 0;
    uint8_t subway_station = 0;

    // Frequent
    uint8_t  freq_category = 0;
    uint64_t freq_index = 0;

    // Railway Waypoint
    uint16_t rwp_line = 0;   // opaque packed value, unpack via POIMeta
    uint32_t rwp_dist_m = 0;

    // Subway Waypoint
    uint8_t  swp_city = 0;
    uint8_t  swp_line = 0;
    uint32_t swp_dist_m = 0;
};

// ---- POI struct (runtime POI data, used by regeo queries) ----
struct POI {
    std::string name;
    double lat;
    double lon;
    POIType poitype;
    double distance;
};

// ============================================================================
//  Encode functions — build a uint64_t key from typed fields
// ============================================================================

inline uint64_t encodeAirport(uint8_t country,
                              const std::string& iata,
                              const std::string& icao) {
    uint64_t key = static_cast<uint64_t>(POIType::Airport) << poi_bits::TYPE_SHIFT;
    key |= static_cast<uint64_t>(country) << poi_bits::AIRPORT_COUNTRY_SHIFT;

    auto a1z26 = [](char c) -> uint64_t {
        if (c >= 'a' && c <= 'z') return static_cast<uint64_t>(c - 'a' + 1);
        if (c >= 'A' && c <= 'Z') return static_cast<uint64_t>(c - 'A' + 1);
        return 0;
    };

    if (iata.size() == 3) {
        key |= a1z26(iata[0]) << (poi_bits::AIRPORT_IATA_SHIFT + 10);
        key |= a1z26(iata[1]) << (poi_bits::AIRPORT_IATA_SHIFT + 5);
        key |= a1z26(iata[2]) << (poi_bits::AIRPORT_IATA_SHIFT);
    }
    if (icao.size() == 4) {
        key |= a1z26(icao[0]) << (poi_bits::AIRPORT_ICAO_SHIFT + 15);
        key |= a1z26(icao[1]) << (poi_bits::AIRPORT_ICAO_SHIFT + 10);
        key |= a1z26(icao[2]) << (poi_bits::AIRPORT_ICAO_SHIFT + 5);
        key |= a1z26(icao[3]) << (poi_bits::AIRPORT_ICAO_SHIFT);
    }
    return key;
}

inline uint64_t encodeRailwayStation(uint8_t country, uint8_t cls, uint64_t idx) {
    uint64_t key = static_cast<uint64_t>(POIType::RailwayStation) << poi_bits::TYPE_SHIFT;
    key |= static_cast<uint64_t>(country) << poi_bits::RSTATION_COUNTRY_SHIFT;
    key |= static_cast<uint64_t>(cls) << poi_bits::RSTATION_CLASS_SHIFT;
    key |= idx << poi_bits::RSTATION_IDX_SHIFT;
    return key;
}

inline uint64_t encodeSubwayStation(uint8_t country, uint8_t city,
                                     uint8_t line, uint8_t station) {
    uint64_t key = static_cast<uint64_t>(POIType::SubwayStation) << poi_bits::TYPE_SHIFT;
    key |= static_cast<uint64_t>(country) << poi_bits::SSTATION_COUNTRY_SHIFT;
    key |= static_cast<uint64_t>(city)    << poi_bits::SSTATION_CITY_SHIFT;
    key |= static_cast<uint64_t>(line)    << poi_bits::SSTATION_LINE_SHIFT;
    key |= static_cast<uint64_t>(station) << poi_bits::SSTATION_STATION_SHIFT;
    return key;
}

inline uint64_t encodeFrequent(uint8_t category, uint64_t index) {
    uint64_t key = static_cast<uint64_t>(POIType::Frequent) << poi_bits::TYPE_SHIFT;
    key |= static_cast<uint64_t>(category) << poi_bits::FREQ_CATEGORY_SHIFT;
    key |= index << poi_bits::FREQ_INDEX_SHIFT;
    return key;
}

inline uint64_t encodeRailwayWP(uint8_t country, uint16_t railway_enc, uint32_t dist_m) {
    uint64_t key = static_cast<uint64_t>(POIType::RailwayWaypoint) << poi_bits::TYPE_SHIFT;
    key |= static_cast<uint64_t>(country)     << poi_bits::RWP_COUNTRY_SHIFT;
    key |= static_cast<uint64_t>(railway_enc) << poi_bits::RWP_RAILWAY_SHIFT;
    key |= static_cast<uint64_t>(dist_m)      << poi_bits::RWP_DIST_SHIFT;
    return key;
}

inline uint64_t encodeSubwayWP(uint8_t country, uint8_t city,
                                uint8_t line, uint32_t dist_m) {
    uint64_t key = static_cast<uint64_t>(POIType::SubwayWaypoint) << poi_bits::TYPE_SHIFT;
    key |= static_cast<uint64_t>(country) << poi_bits::SWP_COUNTRY_SHIFT;
    key |= static_cast<uint64_t>(city)    << poi_bits::SWP_CITY_SHIFT;
    key |= static_cast<uint64_t>(line)    << poi_bits::SWP_LINE_SHIFT;
    key |= static_cast<uint64_t>(dist_m)  << poi_bits::SWP_DIST_SHIFT;
    return key;
}

// ============================================================================
//  Decode — extract all fields from a uint64_t key
// ============================================================================

inline DecodedKey decode(uint64_t key) {
    DecodedKey dk;
    dk.type = static_cast<POIType>(key >> poi_bits::TYPE_SHIFT);

    switch (dk.type) {
    case POIType::Airport:
        dk.country       = (key >> poi_bits::AIRPORT_COUNTRY_SHIFT) & 0xFF;
        dk.iata_a1z26[0] = (key >> (poi_bits::AIRPORT_IATA_SHIFT + 10)) & 0x1F;
        dk.iata_a1z26[1] = (key >> (poi_bits::AIRPORT_IATA_SHIFT + 5))  & 0x1F;
        dk.iata_a1z26[2] = (key >>  poi_bits::AIRPORT_IATA_SHIFT)       & 0x1F;
        dk.icao_a1z26[0] = (key >> (poi_bits::AIRPORT_ICAO_SHIFT + 15)) & 0x1F;
        dk.icao_a1z26[1] = (key >> (poi_bits::AIRPORT_ICAO_SHIFT + 10)) & 0x1F;
        dk.icao_a1z26[2] = (key >> (poi_bits::AIRPORT_ICAO_SHIFT + 5))  & 0x1F;
        dk.icao_a1z26[3] = (key >>  poi_bits::AIRPORT_ICAO_SHIFT)       & 0x1F;
        break;

    case POIType::RailwayStation:
        dk.country       = (key >> poi_bits::RSTATION_COUNTRY_SHIFT) & 0xFF;
        dk.station_class = (key >> poi_bits::RSTATION_CLASS_SHIFT)   & 0xF;
        dk.station_idx   = (key >> poi_bits::RSTATION_IDX_SHIFT)     & 0xFFFFFFFFFFFULL;  // 44 bits
        break;

    case POIType::SubwayStation:
        dk.country        = (key >> poi_bits::SSTATION_COUNTRY_SHIFT) & 0xFF;
        dk.subway_city    = (key >> poi_bits::SSTATION_CITY_SHIFT)    & 0xFF;
        dk.subway_line    = (key >> poi_bits::SSTATION_LINE_SHIFT)    & 0xFF;
        dk.subway_station = (key >> poi_bits::SSTATION_STATION_SHIFT) & 0xFF;
        break;

    case POIType::Frequent:
        dk.freq_category = (key >> poi_bits::FREQ_CATEGORY_SHIFT) & 0xFF;
        dk.freq_index    = (key >> poi_bits::FREQ_INDEX_SHIFT)    & 0xFFFFFFFFFFFFULL;  // 48 bits
        break;

    case POIType::RailwayWaypoint:
        dk.country     = (key >> poi_bits::RWP_COUNTRY_SHIFT) & 0xFF;
        dk.rwp_line    = (key >> poi_bits::RWP_RAILWAY_SHIFT) & 0xFFFF;
        dk.rwp_dist_m  = (key >> poi_bits::RWP_DIST_SHIFT)    & 0xFFFFFFFFULL;
        break;

    case POIType::SubwayWaypoint:
        dk.country     = (key >> poi_bits::SWP_COUNTRY_SHIFT) & 0xFF;
        dk.swp_city    = (key >> poi_bits::SWP_CITY_SHIFT)    & 0xFF;
        dk.swp_line    = (key >> poi_bits::SWP_LINE_SHIFT)    & 0xFF;
        dk.swp_dist_m  = (key >> poi_bits::SWP_DIST_SHIFT)    & 0xFFFFFFFFULL;
        break;
    }
    return dk;
}

// Quick extract — type only, no full decode
inline POIType decodeType(uint64_t key) {
    return static_cast<POIType>(key >> poi_bits::TYPE_SHIFT);
}

} // namespace mygpx
