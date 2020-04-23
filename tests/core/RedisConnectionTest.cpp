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

#include <boost/test/unit_test.hpp>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <core/IIPBase.h>
#include <core/Exceptions.h>
#include <core/RedisConnection.h>
#include <cstdlib>

struct RedisConnectionFixture : IIPBase {

    std::string _log_dir;
    std::string _host;
    std::string _passwd;
    int _port, _db;
    std::unique_ptr<RedisConnection> _redis;

    RedisConnectionFixture() : IIPBase("ForwarderCfg.yaml", "test"){
        BOOST_TEST_MESSAGE("Setup RedisConnectionTest fixture");
        _log_dir = _config_root["LOGGING_DIR"].as<std::string>();

        _host = _config_root["REDIS_HOST"].as<std::string>();
        _port = _config_root["REDIS_PORT"].as<int>();
        _db = _config_root["REDIS_DB"].as<int>();
        _passwd = _credentials->get_redis_passwd();

        _redis = std::unique_ptr<RedisConnection>(new RedisConnection(
                    _host, _port, _db));
        _redis->flushdb();
    }

    ~RedisConnectionFixture() {
        BOOST_TEST_MESSAGE("TearDown RedisConnectionTest fixture");
        std::string log = _log_dir + "/test.log.0";
        std::remove(log.c_str());
    }
};

BOOST_FIXTURE_TEST_SUITE(RedisConnectionTest, RedisConnectionFixture);

BOOST_AUTO_TEST_CASE(constructor) {

    // good
    BOOST_CHECK_NO_THROW(RedisConnection r(_host, _port, _db));

    // bad host
    BOOST_CHECK_THROW(RedisConnection r("host1", _port, _db), L1::RedisError);

    // bad port
    BOOST_CHECK_THROW(RedisConnection r(_host, 637, _db), L1::RedisError);

    // bad database
    BOOST_CHECK_THROW(RedisConnection r(_host, _port, 25), L1::RedisError);
}

BOOST_AUTO_TEST_CASE(constructor_with_pass) {
        std::string cmd = "redis-cli config set requirepass " + _passwd;
        system(cmd.c_str());
        // good
        BOOST_CHECK_NO_THROW(RedisConnection r(_host, _port, _db, _passwd));

        // bad host
        BOOST_CHECK_THROW(RedisConnection r("host1", _port, _db, _passwd), L1::RedisError);

        // bad port
        BOOST_CHECK_THROW(RedisConnection r(_host, 637, _db, _passwd), L1::RedisError);

        // bad database
        BOOST_CHECK_THROW(RedisConnection r(_host, _port, 25, _passwd), L1::RedisError);

        // bad password
        BOOST_CHECK_THROW(RedisConnection r(_host, _port, _db, "badpass"), L1::RedisError);

        cmd = "redis-cli -a " + _passwd + " config set requirepass \"\" ";
        system(cmd.c_str());
}

BOOST_AUTO_TEST_CASE(select) {

    // 0-16 valid
    _redis->select("0");
    BOOST_CHECK_NO_THROW(_redis->exec());

    // -1 invalid
    _redis->select("-1");
    BOOST_CHECK_THROW(_redis->exec(), L1::RedisError);

    // 20 invalid
    _redis->select("20");
    BOOST_CHECK_THROW(_redis->exec(), L1::RedisError);
}

BOOST_AUTO_TEST_CASE(lpush) {

    // lpush key val1
    _redis->lpush("key", { "val1" });
    _redis->exec();

    _redis->lrange("key", "0", "-1");
    BOOST_CHECK_EQUAL(_redis->exec()[0].elements[0].str, "val1");
}


BOOST_AUTO_TEST_CASE(lrange) {
    _redis->lrange("key", "0", "-1");
    BOOST_CHECK_EQUAL(_redis->exec()[0].elements.size(), 0);

    _redis->lpush("key", { "val1", "val2" });
    _redis->lrange("key", "0", "-1");
    std::vector<Reply> replies = _redis->exec();

    BOOST_CHECK_EQUAL(replies[1].elements.size(), 2);
    BOOST_CHECK_EQUAL(replies[1].elements[1].str, "val1");
}

BOOST_AUTO_TEST_CASE(setex) {
    std::string timeout = "3";
    _redis->setex("key", timeout, "val");
    _redis->exec();

    _redis->get("key");
    std::vector<Reply> r0 = _redis->exec();
    BOOST_CHECK_EQUAL(r0[0].str, "val");

    // after 3 seconds, key got deleted so returning empty string
    std::this_thread::sleep_for(std::chrono::seconds(3));
    _redis->get("key");
    std::vector<Reply> replies1 = _redis->exec();
    BOOST_CHECK_EQUAL(replies1[0].str, "");
}

BOOST_AUTO_TEST_CASE(exists) {
    _redis->exists("key");
    BOOST_CHECK_EQUAL(_redis->exec()[0].integer, 0);

    _redis->set("key", "val");
    _redis->exec();
    _redis->exists("key");
    BOOST_CHECK_EQUAL(_redis->exec()[0].integer, 1);
}

BOOST_AUTO_TEST_CASE(set) {
    _redis->set("foo", "bar\x20\x20hello");
    _redis->exec();

    _redis->get("foo");
    BOOST_CHECK_EQUAL(_redis->exec()[0].str, "bar  hello");
}

BOOST_AUTO_TEST_CASE(get) {
    _redis->set("foo", "bar\x20\x20hello");
    _redis->exec();

    _redis->get("foo");
    BOOST_CHECK_EQUAL(_redis->exec()[0].str, "bar  hello");
}

BOOST_AUTO_TEST_CASE(flushdb) {

    _redis->flushdb();
    _redis->exec();

    _redis->keys("*");
    BOOST_CHECK_EQUAL(_redis->exec()[0].elements.size(), 0);
}

BOOST_AUTO_TEST_CASE(keys) {
    _redis->set("foo", "bar");
    _redis->set("foo2", "bar");
    _redis->keys("*");
    BOOST_CHECK_EQUAL(_redis->exec()[3].elements.size(), 2);
}

BOOST_AUTO_TEST_CASE(del) {
    _redis->set("foo", "bar");
    _redis->set("foo2", "bar");
    _redis->exec();

    _redis->del({"foo"});
    _redis->keys("*");
    BOOST_CHECK_EQUAL(_redis->exec()[1].elements.size(), 1);

    _redis->del({"foo2"});
    _redis->keys("*");
    BOOST_CHECK_EQUAL(_redis->exec()[1].elements.size(), 0);
}

BOOST_AUTO_TEST_CASE(exec) {
    _redis->set("foo", "bar");
    BOOST_CHECK_NO_THROW(_redis->exec());
}

BOOST_AUTO_TEST_SUITE_END()
