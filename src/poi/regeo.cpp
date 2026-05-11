#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "regeo.hpp"
#include "utils.hpp"
#include <httplib.h>
#include <tinyxml2.h>
#include <sstream>
#include <iomanip>

namespace mygpx {

std::string queryAddress(double latitude, double longitude) { 
    // TODO: query from other maps if Amap fails
    return queryAddressFromAmap(latitude, longitude, {});
}

std::string queryAddressFromAmap(double latitude, double longitude, std::vector<int> poitypes, double poi_radius) {

    auto [lat,lon] = wgs84_to_gcj02(latitude, longitude);
    httplib::Client cli("https://restapi.amap.com");

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << longitude << "," << latitude;

    httplib::Params params{
        {"key", getAmapAPIKey()},
        {"location", oss.str()},
        {"radius", std::to_string(static_cast<int>(poi_radius))},
        {"extensions", "all"},
        {"roadlevel", "0"},
        {"output", "XML"}
    };

    // add POI types in params
    if (!poitypes.empty()) {
        std::string typesStr;
        for (size_t i = 0; i < poitypes.size(); ++i) {
            typesStr += std::to_string(poitypes[i]);
            if (i != poitypes.size() - 1) typesStr += "|";
        }
        params.emplace("poitype", typesStr);
    }

    httplib::Headers headers;

    auto res = cli.Get("/v3/geocode/regeo", params, headers);

    if (!res) {
        // Check the error type
        const auto err = res.error();

        switch (err) {
        case httplib::Error::SSLConnection:
            throw std::runtime_error("SSL connection failed, SSL error: " + std::to_string(res.ssl_error()));
            break;

        case httplib::Error::SSLLoadingCerts:
            throw std::runtime_error("SSL cert loading failed, OpenSSL error: " + std::to_string(res.ssl_openssl_error()));
            break;

        case httplib::Error::SSLServerVerification:
            throw std::runtime_error("SSL verification failed, X509 error: " + std::to_string(res.ssl_openssl_error()));
            break;

        case httplib::Error::SSLServerHostnameVerification:
            throw std::runtime_error("SSL hostname verification failed, X509 error: " + std::to_string(res.ssl_openssl_error()));
            break;

        default:
            throw std::runtime_error("HTTP error: " + httplib::to_string(err));
        }
    } else {
        std::cout << "HTTP Status: " << res->status << std::endl;
    }

    tinyxml2::XMLDocument doc;

    if (doc.Parse(res->body.c_str()) != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Failed to parse XML");
    }

    auto* response = doc.FirstChildElement("response");
        if (!response) throw std::runtime_error("Missing label <response>");
    auto* status = response->FirstChildElement("status");
        if (!status) throw std::runtime_error("Missing label <status>");

    if (std::string(status->GetText()) != "1") {
        auto* info = response->FirstChildElement("info");
        auto* infocode = response->FirstChildElement("infocode");
        std::string infoText = info ? info->GetText() : "Unknown error";
        std::string infoCode = infocode ? infocode->GetText() : "Unknown code";
        throw std::runtime_error("Amap API error " + infoCode + ": " + infoText);
    }
    
    auto* regeocode = response->FirstChildElement("regeocode");
        if (!regeocode) throw std::runtime_error("Missing label <regeocode>");
    auto* address_component = regeocode->FirstChildElement("addressComponent");
        if (!address_component) throw std::runtime_error("Missing label <addressComponent>");

    std::string poi = "";
    if (regeocode->FirstChildElement("pois")&&regeocode->FirstChildElement("pois")->FirstChildElement("poi")) {
        auto* name = regeocode->FirstChildElement("pois")->FirstChildElement("poi")->FirstChildElement("name");
        if (name && name->GetText()) poi = name->GetText();
    }

    if (!poitypes.empty()) {
        if (!poi.empty()) return poi;
        else throw std::runtime_error("No POI found");
    }
    
    auto* country = address_component->FirstChildElement("country");
    auto* province = address_component->FirstChildElement("province");
    auto* city = address_component->FirstChildElement("city");
    auto* district = address_component->FirstChildElement("district");
    auto* township = address_component->FirstChildElement("township");
    auto* streetNumber = address_component->FirstChildElement("streetNumber");
    auto* street = streetNumber ? streetNumber->FirstChildElement("street") : nullptr;
    auto* number = streetNumber ? streetNumber->FirstChildElement("number") : nullptr;

    std::string address;
    if (country && country->GetText()) address += country->GetText();
    if (province && province->GetText()) address += province->GetText();
    if (city && city->GetText()) address += city->GetText();
    if (district && district->GetText()) address += district->GetText();
    if (township && township->GetText()) address += township->GetText();
    if (street && number && street->GetText() && number->GetText()) {
        address += street->GetText();
        address += number->GetText();
    }

    if (address.empty())
        throw std::runtime_error("No address in Amap response");

    if (!poi.empty()) return poi + ", " + address; 

    return address;

}

POI queryPOIFromAmap(double latitude, double longitude, std::vector<int> poitypes, double poi_radius) {

    auto [lat,lon] = wgs84_to_gcj02(latitude, longitude);
    httplib::Client cli("https://restapi.amap.com");

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << longitude << "," << latitude;

    httplib::Params params{
        {"key", getAmapAPIKey()},
        {"location", oss.str()},
        {"radius", std::to_string(static_cast<int>(poi_radius))},
        {"extensions", "all"},
        {"roadlevel", "0"},
        {"output", "XML"}
    };

    // add POI types in params
    if (!poitypes.empty()) {
        std::string typesStr;
        for (size_t i = 0; i < poitypes.size(); ++i) {
            typesStr += std::to_string(poitypes[i]);
            if (i != poitypes.size() - 1) typesStr += "|";
        }
        params.emplace("poitype", typesStr);
    }

    httplib::Headers headers;

    auto res = cli.Get("/v3/geocode/regeo", params, headers);

    if (!res) {
        // Check the error type
        const auto err = res.error();

        switch (err) {
        case httplib::Error::SSLConnection:
            throw std::runtime_error("SSL connection failed, SSL error: " + std::to_string(res.ssl_error()));
            break;

        case httplib::Error::SSLLoadingCerts:
            throw std::runtime_error("SSL cert loading failed, OpenSSL error: " + std::to_string(res.ssl_openssl_error()));
            break;

        case httplib::Error::SSLServerVerification:
            throw std::runtime_error("SSL verification failed, X509 error: " + std::to_string(res.ssl_openssl_error()));
            break;

        case httplib::Error::SSLServerHostnameVerification:
            throw std::runtime_error("SSL hostname verification failed, X509 error: " + std::to_string(res.ssl_openssl_error()));
            break;

        default:
            throw std::runtime_error("HTTP error: " + httplib::to_string(err));
        }
    } else {
        std::cout << "HTTP Status: " << res->status << std::endl;
    }

    tinyxml2::XMLDocument doc;

    if (doc.Parse(res->body.c_str()) != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Failed to parse XML");
    }

    auto* response = doc.FirstChildElement("response");
        if (!response) throw std::runtime_error("Missing label <response>");
    auto* status = response->FirstChildElement("status");
        if (!status) throw std::runtime_error("Missing label <status>");

    if (std::string(status->GetText()) != "1") {
        auto* info = response->FirstChildElement("info");
        auto* infocode = response->FirstChildElement("infocode");
        std::string infoText = info ? info->GetText() : "Unknown error";
        std::string infoCode = infocode ? infocode->GetText() : "Unknown code";
        throw std::runtime_error("Amap API error " + infoCode + ": " + infoText);
    }
    
    auto* regeocode = response->FirstChildElement("regeocode");
        if (!regeocode) throw std::runtime_error("Missing label <regeocode>");
    auto* address_component = regeocode->FirstChildElement("addressComponent");
        if (!address_component) throw std::runtime_error("Missing label <addressComponent>");

    POI ret_poi;
    if (auto* pois = regeocode->FirstChildElement("pois"))
        if (auto* poi = pois->FirstChildElement("poi")) {
            auto* name = poi->FirstChildElement("name");
            auto* distance = poi->FirstChildElement("distance");
            auto* longitude = poi->FirstChildElement("longitude");
            auto* latitude = poi->FirstChildElement("latitude");
            if (name && name->GetText()) ret_poi.name = name->GetText(); else ret_poi.name = "";
            if (distance && distance->GetText()) ret_poi.distance = std::stod(distance->GetText());
            if (longitude && longitude->GetText()) ret_poi.lon = std::stod(longitude->GetText());
            if (latitude && latitude->GetText()) ret_poi.lat = std::stod(latitude->GetText());
        }

    auto [ret_lat, ret_lon] = gcj02_to_wgs84(ret_poi.lat, ret_poi.lon);
    ret_poi.lat = ret_lat;
    ret_poi.lon = ret_lon;

    if (ret_poi.name.empty()) throw std::runtime_error("No POI found");
    return ret_poi;

}

POI queryRailwayStation(double latitude, double longitude, double radius) {
    POI amap_result = queryPOIFromAmap(latitude, longitude, {150200,150210}, radius);

    return amap_result;
}

POI queryBusStation(double latitude, double longitude, double radius) {
    POI amap_result = queryPOIFromAmap(latitude, longitude, {150400,150701,150702,150703,150704,150705,150706}, radius);
    return amap_result;
}

POI queryAirport(double latitude, double longitude, double radius) {
    POI amap_result = queryPOIFromAmap(latitude, longitude, {150104}, radius);
    return amap_result;
}

POI querySubwayStation(double latitude, double longitude, double radius) {
    POI amap_result = queryPOIFromAmap(latitude, longitude, {150500}, radius);
    return amap_result;
}


}
