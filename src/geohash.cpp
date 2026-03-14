#include "geohash.hpp"
#include <array>
#include <cctype>
#include <cmath>
#include <set>

namespace mygpx {

    namespace {

        constexpr char BASE32[] = "0123456789bcdefghjkmnpqrstuvwxyz";
        constexpr int MAX_GEOHASH_LEN = 12;

        const std::array<int8_t, 256> BASE32_DECODE = []() {
            std::array<int8_t, 256> t{};
            t.fill(-1);
            for (int i = 0; i < 32; i++) {
                const unsigned char c = static_cast<unsigned char>(BASE32[i]);
                t[c] = static_cast<int8_t>(i);
                if (c >= 'a' && c <= 'z') {
                    t[c - 'a' + 'A'] = static_cast<int8_t>(i);
                }
            }
            return t;
        }();

        std::string normalize_geohash(const std::string& geohash) {
            std::string out;
            out.reserve(geohash.size());
            for (unsigned char c : geohash) {
                if (std::isspace(c)) {
                    continue;
                }
                if (c >= 'A' && c <= 'Z') {
                    out.push_back(static_cast<char>(c - 'A' + 'a'));
                } else {
                    out.push_back(static_cast<char>(c));
                }
            }
            return out;
        }

        void validate_geohash_length(const std::string& geohash) {
            validateInRange(geohash.size(), static_cast<size_t>(1), static_cast<size_t>(MAX_GEOHASH_LEN));
        }

        GeoHashDecoded decode_geohash_info_impl(const std::string& geohash_norm) {
            double lat_min = -90.0, lat_max = 90.0;
            double lon_min = -180.0, lon_max = 180.0;

            bool is_lon = true;

            for (unsigned char c : geohash_norm) {
                const int cd = BASE32_DECODE[c];
                if (cd < 0) {
                    throw std::invalid_argument("Invalid geohash character");
                }
                for (int mask = 16; mask > 0; mask >>= 1) {
                    if (is_lon) {
                        double mid = (lon_min + lon_max) * 0.5;
                        if (cd & mask) {
                            lon_min = mid;
                        } else {
                            lon_max = mid;
                        }
                    } else {
                        double mid = (lat_min + lat_max) * 0.5;
                        if (cd & mask) {
                            lat_min = mid;
                        } else {
                            lat_max = mid;
                        }
                    }
                    is_lon = !is_lon;
                }
            }

            double latitude  = (lat_min + lat_max) * 0.5;
            double longitude = (lon_min + lon_max) * 0.5;
            double lat_err = (lat_max - lat_min) * 0.5;
            double lon_err = (lon_max - lon_min) * 0.5;

            return {{latitude, longitude}, {lat_min, lat_max, lon_min, lon_max}, {lat_err, lon_err}, static_cast<int>(geohash_norm.size())};
        }

        double meters_per_deg_lat() {
            return 111320.0;
        }

        double meters_per_deg_lon(double lat_deg) {
            return 111320.0 * std::cos(lat_deg * PI / 180.0);
        }

        void validate_bbox(GeoBBox bbox) {
            auto [lat_min, lat_max, lon_min, lon_max] = bbox;
            validateLatitude(lat_min);
            validateLatitude(lat_max);
            validateLongitude(lon_min);
            validateLongitude(lon_max);
            if (lat_min > lat_max) {
                throw std::invalid_argument("Invalid bbox: lat_min > lat_max");
            }
            if (lon_min > lon_max) {
                throw std::invalid_argument("Invalid bbox: lon_min > lon_max");
            }
        }

        std::pair<double, double> cell_size_degrees(int precision) {
            const int total_bits = precision * 5;
            const int lon_bits = (total_bits + 1) / 2;
            const int lat_bits = total_bits / 2;
            const double lat_height = 180.0 / (1 << lat_bits);
            const double lon_width = 360.0 / (1 << lon_bits);
            return {lat_height, lon_width};
        }

    }

    std::string encode_geohash(double latitude, double longitude, int precision) {

        validateLatitude(latitude);
        validateLongitude(longitude);
        return encode_geohash(std::make_pair(latitude, longitude), precision);

    }

    std::string encode_geohash(GeoPoint gp, int precision) {
        std::string geohash;
        geohash.reserve(precision);

        validateInRange((size_t)precision, (size_t)1, (size_t)MAX_GEOHASH_LEN);

        auto [lat, lon] = gp;
        double lat_min = -90.0, lat_max = 90.0;
        double lon_min = -180.0, lon_max = 180.0;

        bool is_lon = true;   // bit 0 从 longitude 开始
        int bit = 0;
        int ch = 0;

        while ((int)geohash.size() < precision) {
            if (is_lon) {
                double mid = (lon_min + lon_max) * 0.5;
                if (lon >= mid) {
                    ch = (ch << 1) | 1;
                    lon_min = mid;
                } else {
                    ch <<= 1;
                    lon_max = mid;
                }
            } else {
                double mid = (lat_min + lat_max) * 0.5;
                if (lat >= mid) {
                    ch = (ch << 1) | 1;
                    lat_min = mid;
                } else {
                    ch <<= 1;
                    lat_max = mid;
                }
            }

            is_lon = !is_lon;
            bit++;

            if (bit == 5) {
                geohash.push_back(BASE32[ch]);
                bit = 0;
                ch = 0;
            }
        }

        return geohash;
    }

