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

#include <time.h>
#include <ims/Folder.hh>
#include <core/SimpleLogger.h>
#include <daq/Scanner.h>

Scanner::Scanner(const std::string partition, const int day) :
        IMS::Processor(),
        _store(partition.c_str()) {

    // compute images older than given day to catch up
    time_t current;
    time(&current);

    struct tm* local;
    local = localtime(&current);
    local->tm_mday = local->tm_mday - day;
    time_t last_time = mktime(local);

    _timestamp = OSA::TimeStamp(last_time);
}

void Scanner::process(const IMS::Id& id) {
    IMS::Image image(id, _store);
    if (!image) {
        LOG_CRT << "Cannot instantiate IMS::Image for processing";
        return;
    }

    OSA::TimeStamp time = image.metadata().timestamp();
    std::string img_name = image.metadata().name();

    if (_timestamp > time) {
        _images.push_back(img_name);
    }
}

std::vector<std::string> Scanner::get_images() {
    return _images;
}
