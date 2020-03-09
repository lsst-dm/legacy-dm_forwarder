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
#include <thread>
#include <chrono>
#include "core/HeartBeat.h"
#include "core/RedisConnection.h"
#include "core/Exceptions.h"
#include "core/SimpleLogger.h"

void Watcher::start(const heartbeat_params params) {
    clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    _stop = false;
    std::thread t(&Watcher::check, this, params);
    t.detach();
}

void Watcher::clear() {
    std::lock_guard<std::mutex> lk(_mutex);
    _stop = true;
    _cond.notify_all();
}

void Watcher::check(const heartbeat_params params) {
    const std::string host = params.redis_host;
    const int port = params.redis_port;
    const int db = params.redis_db;

    const std::string key = params.key;
    const int timeout = params.seconds_to_expire;
    auto execute = params.action;

    try {
        RedisConnection redis(host, port, db);
        LOG_INF << "Watcher started";
        while (!_stop.load()) {
            redis.exists(key);
            std::vector<Reply> reply = redis.exec();
            bool exists = reply[0].integer;

            if (!exists) {
                LOG_CRT << "Did not receive heartbeat from Commandable SAL"
                    << " Component(CSC)";
                execute();
                break;
            }

            std::unique_lock<std::mutex> lk(_mutex);
            std::cv_status status = _cond.wait_for(lk,
                    std::chrono::seconds(timeout));
        }
    }
    catch(L1::RedisError& e) {
        LOG_CRT << "Cannot start Watcher because " << e.what();
        return;
    }
    catch(std::exception& e) {
        LOG_CRT << "Cannot start Watcher because " << e.what();
        return;
    }
    LOG_CRT << "Watcher ended";
}