    GeoPoint decode_geohash(const std::string& geohash) {
        std::string norm = normalize_geohash(geohash);
        validate_geohash_length(norm);
        GeoHashDecoded info = decode_geohash_info_impl(norm);
        return info.center;
    }

    GeoBBox decode_geohash_bbox(const std::string& geohash) {
        std::string norm = normalize_geohash(geohash);
        validate_geohash_length(norm);
        GeoHashDecoded info = decode_geohash_info_impl(norm);
        return info.bbox;
    }

    GeoHashDecoded decode_geohash_info(const std::string& geohash) {
        std::string norm = normalize_geohash(geohash);
        validate_geohash_length(norm);
        return decode_geohash_info_impl(norm);
    }

    std::string south(const std::string& geohash) {
        auto [lat_min, lat_max, lon_min, lon_max] = decode_geohash_bbox(geohash);

        size_t level = geohash.size();
        double lat = (lat_min + lat_max) / 2;
        double lon = (lon_min + lon_max) / 2;
        lat -= (lat_max - lat_min);
        if (lat < -90.0) throw std::out_of_range("South neighbor crosses the south pole");
        else return encode_geohash(lat, lon, level);
    }
    std::string north(const std::string& geohash) {
        auto [lat_min, lat_max, lon_min, lon_max] = decode_geohash_bbox(geohash);

        size_t level = geohash.size();
        double lat = (lat_min + lat_max) / 2;
        double lon = (lon_min + lon_max) / 2;
        lat += (lat_max - lat_min);
        if (lat > 90.0) throw std::out_of_range("North neighbor crosses the north pole");
        else return encode_geohash(lat, lon, level);
    }
    std::string west(const std::string& geohash) {
        auto [lat_min, lat_max, lon_min, lon_max] = decode_geohash_bbox(geohash);

        size_t level = geohash.size();
        double lat = (lat_min + lat_max) / 2;
        double lon = (lon_min + lon_max) / 2;
        lon -= (lon_max - lon_min);
        if (lon < -180.0) lon += 360.0;
        return encode_geohash(lat, lon, level);
    }
    std::string east(const std::string& geohash) {
        auto [lat_min, lat_max, lon_min, lon_max] = decode_geohash_bbox(geohash);

        size_t level = geohash.size();
        double lat = (lat_min + lat_max) / 2;
        double lon = (lon_min + lon_max) / 2;
        lon += (lon_max - lon_min);
        if (lon > 180.0) lon -= 360.0;
        return encode_geohash(lat, lon, level);
    }

    std::array<std::string, 8> neighbors(const std::string& geohash) {
        std::string norm = normalize_geohash(geohash);
        validate_geohash_length(norm);
        std::array<std::string, 8> out;
        out[0] = north(norm);
        out[1] = east(out[0]);
        out[2] = east(norm);
        out[3] = east(south(norm));
        out[4] = south(norm);
        out[5] = west(out[4]);
        out[6] = west(norm);
        out[7] = west(out[0]);
        return out;
    }

    std::vector<std::string> geohash_cover(GeoBBox bbox, int precision) {
        validate_bbox(bbox);
        validateInRange((size_t)precision, (size_t)1, (size_t)MAX_GEOHASH_LEN);

        auto [lat_min, lat_max, lon_min, lon_max] = bbox;
        auto [lat_step_deg, lon_step_deg] = cell_size_degrees(precision);
        std::vector<std::string> hashes;
        std::set<std::string> uniq;

        double lat = lat_min;
        while (lat <= lat_max) {
            double lon = lon_min;
            while (lon <= lon_max) {
                std::string h = encode_geohash(lat, lon, precision);
                uniq.insert(h);
                lon += lon_step_deg;
            }
            lat += lat_step_deg;
        }

        hashes.assign(uniq.begin(), uniq.end());
        return hashes;
    }

    int precision_for_radius(double radius_meters) {
        validateInRange(radius_meters, 0.0, 20000000.0);
        for (int p = 1; p <= MAX_GEOHASH_LEN; p++) {
            auto [lat_step_deg, lon_step_deg] = cell_size_degrees(p);
            double cell_h = lat_step_deg * meters_per_deg_lat();
            double cell_w = lon_step_deg * meters_per_deg_lon(0.0);
            double diag = std::sqrt(cell_h * cell_h + cell_w * cell_w);
            if (diag <= radius_meters) {
                return p;
            }
        }
        return MAX_GEOHASH_LEN;
    }

    int precision_for_bbox(GeoBBox bbox) {
        validate_bbox(bbox);
        auto [lat_min, lat_max, lon_min, lon_max] = bbox;
        double lat_mid = (lat_min + lat_max) * 0.5;
        double target_h = (lat_max - lat_min) * meters_per_deg_lat();
        double target_w = (lon_max - lon_min) * meters_per_deg_lon(lat_mid);
        double target = std::max(target_h, target_w);
        return precision_for_radius(target);
    }
}
