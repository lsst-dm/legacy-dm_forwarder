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
#include <algorithm>
#include <core/Exceptions.h>
#include <core/SimpleLogger.h>
#include <core/RedisConnection.h>
#include <forwarder/Scoreboard.h>

const std::string TARGET = ":target";
const std::string CCD = ":ccd";
const std::string HEADER = ":header";
const std::string SESSION_ID = ":session_id";
const std::string JOB_NUM = ":job_num";
const std::string LOCATIONS = ":locations";

Scoreboard::Scoreboard(const std::string& host,
                       const int& port,
                       const int& db_num,
                       const std::string& password) {
    _con = std::unique_ptr<RedisConnection>(
            new RedisConnection(host, port, db_num));

    _con->flushdb();
    _con->exec();
}

Scoreboard::~Scoreboard() {

}

bool Scoreboard::ready(const std::string& image_id) {
    _con->exists(image_id + CCD);
    _con->exists(image_id + HEADER);
    std::vector<Reply> replies = _con->exec();

    bool flag = true;
    for (auto&& r : replies) {
        flag = flag && r.integer;
    }
    return flag;
}

void Scoreboard::remove(const std::string& image_id) {
    _con->keys(image_id + "*");
    std::vector<Reply> results = _con->exec();

    std::vector<std::string> keys;
    for (auto&& val : results[0].elements) {
        keys.push_back(val.str);
    }
    _con->del(keys);
    _con->exec();
}

void Scoreboard::add_xfer(const std::string& image_id, const xfer_info& xfer) {
    _con->set(image_id + TARGET, xfer.target);
    _con->set(image_id + SESSION_ID, xfer.session_id);
    _con->set(image_id + JOB_NUM, xfer.job_num);
    _con->lpush(image_id + LOCATIONS, xfer.locations);
    _con->exec();
}

xfer_info Scoreboard::get_xfer(const std::string& image_id) {
    _con->get(image_id + SESSION_ID);
    _con->get(image_id + JOB_NUM);
    _con->get(image_id + TARGET);
    std::vector<Reply> r = _con->exec();

    xfer_info info;
    info.session_id = r[0].str;
    info.job_num = r[1].str;
    info.target = r[2].str;
    info.locations = locations(image_id);
    return info;
}

void Scoreboard::add_header(const std::string& image_id,
                            const std::string& path) {
    _con->set(image_id + HEADER, path);
    _con->exec();
}

void Scoreboard::set_fwd(const std::string& key, const std::string& body) {
    _con->lpush(key, { body });
    _con->exec();
}

std::vector<std::string> Scoreboard::locations(const std::string& image_id) {
    _con->lrange(image_id + LOCATIONS, "0", "-1");
    std::vector<Reply> reply = _con->exec();

    std::vector<std::string> locations;
    for (auto&& loc : reply[0].elements) {
        locations.push_back(loc.str);
    }
    return locations;
}

std::string Scoreboard::header(const std::string& image_id) {
    _con->get(image_id + HEADER);
    std::vector<Reply> r = _con->exec();
    return r[0].str;
}

std::vector<std::string> Scoreboard::ccds(const std::string& image_id) {
    _con->lrange(image_id + CCD, "0", "-1");
    std::vector<Reply> r = _con->exec();

    std::vector<std::string> sensors;
    for (auto&& ccd : r[0].elements) {
        sensors.push_back(ccd.str);
    }
    return sensors;
}
