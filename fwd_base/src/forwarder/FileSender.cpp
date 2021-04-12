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
#include "core/SimpleLogger.h"
#include "core/Exceptions.h"
#include "forwarder/FileSender.h"

namespace fs = boost::filesystem;

FileSender::FileSender(const std::string& xfer_option) {
    _xfer_option = xfer_option;
}

void FileSender::send(std::vector<std::string>& from, const fs::path& to) {
    std::ostringstream files;
 
    for (int i = 0; i < from.size(); i++) {
        if (i == from.size()-1) {
            files << from[i];
        }
        else {
            files << from[i] << " ";
        }
    }


    std::ostringstream bbcp;
    bbcp << _xfer_option
         << " "
         << files.str()
         << " "
         << to.string();

    int status = system(bbcp.str().c_str());
    if (status) {
        std::ostringstream err;
        err << "Cannot copy file from " << files.str()
            << "to" << to.string();
        LOG_CRT << err.str();
        throw L1::CannotCopyFile(err.str());
    }

    LOG_INF << "Sent file from " << files.str() << " to " << to.string();
}
