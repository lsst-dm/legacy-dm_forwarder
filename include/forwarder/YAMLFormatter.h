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

#ifndef YAMLFORMATTER_H
#define YAMLFORMATTER_H

#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <fitsio.h>
#include <yaml-cpp/yaml.h>

class YAMLFormatter {
    public:
        YAMLFormatter(const std::vector<std::string>& daq_mapping);
        ~YAMLFormatter();

        template <typename T>
        boost::optional<T> get(const YAML::Node& n, const std::string keyword);

        bool contains(const std::string key);
        void write_key(fitsfile* fptr, const YAML::Node& n);
        void write_header(const boost::filesystem::path& pix_path,
                          const boost::filesystem::path& header_path);

    private:
        std::vector<std::string> _daq_mapping;
};

#endif
