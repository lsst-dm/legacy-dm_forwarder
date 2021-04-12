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

#include <functional>
#include <memory>
#include <core/Exceptions.h>
#include <core/SimpleLogger.h>
#include <core/RedisResponse.h>

RedisResponse::RedisResponse(redisContext* context,
                                         RedisArg arg){
    _arg = arg;
    _context = context;

    int len = arg.arg.size();
    const char* argv[len];
    size_t argvlen[len];

    for (int i = 0; i < len; i++) {
        argv[i] = arg.arg[i].c_str();
        argvlen[i] = arg.arg[i].size();
    }

    redisAppendCommandArgv(_context, len, argv, argvlen);
}

RedisResponse::~RedisResponse() {
}

Reply RedisResponse::get() {
    std::unique_ptr<redisReply, std::function<void (redisReply*)>> r(
            new redisReply(), [](redisReply* ptr) {
                freeReplyObject(ptr);
            });
    redisReply* replyptr = r.get();
    redisGetReply(_context, (void **)&replyptr);

    if (_context == NULL || _context->err) {
        std::ostringstream err;
        err << "Error occurred while executing redis command because "
            << _context->err;
        LOG_CRT << err.str();
        throw L1::RedisError(err.str());
    }

    Reply g = reply(replyptr);
    return g;
}

Reply RedisResponse::reply(redisReply* r) {
    Reply g;
    g.integer = r->integer;

    if (r->type == REDIS_REPLY_ERROR) {
        std::ostringstream cmd;
        for (auto&& item : _arg.arg) {
            cmd << item << " ";
        }

        std::ostringstream err;
        err << "Error while executing redis `" << cmd.str() << "` because "
            << r->str;
        LOG_CRT << err.str();
        throw L1::RedisError(err.str());
    }

    if (r->type == REDIS_REPLY_STRING || r->type == REDIS_REPLY_STATUS) {
        g.str = std::string(r->str);
    }
    else {
        g.str = "";
    }

    if (r->type == REDIS_REPLY_ARRAY) {
        std::vector<Reply> replies;
        for (int i = 0; i < r->elements; i++) {
            replies.push_back(reply(r->element[i]));
        }
        g.elements = replies;
    }

    return g;
}
