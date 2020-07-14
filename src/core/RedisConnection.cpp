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

#include <sys/time.h>
#include <core/SimpleLogger.h>
#include <core/Exceptions.h>
#include <core/RedisResponse.h>
#include <core/RedisConnection.h>

RedisConnection::RedisConnection(const std::string host,
                                 const int port,
                                 const int db) : _host(host) {
    // Timeout of 2 seconds for connection handshake
    const struct timeval tv{2, 0};

    _context = redisConnectWithTimeout(host.c_str(), port, tv);
    if (_context->err) {
        LOG_CRT << _context->errstr;
        throw L1::RedisError(_context->errstr);
    }

    select(std::to_string(db));
    exec();
    LOG_INF << "Made connection to redis " << host << " using db " << db;
}

RedisConnection::RedisConnection(const std::string host,
                                 const int port,
                                 const int db,
                                 const std::string passwd) : _host(host) {
    // Timeout of 2 seconds for connection handshake
    const struct timeval tv{2, 0};

    _context = redisConnectWithTimeout(host.c_str(), port, tv);
    if (_context->err) {
        LOG_CRT << _context->errstr;
        throw L1::RedisError(_context->errstr);
    }

    auth(passwd);

    select(std::to_string(db));
    exec();
    LOG_INF << "Made connection to redis " << host << " using db " << db;
}

RedisConnection::RedisConnection(struct redis_connection_params params)
        : _host(params.host) {
    // Timeout of 2 seconds for connection handshake
    const struct timeval tv{2, 0};

    _context = redisConnectWithTimeout(params.host.c_str(), params.port, tv);
    if (_context->err) {
        LOG_CRT << _context->errstr;
        throw L1::RedisError(_context->errstr);
    }

    auth(params.passwd);

    select(std::to_string(params.db));
    exec();
    LOG_INF << "Made connection to redis " << params.host
            << " using db " << params.db;
}

RedisConnection::~RedisConnection() {
    redisFree(_context);
}

void RedisConnection::auth(const std::string passwd){
    std::vector<std::string> v{ "AUTH", passwd };
    RedisArg arg;
    arg.arg = v;
    _commands.push_back(arg);
}

void RedisConnection::select(const std::string index) {
    std::vector<std::string> v{ "select", index };
    RedisArg arg;
    arg.arg = v;
    _commands.push_back(arg);
}

void RedisConnection::lpush(const std::string key,
                            std::vector<std::string> values) {
    std::vector<std::string> v{ "lpush", key };
    std::copy(values.begin(), values.end(), std::back_inserter(v));
    RedisArg arg;
    arg.arg = v;
    _commands.push_back(arg);
}

void RedisConnection::lrange(const std::string key,
                             const std::string start,
                             const std::string stop) {
    std::vector<std::string> v{ "lrange", key, start, stop };
    RedisArg arg;
    arg.arg = v;
    _commands.push_back(arg);
}

void RedisConnection::setex(const std::string key,
                            const std::string seconds,
                            const std::string value) {
    std::vector<std::string> v{ "setex", key, seconds, value };
    RedisArg arg;
    arg.arg = v;
    _commands.push_back(arg);
}

void RedisConnection::exists(const std::string key) {
    std::vector<std::string> v{ "exists", key };
    RedisArg arg;
    arg.arg = v;
    _commands.push_back(arg);
}

void RedisConnection::set(const std::string key,
                          const std::string value) {
    std::vector<std::string> v{ "set", key, value };
    RedisArg arg;
    arg.arg = v;
    _commands.push_back(arg);
}

void RedisConnection::get(const std::string key) {
    std::vector<std::string> v{ "get", key };
    RedisArg arg;
    arg.arg = v;
    _commands.push_back(arg);
}

void RedisConnection::flushdb() {
    std::vector<std::string> v{ "flushdb" };
    RedisArg arg;
    arg.arg = v;
    _commands.push_back(arg);
}

void RedisConnection::keys(const std::string pattern) {
    std::vector<std::string> v{ "keys", pattern };
    RedisArg arg;
    arg.arg = v;
    _commands.push_back(arg);
}

void RedisConnection::del(std::vector<std::string> keys) {
    std::vector<std::string> v{ "del" };
    std::copy(keys.begin(), keys.end(), std::back_inserter(v));
    RedisArg arg;
    arg.arg = v;
    _commands.push_back(arg);
}

std::vector<Reply> RedisConnection::exec() {
    std::vector<Reply> replies;
    std::vector<RedisResponse> responses;
    for (auto&& arg : _commands) {
        RedisResponse r(_context, arg);
        responses.push_back(r);
    }

    for (auto&& res : responses) {
        replies.push_back(res.get());
    }

    _commands.clear();
    return replies;
}
