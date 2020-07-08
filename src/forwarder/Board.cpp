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

#include <forwarder/Board.h>

L1::Board L1::Board::decode_filename(const std::string filename) {
    size_t hyphen = filename.find_last_of("-");
    size_t dot = filename.find_last_of(".");
    size_t slash = filename.find_last_of("/");

    if (slash == std::string::npos) {
        slash = 0;
    }

    std::string sensor_name = filename.substr(hyphen+1, dot-hyphen-1);
    std::string bay = sensor_name.substr(1, 2);
    std::string board = sensor_name.substr(4, 2);

    std::string bay_board = bay + "/" + board[0];
    std::string obsid = filename.substr(slash+1, hyphen-slash-1);

    Board b;
    b.bay_board = bay_board;
    b.obsid = obsid;
    b.raft = bay;
    b.ccd = board;

    return b;
}

L1::Board L1::Board::decode_location(const std::string location) {
    size_t slash = location.find_last_of("/");
    std::string bay = location.substr(0, slash);
    std::string board = location.substr(slash+1);

    Board b;
    b.bay_board = location;
    b.raft = bay;
    b.ccd = board;

    return b;
}
