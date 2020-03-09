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

#ifndef REDISRESPONSE_H
#define REDISRESPONSE_H

#include <string>
#include <vector>
#include <hiredis/hiredis.h>

struct Reply {
    long long integer;
    std::string str;
    std::vector<Reply> elements;
};

struct RedisArg {
    std::vector<std::string> arg;
};

class RedisResponse {
    public:
        RedisResponse(redisContext* context,
                      RedisArg arg);
        ~RedisResponse();
        Reply get();
        Reply reply(redisReply* r);

    private:
        redisContext* _context;
        RedisArg _arg;
};

#endif
