#pragma once
#include <vector>
#include <set>
#include <string>
#include <map>
#include <unordered_map>
#include <optional>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include "utils.hpp"
#include "geohash.hpp"
#include "poi.hpp"

namespace mygpx {

constexpr int GEOHASH_LEVEL = 7;

// ============================================================================
//  Metadata structs
// ============================================================================

struct CountryInfo {
    uint8_t id;
    std::string iso2, iso3, name, name_zh;
};

struct RailwayLayoutInfo {
    std::string strategy = "flat";   // "flat", "operator_line", "range"
    std::vector<uint16_t> partition_range;
    std::vector<std::string> partition_op;
    uint16_t op_bits = 0;
    uint16_t line_bits = 16;
};

struct RailwayOperatorInfo {
    uint8_t country_id;
    uint8_t op_id;
    std::string code;       // short mnemonic, e.g. "jre"
    std::string name_en;    // English name
    std::string name_zh;    // Chinese name
};

// ============================================================================
//  POIMeta — country & railway metadata
// ============================================================================

class POIMeta {
public:
    // ---- Factory methods ----

    /// Load metadata from JSON files in @p data_dir.
    static POIMeta load(const std::string& data_dir);

    /// Load metadata from an already-initialized SQLite database.
    static POIMeta loadFromDB(soci::session& db);

    /// Bootstrap the database with metadata tables and populate from JSON.
    /// Safe to call on an existing database (CREATE TABLE IF NOT EXISTS).
    static void initDB(soci::session& db, const std::string& data_dir);

    // ---- Country lookup ----

    std::optional<uint8_t>    countryId(const std::string& iso2) const;
    const CountryInfo*        countryInfo(uint8_t id) const;
    const std::unordered_map<uint8_t, CountryInfo>& allCountries() const;

    // ---- Railway layout ----

    const RailwayLayoutInfo*  railwayLayout(uint8_t country_id) const;

    uint16_t packRailway(uint8_t country_id, uint8_t op, uint16_t line) const;
    std::pair<uint8_t, uint16_t> unpackRailway(uint8_t country_id, uint16_t enc) const;

    // ---- Railway operator lookup ----

    const RailwayOperatorInfo* operatorInfo(uint8_t country_id, uint8_t op_id) const;
    std::optional<uint8_t>     operatorId(uint8_t country_id, const std::string& code) const;

    // ---- Human-readable formatting ----

    std::string formatKey(uint64_t key) const;

    // ========================================================================
    //  Database update API
    //  Each method writes through to both the DB and the in-memory maps.
    // ========================================================================

    /// Insert or update a country record.  Also refreshes iso2→id index.
    void upsertCountry(soci::session& db, const CountryInfo& ci);
    /// Delete a country and its associated layouts & operators.
    void deleteCountry(soci::session& db, uint8_t id);

    /// Insert or update a railway operator.  Also refreshes code→id index.
    void upsertOperator(soci::session& db,
                        uint8_t country_id, uint8_t op_id,
                        const std::string& code, const std::string& name_en,
                        const std::string& name_zh = "");
    /// Delete a single railway operator.
    void deleteOperator(soci::session& db, uint8_t country_id, uint8_t op_id);

    /// Replace the railway layout for a country.
    void setRailwayLayout(soci::session& db, uint8_t country_id,
                          const RailwayLayoutInfo& layout);

    /// Drop all metadata tables, re-create from JSON, rebuild in-memory maps.
    void reloadMetaFromJSON(soci::session& db, const std::string& data_dir);

private:
    void loadCountriesFromJSON(const std::string& data_dir);
    void loadRailwayLayoutsFromJSON(const std::string& data_dir);
    void loadOperatorsFromJSON(const std::string& data_dir);

    void loadCountriesFromDB(soci::session& db);
    void loadRailwayLayoutsFromDB(soci::session& db);
    void loadOperatorsFromDB(soci::session& db);

    void clearMaps();

    // country_id → country info
    std::unordered_map<uint8_t, CountryInfo>            countries_;

    // country iso2 code → country_id
    std::unordered_map<std::string, uint8_t>            iso2_to_id_;

    // country_id → railway layout
    std::unordered_map<uint8_t, RailwayLayoutInfo>      layouts_;

    // (country_id << 8) | op_id  →  operator info
    std::unordered_map<uint32_t, RailwayOperatorInfo> operators_by_id_;

    // "<country_id>:<op_code>"  →  (country_id, op_id)
    std::unordered_map<std::string, std::pair<uint8_t, uint8_t>> op_code_index_;
};

// ============================================================================
//  POIDataBase — SQLite-backed POI storage
// ============================================================================

class POIDataBase {

private:
    soci::session _poidb;

    static const CoordinateSystem _coord = CoordinateSystem::CoordinateSystem_WGS84;

    void initPOIDataBase();

public:
    POIDataBase();
    ~POIDataBase();

    void saveToCSV(const std::string& filename) const;
    int  loadFromCSV(const std::string& filename);

    int  loadFromAirportCSV(const std::string& filename,
                            uint8_t country_id = 0,
                            CoordinateSystem coord = CoordinateSystem::CoordinateSystem_WGS84);

    int  loadFromRailwayCSV(const std::string& filename,
                            uint8_t country_id = 0,
                            CoordinateSystem coord = CoordinateSystem::CoordinateSystem_WGS84);

    void insertPOI(double lat, double lon, POIType poitype,
                   const std::string& name, uint64_t poicode = 0,
                   CoordinateSystem coord = CoordinateSystem::CoordinateSystem_WGS84);

    std::vector<POI> query(double lat, double lon,
                           uint8_t type_filter = 0xFF,
                           double range = 150.0) const;
};

} // namespace mygpx
