#include "poidb.hpp"
#include <rapidcsv.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <cmath>

namespace mygpx {

// ============================================================================
//  POIMeta — private helpers
// ============================================================================

void POIMeta::clearMaps() {
    countries_.clear();
    iso2_to_id_.clear();
    layouts_.clear();
    operators_by_id_.clear();
    op_code_index_.clear();
}

void POIMeta::loadCountriesFromJSON(const std::string& data_dir) {
    std::ifstream f(data_dir + "/countries_regions.json");
    if (!f.is_open())
        throw std::runtime_error("Cannot open " + data_dir + "/countries_regions.json");

    nlohmann::json j = nlohmann::json::parse(f);
    for (const auto& e : j) {
        CountryInfo ci;
        ci.id       = e.at("id").get<uint8_t>();
        ci.iso2     = e.at("iso2").get<std::string>();
        ci.iso3     = e.at("iso3").get<std::string>();
        ci.name     = e.at("name").get<std::string>();
        ci.name_zh  = e.value("name_zh", "");
        countries_[ci.id] = ci;
        iso2_to_id_[ci.iso2] = ci.id;
    }
}

void POIMeta::loadRailwayLayoutsFromJSON(const std::string& data_dir) {
    std::string path = data_dir + "/railway_layouts.json";
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("Cannot open " + path);

    nlohmann::json j = nlohmann::json::parse(f);

    for (auto it = j.begin(); it != j.end(); ++it) {
        const std::string& key = it.key();
        if (key == "_schema" || key == "_comment") continue;

        auto cid = countryId(key);
        if (!cid)
            throw std::runtime_error("railway_layouts.json: unknown country '" + key + "'");

        const nlohmann::json& v = it.value();

        RailwayLayoutInfo layout;
        std::string strategy = v.value("strategy", "");

        if (strategy == "range" || v.contains("partitions")) {
            assert(v.contains("partitions"));

            for (const auto& p : v["partitions"]) {
                assert(p.contains("min"));
                assert(p.contains("max"));
                assert(p.contains("code"));
                layout.partition_range.push_back(p.at("min").get<uint16_t>());
                layout.partition_op.push_back(p.at("code").get<std::string>());
            }
            // Detect strategy: explicit "strategy" field > "range" (multi-partition)
            layout.strategy = strategy.empty() ? "range" : strategy;

        } else if (strategy == "operator_line") {

            assert(v.contains("op_bits"));
            assert(v.contains("line_bits"));

            layout.op_bits   = v.value("op_bits", 0);
            layout.line_bits = v.value("line_bits", 16);
            layout.partition_op    = {};
            layout.partition_range = {};
            layout.strategy = strategy.empty() ? "operator_line" : strategy;

        } else {

            throw std::invalid_argument("Unexpected dividing strategy: " + strategy);
        
        }

        layouts_[*cid] = std::move(layout);

    }
}

void POIMeta::loadOperatorsFromJSON(const std::string& data_dir) {
    std::string path = data_dir + "/railway_operators.json";
    std::ifstream f(path);
    if (!f.is_open()) return;   // file is optional

    nlohmann::json j = nlohmann::json::parse(f);
    for (auto it = j.begin(); it != j.end(); ++it) {
        auto cid = countryId(it.key());
        if (!cid) continue;     // skip unknown countries

        for (const auto& e : it.value()) {
            RailwayOperatorInfo op;
            op.country_id = *cid;
            op.op_id      = e.value("id", 0);
            op.code       = e.value("code", "");
            op.name_en    = e.value("en", e.value("name_en", ""));
            op.name_zh    = e.value("zh", e.value("name_zh", ""));

            uint32_t key = (static_cast<uint32_t>(op.country_id) << 8) | op.op_id;
            operators_by_id_[key] = op;

            std::string code_key = std::to_string(op.country_id) + ":" + op.code;
            op_code_index_[code_key] = {op.country_id, op.op_id};
        }
    }
}

void POIMeta::loadCountriesFromDB(soci::session& db) {
    soci::rowset<soci::row> rs = (db.prepare << "SELECT id, iso2, iso3, name, name_zh FROM country");
    for (auto it = rs.begin(); it != rs.end(); ++it) {
        CountryInfo ci;
        ci.id      = it->get<int>(0);
        ci.iso2    = it->get<std::string>(1);
        ci.iso3    = it->get<std::string>(2);
        ci.name     = it->get<std::string>(3);
        ci.name_zh  = it->get<std::string>(4, "");
        countries_[ci.id] = ci;
        iso2_to_id_[ci.iso2] = ci.id;
    }
}

void POIMeta::loadRailwayLayoutsFromDB(soci::session& db) {
    soci::rowset<soci::row> rs = (db.prepare <<
        "SELECT country_id, part_idx, min_val, max_val, op_name, layout, op_bits, line_bits "
        "FROM railway_layout ORDER BY country_id, part_idx");

    for (auto it = rs.begin(); it != rs.end(); ++it) {
        uint8_t  cid    = it->get<int>(0);
        uint16_t max_val = static_cast<uint16_t>(it->get<int>(3));
        std::string op_name = it->get<std::string>(4, "");
        std::string layout  = it->get<std::string>(5);

        auto& info = layouts_[cid];

        if (layout == "operator_line") {
            info.strategy  = "operator_line";
            info.op_bits   = static_cast<uint16_t>(it->get<int>(6));
            info.line_bits = static_cast<uint16_t>(it->get<int>(7));
        } else {
            // "flat" partition — part of a range-based layout
            info.strategy = "range";
            info.partition_range.push_back(max_val);
            info.partition_op.push_back(op_name);
        }
    }
}

void POIMeta::loadOperatorsFromDB(soci::session& db) {
    soci::rowset<soci::row> rs = (db.prepare <<
        "SELECT country_id, op_id, code, name_en, name_zh FROM railway_operator");

    for (auto it = rs.begin(); it != rs.end(); ++it) {
        RailwayOperatorInfo op;
        op.country_id = static_cast<uint8_t>(it->get<int>(0));
        op.op_id      = static_cast<uint8_t>(it->get<int>(1));
        op.code       = it->get<std::string>(2);
        op.name_en    = it->get<std::string>(3);
        op.name_zh    = it->get<std::string>(4, "");

        uint32_t key = (static_cast<uint32_t>(op.country_id) << 8) | op.op_id;
        operators_by_id_[key] = op;

        std::string code_key = std::to_string(op.country_id) + ":" + op.code;
        op_code_index_[code_key] = {op.country_id, op.op_id};
    }
}

// ============================================================================
//  POIMeta — factory methods
// ============================================================================

POIMeta POIMeta::load(const std::string& data_dir) {
    POIMeta meta;
    meta.loadCountriesFromJSON(data_dir);
    meta.loadRailwayLayoutsFromJSON(data_dir);
    meta.loadOperatorsFromJSON(data_dir);
    return meta;
}

POIMeta POIMeta::loadFromDB(soci::session& db) {
    POIMeta meta;
    meta.loadCountriesFromDB(db);
    meta.loadRailwayLayoutsFromDB(db);
    meta.loadOperatorsFromDB(db);
    return meta;
}

void POIMeta::initDB(soci::session& db, const std::string& data_dir) {
    // Create metadata tables
    db << R"(
        CREATE TABLE IF NOT EXISTS country (
            id       INTEGER PRIMARY KEY,
            iso2     TEXT NOT NULL,
            iso3     TEXT NOT NULL,
            name     TEXT NOT NULL,
            name_zh  TEXT DEFAULT ''
        )
    )";
    db << R"(
        CREATE TABLE IF NOT EXISTS railway_layout (
            country_id INTEGER NOT NULL,
            part_idx   INTEGER NOT NULL,
            min_val    INTEGER NOT NULL,
            max_val    INTEGER NOT NULL,
            op_name    TEXT DEFAULT '',
            layout     TEXT NOT NULL,
            op_bits    INTEGER DEFAULT 0,
            line_bits  INTEGER DEFAULT 16,
            PRIMARY KEY (country_id, part_idx)
        )
    )";
    db << R"(
        CREATE TABLE IF NOT EXISTS railway_operator (
            country_id INTEGER NOT NULL,
            op_id      INTEGER NOT NULL,
            code       TEXT NOT NULL,
            name_en    TEXT NOT NULL,
            name_zh    TEXT DEFAULT '',
            PRIMARY KEY (country_id, op_id)
        )
    )";

    // Populate from JSON using a temporary POIMeta
    POIMeta tmp = POIMeta::load(data_dir);

    // Insert countries
    for (const auto& [id, ci] : tmp.countries_) {
        db << "INSERT OR REPLACE INTO country(id, iso2, iso3, name, name_zh) "
              "VALUES(:id, :iso2, :iso3, :name, :name_zh)",
              soci::use(ci.id), soci::use(ci.iso2), soci::use(ci.iso3),
              soci::use(ci.name), soci::use(ci.name_zh);
    }

    // Insert railway layouts
    for (const auto& [cid, layout] : tmp.layouts_) {
        if (layout.strategy == "operator_line") {
            db << "INSERT OR REPLACE INTO railway_layout "
                  "(country_id, part_idx, min_val, max_val, op_name, layout, op_bits, line_bits) "
                  "VALUES(:cid, 0, 0, 65535, '', 'operator_line', :opb, :lnb)",
                  soci::use(cid),
                  soci::use(layout.op_bits), soci::use(layout.line_bits);
        } else if (layout.strategy == "range") {
            for (size_t i = 0; i < layout.partition_range.size(); ++i) {
                uint16_t lo = (i == 0) ? 0 : (layout.partition_range[i - 1] + 1);
                uint16_t hi = layout.partition_range[i];
                db << "INSERT OR REPLACE INTO railway_layout "
                      "(country_id, part_idx, min_val, max_val, op_name, layout, op_bits, line_bits) "
                      "VALUES(:cid, :pidx, :min, :max, :op, 'flat', 0, 16)",
                      soci::use(cid), soci::use(static_cast<int>(i)),
                      soci::use(lo), soci::use(hi),
                      soci::use(layout.partition_op[i]);
            }
        }
    }

    // Insert operators
    for (const auto& [key, op] : tmp.operators_by_id_) {
        db << "INSERT OR REPLACE INTO railway_operator "
              "(country_id, op_id, code, name_en, name_zh) "
              "VALUES(:cid, :oid, :code, :name_en, :name_zh)",
              soci::use(op.country_id), soci::use(op.op_id),
              soci::use(op.code), soci::use(op.name_en), soci::use(op.name_zh);
    }
}

