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

#ifndef DAQFETCHER_H
#define DAQFETCHER_H

#include <boost/filesystem.hpp>
#include <daq/Sensor.hh>
#include <daq/Location.hh>
#include <ims/Store.hh>
#include "daq/Data.h"
#include "daq/Pixel3d.h"
#include "forwarder/Formatter.h"

class DAQFetcher {
    public:
        DAQFetcher(const std::string& partition,
                   const std::string& folder,
                   std::vector<std::string> daq_mapping,
                   std::vector<std::string> header_mapping);
        void fetch(const boost::filesystem::path& prefix,
                   const std::string& image,
                   const std::string& location);
        DAQ::Sensor::Type sensor(const DAQ::Location& location);
        Pixel3d declutter(std::vector<Data>& data,
                          uint64_t samples,
                          DAQ::Sensor::Type sensor);
        std::vector<long> naxes(Data& data, uint64_t samples);

    private:
        IMS::Store _store;
        std::string _folder;
        Formatter _fmt;
        boost::filesystem::path _prefix;
};

#endif
