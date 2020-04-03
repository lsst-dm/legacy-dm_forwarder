/*
 * This file is part of dm_forwarder
 *
 * Developed for the LSST Data Management System.
 * This product includes software developed by the LSST Project
 * (https://www.lsst.org).
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <core/SimpleLogger.h>
#include <core/Exceptions.h>
#include <forwarder/Formatter.h>
#include <forwarder/YAMLFormatter.h>

namespace fs = boost::filesystem;

std::vector<std::string> excluded_keywords {
    "SIMPLE",
    "BITPIX",
    "NAXIS",
    "NAXIS1",
    "NAXIS2",
    "EXTEND",

    "XTENSION",
    "PCOUNT",
    "GCOUNT"
};

YAMLFormatter::YAMLFormatter(const std::vector<std::string>& daq_mapping) {
    _daq_mapping = daq_mapping;
}

YAMLFormatter::~YAMLFormatter() {

}

template <typename T>
boost::optional<T> YAMLFormatter::get(const YAML::Node& n,
                                      const std::string keyword) {
    if (!n[keyword]) {
        std::ostringstream err;
        err << "Keyword " << keyword << " does not exist in node.";
        LOG_CRT << err.str();
        throw L1::CannotFormatFitsfile(err.str());
    }

    if (n.IsNull()) {
        return {};
    }

    try {
        T val = n[keyword].as<T>();
        return val;
    }
    catch (YAML::TypedBadConversion<T>& e) {
        return {};
    }
}

bool YAMLFormatter::contains(const std::string key) {
    auto it = find(excluded_keywords.begin(), excluded_keywords.end(), key);
    if (it != excluded_keywords.end()) {
        return true;
    }
    return false;
}

void YAMLFormatter::write_key(fitsfile* fptr, const YAML::Node& n) {
    if (!n["keyword"] || !n["value"] || !n["comment"]) {
        std::ostringstream err;
        err << "keyword, value or comment do not exist in given node";
        LOG_CRT << err.str();
        throw L1::CannotFormatFitsfile(err.str());
    }

    int status = 0;

    auto o_key = get<std::string>(n, "keyword");
    auto o_comment = get<std::string>(n, "comment");

    const char* key = o_key ? (*o_key).c_str() : nullptr;
    const char* comment = o_comment ? (*o_comment).c_str() : nullptr;

    auto val_b = get<bool>(n, "value");
    auto val_d = get<double>(n, "value");
    auto val_i32 = get<int32_t>(n, "value");
    auto val_i64 = get<int64_t>(n, "value");
    auto val_s = get<std::string>(n, "value");

    if (o_key && contains(*o_key)) {
        return;
    }

    if (n["value"].Tag() == "!") {
        fits_write_key(fptr, TSTRING, key, (void *)(*val_s).c_str(), comment,
                 &status);
        return;
    }

    if (val_b) {
        fits_write_key(fptr, TLOGICAL, key, &(*val_b), comment, &status);
    }
    else if (val_i32) {
        fits_write_key(fptr, TINT, key, &(*val_i32), comment, &status);
    }
    else if (val_i64) {
        fits_write_key(fptr, TLONGLONG, key, &(*val_i64), comment,
                 &status);
    }
    else if (val_d) {
        fits_write_key(fptr, TDOUBLE, key, &(*val_d), comment, &status);
    }
    else if (val_s) {
        fits_write_key(fptr, TSTRING, key, (void *)(*val_s).c_str(), comment,
                 &status);
    }
    else {
        if (o_key) {
            // key valid, value null
            fits_write_key_null(fptr, key, comment, &status);
        }
        else {
            // key null, value null
            std::string pretty_comment = "        " + *o_comment;
            fits_write_record(fptr, pretty_comment.c_str(), &status);
        }
    }

    if (status) {
        char errmsg[FLEN_ERRMSG];
        fits_read_errmsg(errmsg);

        std::ostringstream err;
        err << "Cannot write header keywords because " << errmsg;
        LOG_CRT << err.str();
        throw L1::CannotFormatFitsfile(err.str());
    }
}

void YAMLFormatter::write_header(const fs::path& pix_path,
                                 const fs::path& header_path) {
    int status = 0;

    FitsOpener pix_file(pix_path, READWRITE);
    fitsfile* pix = pix_file.get();

    std::string pix_str = pix_path.string();
    size_t hyphen = pix_str.find_last_of("-");
    size_t dot = pix_str.find_last_of(".");

    if (hyphen == std::string::npos || dot == std::string::npos) {
        std::ostringstream err;
        err << "Pixel filename " << pix_path.string() << " is not valid. "
            << "Format is imageName-R00S00.fits";
        LOG_CRT << err.str();
        throw L1::CannotFormatFitsfile(err.str());
    }

    std::string sensor = pix_str.substr(hyphen+1, dot-hyphen-1);

    YAML::Node header_node, primary, primary_common;
    try {
        header_node  = YAML::LoadFile(header_path.string());
        primary = header_node["PRIMARY"];
        primary_common = header_node[sensor + "_PRIMARY"];
    }
    catch (YAML::BadFile& e) {
        std::ostringstream err;
        err << "Header file at " << header_path.string() << " is not valid "
            << "because " << e.what();
        LOG_CRT << err.str();
        throw L1::CannotFormatFitsfile(err.str());
    }
    catch (YAML::BadSubscript& e) {
        std::ostringstream err;
        err << "Header file at " << header_path.string() << " is not valid "
            << "because " << e.what();
        LOG_CRT << err.str();
        throw L1::CannotFormatFitsfile(err.str());
    }

    if (!primary || !primary_common) {
        std::ostringstream err;
        err << "Keywords PRIMARY or " << sensor << "_PRIMARY do not exist.";
        LOG_CRT << err.str();
        throw L1::CannotFormatFitsfile(err.str());
    }

    // write primary hdu
    fits_movabs_hdu(pix, 1, nullptr, &status);
    for (auto&& x : primary) {
        write_key(pix, x);
    }

    // combine with primary common
    for (auto&& x : primary_common) {
        write_key(pix, x);
    }

    for (int i = 0; i  < _daq_mapping.size(); i++) {
        fits_movabs_hdu(pix, i+2, IMAGE_HDU, &status);
        std::string segment_hdr = sensor + "_Segment" + _daq_mapping[i];
        YAML::Node segment = header_node[segment_hdr];
        if (!segment) {
            std::ostringstream err;
            err << "Keyword " << segment_hdr << " does not exist.";
            LOG_CRT << err.str();
            throw L1::CannotFormatFitsfile(err.str());
        }

        for (auto&& x : segment) {
            write_key(pix, x);
        }
    }
}
