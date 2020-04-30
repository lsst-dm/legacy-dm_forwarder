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

#include <sstream>
#include <future>
#include <ims/Image.hh>
#include <core/Exceptions.h>
#include <core/SimpleLogger.h>
#include <forwarder/Formatter.h>
#include <forwarder/ReadoutPattern.h>
#include <daq/DAQDecoder.h>
#include <daq/DAQFetcher.h>

namespace fs = boost::filesystem;

DAQFetcher::DAQFetcher(const std::string& partition,
                       const std::string& folder,
                       const std::vector<int>& data_segment,
                       redis_connection_params params,
                       const int xor_pattern) :
        _folder{folder},
        // Bug: Invalid partition name segfaults from DAQ
        _store(partition.c_str()),
        _xor{xor_pattern},
        _fmt(data_segment, params) {
}

void DAQFetcher::fetch(const fs::path& prefix,
                       const std::string& image,
                       const std::string& location) {
    IMS::Id id = _store.catalog.lookup(image.c_str(), _folder.c_str());
    if (!id) {
        std::ostringstream err;
        err << "Folder " << _folder << " or Image " << image
            << " does not exist in the catalog";
        LOG_CRT << err.str();
        throw L1::CannotFetchPixel(err.str());
    }

    IMS::Image img(id, _store);
    if (!img) {
        std::ostringstream err;
        err << "Cannot create IMS::Image for " << image << " at location "
            << location;
        LOG_CRT << err.str();
        throw L1::CannotFetchPixel(err.str());
    }

    DAQ::Location mine(location.c_str());
    DAQ::LocationSet filter(mine);

    DAQ::Sensor::Type sensor_type;
    try {
        sensor_type = ReadoutPattern::sensor(location);
    }
    catch (L1::InvalidLocation& e) {
        std::ostringstream err;
        err << "Location: " << location << ". " << e.what();
        LOG_CRT << err.str();
        throw L1::CannotFetchPixel(err.str());
    }

    DAQDecoder decoder(img, filter);
    decoder.run();

    if (!decoder.valid()) {
        std::ostringstream err;
        err << "There is no data from DAQ for image " << image
            << " and location " << location;
        LOG_CRT << err.str();
        throw L1::CannotFetchPixel(err.str());
    }

    std::vector<Data> data = decoder.data();
    uint64_t samples = decoder.samples();
    Pixel3d ccds = declutter(data, samples, sensor_type);

    // naxes calculation
    std::vector<long> axes = naxes(data[0], samples);
    long* naxes = axes.data();

    // filename calculation
    std::string new_location(location);
    size_t found = new_location.find("/");
    if (found == std::string::npos) {
        std::ostringstream err;
        err << "Location " << location << " is not a valid string with `/`";
        LOG_CRT << err.str();
        throw L1::CannotFetchPixel(err.str());
    }
    new_location.replace(found, 1, "S");
    fs::path filename = prefix / fs::path(image + "-R" + new_location);

    try {
        _fmt.write(image, ccds, naxes, filename.string());
    }
    catch (L1::CannotFormatFitsfile& e) {
        throw L1::CannotFetchPixel(e.what());
    }
}

Pixel3d DAQFetcher::declutter(std::vector<Data>& data,
                              uint64_t samples,
                              DAQ::Sensor::Type sensor) {
    uint64_t segments = (unsigned) DAQ::Sensor::Segment::NUMOF;
    uint64_t offset = 0;

    Pixel3d pixels(sensor, segments, samples);
    int32_t*** pix = pixels.get();

    for (auto&& x : data) {
        for (int i = 0; i < sensor; i++) {
            for (int j = 0; j < x.samples(); j++) {
                for (int k = 0; k < segments; k++) {
                    pix[i][k][j+offset] = _xor ^ x.pixel(j, i, k);
                }
            }
        }
        offset += x.samples();
    }
    return pixels;
}

std::vector<long> DAQFetcher::naxes(Data& data, uint64_t samples) {
    std::vector<long> axes = data.naxes();
    uint64_t total = axes[0] * axes[1];
    if (total != samples) {
        std::ostringstream err;
        err << "Naxes calculation from DAQ registers and number of Source "
            << "samples(" << axes[0] << " x " << axes[1] << "!=" << samples
            << ") are not equal. Defaulting to (1, " << samples << ")";
        LOG_WRN << err.str();
        return std::vector<long>{1, static_cast<long>(samples)};
    }
    return axes;
}
