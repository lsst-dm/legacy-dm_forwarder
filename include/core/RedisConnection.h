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

#ifndef REDIS_CONNECTION_H
#define REDIS_CONNECTION_H

#include <vector>
#include <hiredis/hiredis.h>
#include <core/SimpleLogger.h>
#include <core/Exceptions.h>
#include <core/RedisResponse.h>

/**
 * Wrapper class for creating a redis connection
 *
 * RedisConnection is a higher level abstraction for hiredis API calls. It
 * implements one method called `command`, which should be used for sending
 * command to redis database.
 *
 * Note: redisContext is not thread-safe.
 */

struct redis_connection_params {
    std::string host;
    int port;
    int db;
    std::string passwd;
};

class RedisConnection {
    public:
        RedisConnection(const std::string host,
                        const int port,
                        const int db);
        RedisConnection(const std::string host,
                        const int port,
                        const int db,
                        const std::string passwd);
        RedisConnection(redis_connection_params params);
        ~RedisConnection();
        void auth(const std::string passwd);
        void select(const std::string index);
        void lpush(const std::string key,
                   std::vector<std::string> values);
        void setex(const std::string key,
                   const std::string seconds,
                   const std::string value);
        void exists(const std::string key);
        void set(const std::string key,
                 const std::string value);
        void get(const std::string key);
        void lrange(const std::string key,
                    const std::string start,
                    const std::string stop);
        void flushdb();
        void keys(const std::string pattern);
        void del(std::vector<std::string> keys);
        std::vector<Reply> exec();

    private:
        std::string _host;
        redisContext* _context;
        std::vector<RedisArg> _commands;
};

#endif