// ============================================================================
//  POIMeta — lookups
// ============================================================================

std::optional<uint8_t> POIMeta::countryId(const std::string& iso2) const {
    auto it = iso2_to_id_.find(iso2);
    if (it != iso2_to_id_.end()) return it->second;
    return std::nullopt;
}

const CountryInfo* POIMeta::countryInfo(uint8_t id) const {
    auto it = countries_.find(id);
    return it != countries_.end() ? &it->second : nullptr;
}

const std::unordered_map<uint8_t, CountryInfo>& POIMeta::allCountries() const {
    return countries_;
}

const RailwayLayoutInfo* POIMeta::railwayLayout(uint8_t country_id) const {
    auto it = layouts_.find(country_id);
    return it != layouts_.end() ? &it->second : nullptr;
}

uint16_t POIMeta::packRailway(uint8_t country_id, uint8_t op, uint16_t line) const {
    auto* layout = railwayLayout(country_id);
    if (!layout) return 0;

    if (layout->strategy == "flat") {
        return line;
    }

    if (layout->strategy == "operator_line") {
        uint8_t  op_max   = (1u << layout->op_bits) - 1;
        uint16_t line_max  = (1u << layout->line_bits) - 1;
        if (op <= op_max && line <= line_max)
            return static_cast<uint16_t>((static_cast<uint32_t>(op) << layout->line_bits) | line);
        return 0;
    }

    if (layout->strategy == "range") {
        auto* op_info = operatorInfo(country_id, op);
        if (!op_info) return 0;
        for (size_t i = 0; i < layout->partition_op.size(); ++i) {
            if (layout->partition_op[i] == op_info->code) {
                uint16_t lo = (i == 0) ? 0 : (layout->partition_range[i - 1] + 1);
                uint16_t hi = layout->partition_range[i];
                if (line <= static_cast<uint32_t>(hi - lo))
                    return lo + line;
                return 0;
            }
        }
        return 0;
    }

    return 0;
}

