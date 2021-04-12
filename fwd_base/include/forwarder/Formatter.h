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

#ifndef FORMATTER_H
#define FORMATTER_H

#include <memory>
#include <fitsio.h>
#include <boost/filesystem.hpp>
#include <daq/Pixel3d.h>
#include <core/RedisConnection.h>

class Formatter {
    public:
        Formatter(const std::vector<int>& data_segment, redis_connection_params params);
        std::string write_pix_file(int32_t**,
                                   int32_t&,
                                   long*,
                                   const boost::filesystem::path&);
        void write(const std::string image,
                   Pixel3d& ccds,
                   long* naxes,
                   const boost::filesystem::path& prefix);

    protected:
        std::unique_ptr<RedisConnection> _db;
        std::vector<int> _data_segment;
};

class FitsFormatter : public Formatter {
    public:
        FitsFormatter(const std::vector<std::string>& daq_mapping,
                      const std::vector<std::string>& hdr_mapping,
                      redis_connection_params params);
        void write_header(const boost::filesystem::path& pix_path,
                          const boost::filesystem::path& header_path);
        bool contains_excluded_key(const char*);
        int get_segment_num(fitsfile* header);
};

class FitsOpener {
    public:
        FitsOpener(const boost::filesystem::path&, int);
        ~FitsOpener();
        fitsfile* get();
        int num_hdus();

    private:
        fitsfile* _fptr;
        int _status;
};

enum FILE_MODE {
    WRITE_ONLY = -1
};

#endif
