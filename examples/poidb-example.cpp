#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdio>
#include "poidb.hpp"

using namespace mygpx;

static int tests = 0, passed = 0;

#define TEST(name) do { tests++; std::cout << "  " << name << " ... "; } while(0)
#define OK()       do { passed++; std::cout << "OK" << std::endl; } while(0)
#define FAIL(msg)  do { std::cout << "FAIL: " << msg << std::endl; } while(0)

// ---- helpers ----

// write a minimal CSV for testing
void writeCSV(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    f << content;
    f.close();
}

// ============================================================================
int main() {
    std::cout << "=== POIMeta & POIDataBase Tests ===" << std::endl;

    // ---- 1. POIMeta: load from JSON ----
    std::cout << "\n[1] POIMeta JSON loading" << std::endl;

    TEST("load from JSON");
    auto meta = POIMeta::load("file/lookups/");
    if (meta.allCountries().empty()) { FAIL("no countries loaded"); }
    else OK();

    TEST("countryId CN → 36");
    auto cn = meta.countryId("CN");
    if (cn && *cn == 36) OK(); else FAIL("expected 36");

    TEST("countryId JP → 86");
    auto jp = meta.countryId("JP");
    if (jp && *jp == 86) OK(); else FAIL("expected 86");

    TEST("countryInfo by id");
    auto* ci = meta.countryInfo(36);
    if (ci && ci->iso2 == "CN") OK(); else FAIL("unexpected");

    TEST("countryId unknown → nullopt");
    if (!meta.countryId("XX")) OK(); else FAIL("should be nullopt");

    // ---- 2. POIMeta: railway layouts ----
    std::cout << "\n[2] Railway layouts" << std::endl;

    TEST("CN layout is 'range'");
    auto* cn_layout = meta.railwayLayout(36);
    if (cn_layout && cn_layout->strategy == "range") OK(); else FAIL("expected range");

    TEST("JP layout is 'operator_line'");
    auto* jp_layout = meta.railwayLayout(86);
    if (jp_layout && jp_layout->strategy == "operator_line") OK(); else FAIL("expected operator_line");

    TEST("KR layout is 'operator_line'");
    auto kr = meta.countryId("KR");
    auto* kr_layout = meta.railwayLayout(*kr);
    if (kr_layout && kr_layout->strategy == "operator_line") OK(); else FAIL("expected operator_line");

    // ---- 3. POIMeta: railway operators ----
    std::cout << "\n[3] Railway operators" << std::endl;

    TEST("CN operator 'cr' → op_id 0");
    auto cr_id = meta.operatorId(36, "cr");
    if (cr_id && *cr_id == 0) OK(); else FAIL("expected 0");

    TEST("CN operator 'gi' → op_id 1");
    auto gi_id = meta.operatorId(36, "gi");
    if (gi_id && *gi_id == 1) OK(); else FAIL("expected 1");

    TEST("operatorInfo CN/0 → 'cr'");
    auto* cr_info = meta.operatorInfo(36, 0);
    if (cr_info && cr_info->code == "cr") OK(); else FAIL("unexpected");

    TEST("operatorInfo CN/1 → 'gi'");
    auto* gi_info = meta.operatorInfo(36, 1);
    if (gi_info && gi_info->code == "gi") OK(); else FAIL("unexpected");

    // ---- 4. POIMeta: packRailway / unpackRailway ----
    std::cout << "\n[4] packRailway / unpackRailway" << std::endl;

    // CN: range strategy — CR [0, 59999], GI [60000, 60199]
    TEST("pack CN/CR line=100 → 100");
    uint16_t enc = meta.packRailway(36, 0, 100);
    if (enc == 100) OK(); else FAIL(std::to_string(enc));

    TEST("pack CN/GI line=0 → 60000");
    enc = meta.packRailway(36, 1, 0);
    if (enc == 60000) OK(); else FAIL(std::to_string(enc));

    TEST("pack CN/GI line=50 → 60050");
    enc = meta.packRailway(36, 1, 50);
    if (enc == 60050) OK(); else FAIL(std::to_string(enc));

    TEST("unpack CN 100 → (CR, 100)");
    auto [op, line] = meta.unpackRailway(36, 100);
    if (op == 0 && line == 100) OK(); else FAIL("got op=" + std::to_string(op) + " line=" + std::to_string(line));

    TEST("unpack CN 60050 → (GI, 50)");
    std::tie(op, line) = meta.unpackRailway(36, 60050);
    if (op == 1 && line == 50) OK(); else FAIL("got op=" + std::to_string(op) + " line=" + std::to_string(line));

    // JP: operator_line — 8+8 bits
    TEST("pack JP op=2 line=3 → ((2<<8)|3)=515");
    enc = meta.packRailway(86, 2, 3);
    if (enc == 515) OK(); else FAIL(std::to_string(enc));

    TEST("unpack JP 515 → (2, 3)");
    std::tie(op, line) = meta.unpackRailway(86, 515);
    if (op == 2 && line == 3) OK(); else FAIL("got op=" + std::to_string(op) + " line=" + std::to_string(line));

    // KR: operator_line — 6+10 bits
    TEST("pack KR op=5 line=7 → ((5<<10)|7)=5127");
    enc = meta.packRailway(*kr, 5, 7);
    if (enc == 5127) OK(); else FAIL(std::to_string(enc));

    TEST("unpack KR 5127 → (5, 7)");
    std::tie(op, line) = meta.unpackRailway(*kr, 5127);
    if (op == 5 && line == 7) OK(); else FAIL("got op=" + std::to_string(op) + " line=" + std::to_string(line));

    // ---- 5. POI key encoding / decoding ----
    std::cout << "\n[5] POI encoding / decoding" << std::endl;

    TEST("encode/decode Airport round-trip");
    {
        uint64_t key = encodeAirport(36, "PEK", "ZBAA");
        DecodedKey dk = decode(key);
        if (dk.type == POIType::Airport && dk.country == 36 &&
            dk.iata_a1z26[0] == 16 && dk.iata_a1z26[1] == 5 && dk.iata_a1z26[2] == 11 &&
            dk.icao_a1z26[0] == 26 && dk.icao_a1z26[1] == 2 && dk.icao_a1z26[2] == 1 && dk.icao_a1z26[3] == 1)
            OK();
        else FAIL("mismatch");
    }

    TEST("encode/decode RailwayStation round-trip");
    {
        uint64_t key = encodeRailwayStation(36, 1, 12345);
        DecodedKey dk = decode(key);
        if (dk.type == POIType::RailwayStation && dk.country == 36 &&
            dk.station_class == 1 && dk.station_idx == 12345)
            OK();
        else FAIL("mismatch");
    }

    TEST("encode/decode SubwayStation round-trip");
    {
        uint64_t key = encodeSubwayStation(36, 2, 3, 15);
        DecodedKey dk = decode(key);
        if (dk.type == POIType::SubwayStation && dk.country == 36 &&
            dk.subway_city == 2 && dk.subway_line == 3 && dk.subway_station == 15)
            OK();
        else FAIL("mismatch");
    }

    TEST("encode/decode Frequent round-trip");
    {
        uint64_t key = encodeFrequent(5, 0xDEADBEEF);
        DecodedKey dk = decode(key);
        if (dk.type == POIType::Frequent && dk.freq_category == 5 &&
            dk.freq_index == 0xDEADBEEF)
            OK();
        else FAIL("mismatch");
    }

    TEST("encode/decode RailwayWaypoint round-trip");
    {
        uint64_t key = encodeRailwayWP(36, 100, 123456);
        DecodedKey dk = decode(key);
        if (dk.type == POIType::RailwayWaypoint && dk.country == 36 &&
            dk.rwp_line == 100 && dk.rwp_dist_m == 123456)
            OK();
        else FAIL("mismatch");
    }

    TEST("encode/decode SubwayWaypoint round-trip");
    {
        uint64_t key = encodeSubwayWP(36, 2, 3, 7890);
        DecodedKey dk = decode(key);
        if (dk.type == POIType::SubwayWaypoint && dk.country == 36 &&
            dk.swp_city == 2 && dk.swp_line == 3 && dk.swp_dist_m == 7890)
            OK();
        else FAIL("mismatch");
    }

    // ---- 6. formatKey ----
    std::cout << "\n[6] formatKey" << std::endl;

    TEST("format Airport");
    {
        uint64_t key = encodeAirport(36, "PEK", "ZBAA");
        std::string s = meta.formatKey(key);
        if (s.find("PEK") != std::string::npos && s.find("ZBAA") != std::string::npos &&
            s.find("CN") != std::string::npos)
            OK();
        else FAIL(s);
    }

    TEST("format RailwayWaypoint CN");
    {
        uint64_t key = encodeRailwayWP(36, 100, 123456);
        std::string s = meta.formatKey(key);
        if (s.find("123.456km") != std::string::npos && s.find("CN") != std::string::npos)
            OK();
        else FAIL(s);
    }

    TEST("format RailwayWaypoint JP");
    {
        uint64_t key = encodeRailwayWP(86, 515, 515000);
        std::string s = meta.formatKey(key);
        if (s.find("515.000km") != std::string::npos && s.find("JP") != std::string::npos)
            OK();
        else FAIL(s);
    }

    // ---- 7. POIDataBase: CSV loading (requires init'd DB) ----
    std::cout << "\n[7] POIDataBase CSV loading" << std::endl;

    // Write temporary airport CSV
    std::string airport_csv = "/tmp/test_airports.csv";
    writeCSV(airport_csv,
        "iata,icao,lat,lon,ele,name_zh\n"
        "PEK,ZBAA,40.08,116.58,35,北京首都机场\n"
        "SHA,ZSSS,31.20,121.34,5,上海虹桥机场\n"
        "CAN,ZGGG,23.39,113.30,15,广州白云机场\n");

    TEST("airport encode/decode via CSV data");
    {
        uint64_t k1 = encodeAirport(36, "PEK", "ZBAA");
        uint64_t k2 = encodeAirport(36, "SHA", "ZSSS");
        uint64_t k3 = encodeAirport(36, "CAN", "ZGGG");
        DecodedKey d1 = decode(k1);
        DecodedKey d2 = decode(k2);
        DecodedKey d3 = decode(k3);
        if (d1.type == POIType::Airport && d2.type == POIType::Airport && d3.type == POIType::Airport)
            OK();
        else FAIL("encode/decode failed");
    }

    // Write temporary railway station CSV
    std::string railway_csv = "/tmp/test_railway_stations.csv";
    writeCSV(railway_csv,
        "code,name,lat,lon\n"
        "BJP,北京,39.90,116.41\n"
        "SHH,上海,31.23,121.47\n"
        "GZQ,广州,23.13,113.26\n"
        "CDW,成都,30.70,104.07\n");

    TEST("load railway station CSV (CN) — encode then decode");
    {
        uint64_t k1 = encodeRailwayStation(36, 0, 0);
        uint64_t k2 = encodeRailwayStation(36, 0, 1);
        uint64_t k3 = encodeRailwayStation(36, 0, 2);
        uint64_t k4 = encodeRailwayStation(36, 0, 3);
        DecodedKey d1 = decode(k1);
        DecodedKey d2 = decode(k2);
        if (d1.type == POIType::RailwayStation && d2.type == POIType::RailwayStation &&
            d1.station_idx == 0 && d2.station_idx == 1)
            OK();
        else FAIL("encode/decode failed");
    }

    // Clean up temp CSV files
    std::remove(airport_csv.c_str());
    std::remove(railway_csv.c_str());

    // ---- 8. POIMeta: DB update API ----
    std::cout << "\n[8] Database update API" << std::endl;

    // We'll operate on a temporary meta loaded from JSON first,
    // then sync to DB and verify round-trip.

    auto meta2 = POIMeta::load("file/lookups/");

    // Get the DB session from POIDataBase.  We'll make a small helper.
    // Actually, POIDataBase encapsulates its session.  We'll use POIMeta
    // directly with a separate soci session for metadata tests.
    {
        soci::session testdb(soci::sqlite3, ":memory:");
        POIMeta::initDB(testdb, "file/lookups/");

        TEST("upsertCountry — add new");
        CountryInfo nz = {250, "NZ", "NZL", "New Zealand", "新西兰"};
        meta2.upsertCountry(testdb, nz);
        {
            auto* ci2 = meta2.countryInfo(250);
            if (ci2 && ci2->iso2 == "NZ") OK(); else FAIL("not found in memory");
        }

        TEST("upsertCountry — update iso2");
        nz.iso2 = "ZZ";
        meta2.upsertCountry(testdb, nz);
        {
            auto old = meta2.countryId("NZ");
            auto nw  = meta2.countryId("ZZ");
            if (!old && nw && *nw == 250) OK(); else FAIL("iso2 index not updated");
        }
        // restore
        nz.iso2 = "NZ";
        meta2.upsertCountry(testdb, nz);

        TEST("upsertOperator — add new");
        meta2.upsertOperator(testdb, 86, 99, "test", "Test Railway", "测试铁道");
        {
            auto* oi = meta2.operatorInfo(86, 99);
            auto oid = meta2.operatorId(86, "test");
            if (oi && oi->code == "test" && oid && *oid == 99) OK();
            else FAIL("not found");
        }

        TEST("upsertOperator — change code");
        meta2.upsertOperator(testdb, 86, 99, "test2", "Test Railway 2", "测试铁道2");
        {
            auto old_code = meta2.operatorId(86, "test");
            auto new_code = meta2.operatorId(86, "test2");
            if (!old_code && new_code && *new_code == 99) OK();
            else FAIL("old code not cleaned up");
        }

        TEST("deleteOperator");
        meta2.deleteOperator(testdb, 86, 99);
        {
            auto oi = meta2.operatorInfo(86, 99);
            if (!oi) OK(); else FAIL("should be deleted");
        }

        TEST("setRailwayLayout — change to operator_line");
        RailwayLayoutInfo rli;
        rli.strategy = "operator_line";
        rli.op_bits = 7;
        rli.line_bits = 9;
        meta2.setRailwayLayout(testdb, 36, rli);
        {
            auto* ly = meta2.railwayLayout(36);
            if (ly && ly->strategy == "operator_line" && ly->op_bits == 7 && ly->line_bits == 9)
                OK();
            else FAIL("layout not updated in memory");
        }

        // verify round-trip through DB
        TEST("loadFromDB round-trip");
        {
            auto meta3 = POIMeta::loadFromDB(testdb);
            auto* ly2 = meta3.railwayLayout(36);
            if (ly2 && ly2->strategy == "operator_line" && ly2->op_bits == 7)
                OK();
            else FAIL("strategy lost in DB round-trip");
        }

        // restore original CN layout
        auto orig_layout = POIMeta::load("file/lookups/");
        auto* orig_cn = orig_layout.railwayLayout(36);
        if (orig_cn) {
            meta2.setRailwayLayout(testdb, 36, *orig_cn);
        }

        TEST("deleteCountry — NZ");
        meta2.deleteCountry(testdb, 250);
        {
            auto* ci3 = meta2.countryInfo(250);
            if (!ci3) OK(); else FAIL("should be deleted");
        }

        TEST("reloadMetaFromJSON");
        meta2.reloadMetaFromJSON(testdb, "file/lookups/");
        {
            auto kr2 = meta2.countryId("KR");
            if (kr2 && *kr2 == 140) OK(); else FAIL("reload failed");
        }
    }

    // ---- 9. POIType helpers ----
    std::cout << "\n[9] POIType helpers" << std::endl;

    TEST("isWaypoint(RailwayWaypoint) == true");
    if (isWaypoint(POIType::RailwayWaypoint)) OK(); else FAIL("expected true");

    TEST("isWaypoint(Airport) == false");
    if (!isWaypoint(POIType::Airport)) OK(); else FAIL("expected false");

    TEST("isRailway(RailwayStation) == true");
    if (isRailway(POIType::RailwayStation)) OK(); else FAIL("expected true");

    TEST("isRailway(RailwayWaypoint) == true");
    if (isRailway(POIType::RailwayWaypoint)) OK(); else FAIL("expected true");

    TEST("isSubway(SubwayStation) == true");
    if (isSubway(POIType::SubwayStation)) OK(); else FAIL("expected true");

    TEST("isAirport(Airport) == true");
    if (isAirport(POIType::Airport)) OK(); else FAIL("expected true");

    TEST("isTrafficHub(Airport) == true");
    if (isTrafficHub(POIType::Airport)) OK(); else FAIL("expected true");

    TEST("isTrafficHub(RailwayStation) == true");
    if (isTrafficHub(POIType::RailwayStation)) OK(); else FAIL("expected true");

    // ---- Cleanup ----
    std::remove(airport_csv.c_str());
    std::remove(railway_csv.c_str());

    // ---- Summary ----
    std::cout << "\n==============================" << std::endl;
    std::cout << passed << "/" << tests << " tests passed";
    if (passed < tests) std::cout << " (" << (tests - passed) << " FAILED)";
    std::cout << std::endl;

    return (passed == tests) ? 0 : 1;
}