std::pair<uint8_t, uint16_t> POIMeta::unpackRailway(uint8_t country_id, uint16_t enc) const {
    auto* layout = railwayLayout(country_id);
    if (!layout) return {0, 0};

    if (layout->strategy == "flat") {
        return {0, enc};
    }

    if (layout->strategy == "operator_line") {
        uint16_t line_mask = (1u << layout->line_bits) - 1;
        return {static_cast<uint8_t>(enc >> layout->line_bits),
                static_cast<uint16_t>(enc & line_mask)};
    }

    if (layout->strategy == "range") {
        // Find partition by value range, then resolve operator
        for (size_t i = 0; i < layout->partition_range.size(); ++i) {
            uint16_t lo = (i == 0) ? 0 : (layout->partition_range[i - 1] + 1);
            uint16_t hi = layout->partition_range[i];
            if (enc >= lo && enc <= hi) {
                uint8_t op_id = 0;
                auto oid = operatorId(country_id, layout->partition_op[i]);
                if (oid) op_id = *oid;
                return {op_id, static_cast<uint16_t>(enc - lo)};
            }
        }
        return {0, 0};
    }

    return {0, 0};
}

const RailwayOperatorInfo* POIMeta::operatorInfo(uint8_t country_id, uint8_t op_id) const {
    uint32_t key = (static_cast<uint32_t>(country_id) << 8) | op_id;
    auto it = operators_by_id_.find(key);
    return it != operators_by_id_.end() ? &it->second : nullptr;
}

