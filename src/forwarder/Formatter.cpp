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
#include <core/SimpleLogger.h>
#include <core/Exceptions.h>
#include <core/RedisConnection.h>
#include <forwarder/Formatter.h>

namespace fs = boost::filesystem;

Formatter::Formatter(const std::vector<int>& data_segment) :
        _data_segment{data_segment} {
    _db = std::unique_ptr<RedisConnection>(
            new RedisConnection("localhost", 6379, 0));
}

std::string Formatter::write_pix_file(int32_t** ccd,
                                      int32_t& len,
                                      long* naxes,
                                      const fs::path& filepath) {
    try {
        int status = 0;
        int bitpix = LONG_IMG;
        int num_axes = 2;
        int first_elem = 1;

        FitsOpener file(filepath, FILE_MODE::WRITE_ONLY);
        fitsfile* optr = file.get();

        fits_create_img(optr, bitpix, 0, NULL, &status);
        for (int i = 0; i < _data_segment.size(); i++) {
            int idx = _data_segment[i];
            fits_create_img(optr, bitpix, num_axes, naxes, &status);
            fits_write_img(optr, TINT, first_elem, len, ccd[idx], &status);
        }

        if (status) {
            char err[FLEN_ERRMSG];
            fits_read_errmsg(err);
            LOG_CRT << std::string(err);
            throw L1::CannotFormatFitsfile(err);
        }
        LOG_INF << "Finished writing pixel fitsfile at " << filepath.string();

        return filepath.string();
    }
    catch (L1::CfitsioError& e) {
        throw L1::CannotFormatFitsfile(e.what());
    }
}

void Formatter::write(const std::string image,
                      Pixel3d& ccds,
                      long* naxes,
                      const fs::path& prefix) {
    int32_t d3 = static_cast<int32_t>(ccds.d3());
    int32_t*** pix = ccds.get();
    std::vector<std::future<std::string>> tasks;

    for (int i = 0; i < ccds.d1(); i++) {
        int32_t** ccd = pix[i];

        std::ostringstream osname;
        osname << prefix.string() << i << ".fits";

        fs::path filename(osname.str());

        std::future<std::string> job = std::async(
                std::launch::async,
                &Formatter::write_pix_file,
                this,
                ccd,
                std::ref(d3),
                naxes,
                filename);
        tasks.push_back(std::move(job));
    }

    for (auto&& task : tasks) {
        std::string filename = task.get();
        std::string key = image + ":ccd";
        _db->lpush(key, { filename });
        _db->exec();
    }
}