std::optional<uint8_t> POIMeta::operatorId(uint8_t country_id, const std::string& code) const {
    std::string key = std::to_string(country_id) + ":" + code;
    auto it = op_code_index_.find(key);
    if (it != op_code_index_.end()) return it->second.second;
    return std::nullopt;
}

std::string POIMeta::formatKey(uint64_t key) const {
    DecodedKey dk = decode(key);
    auto* ci = countryInfo(dk.country);
    std::string iso = ci ? ci->iso2 : "??";

    auto a1z26_to_str = [](const uint8_t* vals, int n) -> std::string {
        std::string s;
        for (int i = 0; i < n && vals[i] != 0; ++i)
            s += static_cast<char>('A' + vals[i] - 1);
        return s;
    };

    switch (dk.type) {
    case POIType::Airport: {
        std::string iata = a1z26_to_str(dk.iata_a1z26, 3);
        std::string icao = a1z26_to_str(dk.icao_a1z26, 4);
        if (!iata.empty() && !icao.empty())
            return "Airport: " + iso + "/" + iata + " (" + icao + ")";
        else if (!icao.empty())
            return "Airport: " + iso + "/" + icao;
        else
            return "Airport: " + iso + "/" + iata;
    }
    case POIType::RailwayStation:
        return "RailwayStation: " + iso + "/class=" + std::to_string(dk.station_class)
               + "/#" + std::to_string(dk.station_idx);

    case POIType::SubwayStation:
        return "SubwayStation: " + iso + "/city=" + std::to_string(dk.subway_city)
               + "/line=" + std::to_string(dk.subway_line)
               + "/stn=" + std::to_string(dk.subway_station);

    case POIType::Frequent:
        return "Frequent: cat=" + std::to_string(dk.freq_category)
               + "/#" + std::to_string(dk.freq_index);

    case POIType::RailwayWaypoint: {
        auto [op, line] = unpackRailway(dk.country, dk.rwp_line);
        auto* op_info = operatorInfo(dk.country, op);
        double km = dk.rwp_dist_m / 1000.0;
        char buf[256];
        snprintf(buf, sizeof(buf), "RailwayWP: %s/%s/line=%d @ %.3fkm",
                 iso.c_str(), op_info ? op_info->code.c_str() : "?",
                 line, km);
        return buf;
    }
    case POIType::SubwayWaypoint: {
        double km = dk.swp_dist_m / 1000.0;
        char buf[128];
        snprintf(buf, sizeof(buf), "SubwayWP: %s/city=%d/line=%d @ %.3fkm",
                 iso.c_str(), dk.swp_city, dk.swp_line, km);
        return buf;
    }
    default:
        return "Unknown type=0x" +
               std::to_string(static_cast<uint8_t>(dk.type));
    }
}

// ============================================================================
//  POIMeta — database update API
// ============================================================================

void POIMeta::upsertCountry(soci::session& db, const CountryInfo& ci) {
    // Remove old iso2→id mapping if iso2 changed
    auto old = countries_.find(ci.id);
    if (old != countries_.end() && old->second.iso2 != ci.iso2)
        iso2_to_id_.erase(old->second.iso2);

    // Write to DB
    db << "INSERT OR REPLACE INTO country(id, iso2, iso3, name, name_zh) "
          "VALUES(:id, :iso2, :iso3, :name, :name_zh)",
          soci::use(ci.id), soci::use(ci.iso2), soci::use(ci.iso3),
          soci::use(ci.name), soci::use(ci.name_zh);

    // Update in-memory
    countries_[ci.id] = ci;
    iso2_to_id_[ci.iso2] = ci.id;
}

void POIMeta::deleteCountry(soci::session& db, uint8_t id) {
    db << "DELETE FROM country WHERE id = :id", soci::use(id);
    db << "DELETE FROM railway_layout WHERE country_id = :id", soci::use(id);

    // Remove associated operators from code index
    uint32_t base = static_cast<uint32_t>(id) << 8;
    for (int op = 0; op < 256; ++op) {
        auto it = operators_by_id_.find(base | op);
        if (it != operators_by_id_.end()) {
            std::string code_key = std::to_string(id) + ":" + it->second.code;
            op_code_index_.erase(code_key);
            operators_by_id_.erase(it);
        }
    }
    db << "DELETE FROM railway_operator WHERE country_id = :id", soci::use(id);

    auto ci = countries_.find(id);
    if (ci != countries_.end()) iso2_to_id_.erase(ci->second.iso2);
    countries_.erase(id);
    layouts_.erase(id);
}

void POIMeta::upsertOperator(soci::session& db,
                              uint8_t country_id, uint8_t op_id,
                              const std::string& code, const std::string& name_en,
                              const std::string& name_zh) {
    // Remove old code index entry if code changed
    uint32_t key = (static_cast<uint32_t>(country_id) << 8) | op_id;
    auto old = operators_by_id_.find(key);
    if (old != operators_by_id_.end() && old->second.code != code) {
        std::string old_key = std::to_string(country_id) + ":" + old->second.code;
        op_code_index_.erase(old_key);
    }

    // Write to DB
    db << "INSERT OR REPLACE INTO railway_operator "
          "(country_id, op_id, code, name_en, name_zh) "
          "VALUES(:cid, :oid, :code, :name_en, :name_zh)",
          soci::use(country_id), soci::use(op_id),
          soci::use(code), soci::use(name_en), soci::use(name_zh);

    // Update in-memory
    RailwayOperatorInfo op{country_id, op_id, code, name_en, name_zh};
    operators_by_id_[key] = op;
    std::string code_key = std::to_string(country_id) + ":" + code;
    op_code_index_[code_key] = {country_id, op_id};
}

void POIMeta::deleteOperator(soci::session& db, uint8_t country_id, uint8_t op_id) {
    db << "DELETE FROM railway_operator WHERE country_id = :cid AND op_id = :oid",
          soci::use(country_id), soci::use(op_id);

    uint32_t key = (static_cast<uint32_t>(country_id) << 8) | op_id;
    auto it = operators_by_id_.find(key);
    if (it != operators_by_id_.end()) {
        std::string code_key = std::to_string(country_id) + ":" + it->second.code;
        op_code_index_.erase(code_key);
        operators_by_id_.erase(it);
    }
}

void POIMeta::setRailwayLayout(soci::session& db, uint8_t country_id,
                               const RailwayLayoutInfo& layout) {
    db << "DELETE FROM railway_layout WHERE country_id = :id", soci::use(country_id);

    if (layout.strategy == "operator_line") {
        db << "INSERT INTO railway_layout "
              "(country_id, part_idx, min_val, max_val, op_name, layout, op_bits, line_bits) "
              "VALUES(:cid, 0, 0, 65535, '', 'operator_line', :opb, :lnb)",
              soci::use(country_id),
              soci::use(layout.op_bits), soci::use(layout.line_bits);
    } else if (layout.strategy == "range") {
        for (size_t i = 0; i < layout.partition_range.size(); ++i) {
            uint16_t lo = (i == 0) ? 0 : (layout.partition_range[i - 1] + 1);
            uint16_t hi = layout.partition_range[i];
            db << "INSERT INTO railway_layout "
                  "(country_id, part_idx, min_val, max_val, op_name, layout, op_bits, line_bits) "
                  "VALUES(:cid, :pidx, :min, :max, :op, 'flat', 0, 16)",
                  soci::use(country_id), soci::use(static_cast<int>(i)),
                  soci::use(lo), soci::use(hi),
                  soci::use(layout.partition_op[i]);
        }
    }

    layouts_[country_id] = layout;
}

void POIMeta::reloadMetaFromJSON(soci::session& db, const std::string& data_dir) {
    db << "DROP TABLE IF EXISTS railway_operator";
    db << "DROP TABLE IF EXISTS railway_layout";
    db << "DROP TABLE IF EXISTS country";

    initDB(db, data_dir);

    clearMaps();
    loadCountriesFromDB(db);
    loadRailwayLayoutsFromDB(db);
    loadOperatorsFromDB(db);
}

// ============================================================================
//  POIDataBase implementation
// ============================================================================

POIDataBase::POIDataBase() {
    try {
        _poidb.open(soci::sqlite3, "");
    } catch (std::exception& e) {
        throw e;
    }
}

POIDataBase::~POIDataBase() {
    _poidb.close();
}

void POIDataBase::initPOIDataBase() {
    _poidb << R"(
        CREATE TABLE IF NOT EXISTS point (
            id INTEGER PRIMARY KEY,
            lat REAL,
            lon REAL,
            ele REAL,
            name TEXT,
            geohash TEXT
        )
    )";
    _poidb << "CREATE INDEX idx_geohash ON point(geohash);";
}

void POIDataBase::saveToCSV(const std::string& filename) const {
    // TODO
}

int POIDataBase::loadFromCSV(const std::string& filename) {
    // TODO
    return 0;
}

int POIDataBase::loadFromAirportCSV(const std::string& filename,
                                     uint8_t country_id,
                                     CoordinateSystem coord) {

    rapidcsv::Document doc(filename, rapidcsv::LabelParams(0, -1));
    int rowCount = doc.GetRowCount();

    std::vector<double> lats(rowCount), lons(rowCount), eles(rowCount);
    std::vector<int64_t> poikeys(rowCount);
    std::vector<std::string> geohashes(rowCount);
    std::vector<std::string> names(rowCount);

    for (int i = 0; i < rowCount; i++) {
        std::string airport_iata = doc.GetCell<std::string>("iata", i);
        std::string airport_icao = doc.GetCell<std::string>("icao", i);
        double lat = doc.GetCell<double>("lat", i);
        double lon = doc.GetCell<double>("lon", i);
        double ele = doc.GetCell<double>("ele", i);

        lats[i] = lat;
        lons[i] = lon;
        eles[i] = ele;
        names[i] = doc.GetCell<std::string>("name_zh", i);

        uint64_t key = encodeAirport(country_id, airport_iata, airport_icao);
        poikeys[i] = static_cast<int64_t>(key);
        geohashes[i] = encode_geohash(lat, lon, 8);
    }

    _poidb << "INSERT INTO point(lat, lon, ele, id, name, geohash) "
              "VALUES(:lat, :lon, :ele, :id, :name, :geohash)",
              soci::use(lats),
              soci::use(lons),
              soci::use(eles),
              soci::use(poikeys),
              soci::use(names),
              soci::use(geohashes);

    return rowCount;
}

int POIDataBase::loadFromRailwayCSV(const std::string& filename,
                                     uint8_t country_id,
                                     CoordinateSystem coord) {

    rapidcsv::Document doc(filename, rapidcsv::LabelParams(0, -1));
    int rowCount = doc.GetRowCount();

    std::vector<double> lats(rowCount), lons(rowCount), eles(rowCount);
    std::vector<int64_t> poikeys(rowCount);
    std::vector<std::string> geohashes(rowCount);
    std::vector<std::string> names(rowCount);

    for (int i = 0; i < rowCount; i++) {
        std::string station_code = doc.GetCell<std::string>("code", i);
        std::string station_name = doc.GetCell<std::string>("name", i);
        double lat = doc.GetCell<double>("lat", i);
        double lon = doc.GetCell<double>("lon", i);

        lats[i] = lat;
        lons[i] = lon;
        names[i] = station_name;
        geohashes[i] = encode_geohash(lat, lon, 8);

        uint64_t station_idx = static_cast<uint64_t>(i);
        uint8_t  cls = 0;

        uint64_t key = encodeRailwayStation(country_id, cls, station_idx);
        poikeys[i] = static_cast<int64_t>(key);
    }

    _poidb << "INSERT INTO point(lat, lon, ele, id, name, geohash) "
              "VALUES(:lat, :lon, :ele, :id, :name, :geohash)",
              soci::use(lats),
              soci::use(lons),
              soci::use(eles),
              soci::use(poikeys),
              soci::use(names),
              soci::use(geohashes);

    return rowCount;
}

void POIDataBase::insertPOI(double lat, double lon, POIType poitype,
                             const std::string& name, uint64_t poicode,
                             CoordinateSystem coord) {

    validateLatitude(lat);
    validateLongitude(lon);

    // TODO: full insert implementation
}

std::vector<POI> POIDataBase::query(double lat, double lon,
                                     uint8_t type_filter, double range) const {

    validateLatitude(lat);
    validateLongitude(lon);

    if (range > 5000.0) range = 5000.0;
    if (range < 150.0)  range = 150.0;

    // TODO: full query implementation
    return {};
}

} // namespace mygpx
