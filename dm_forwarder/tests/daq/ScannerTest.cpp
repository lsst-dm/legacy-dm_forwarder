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

#include <memory>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <core/IIPBase.h>
#include <core/Exceptions.h>
#include <ims/Folder.hh>
#include <daq/Scanner.h>

namespace fs = boost::filesystem;

struct ScannerFixture : IIPBase {
    std::string _partition, _folder;
    fs::path _log_dir;

    ScannerFixture() : IIPBase("ForwarderCfg.yaml", "test") {
        BOOST_TEST_MESSAGE("Setup ScannerTest fixture");
        _log_dir = fs::path(_config_root["LOGGING_DIR"].as<std::string>());
        _partition = _config_root["PARTITION"].as<std::string>();
        _folder = _config_root["FOLDER"].as<std::string>();
    }

    ~ScannerFixture() {
        BOOST_TEST_MESSAGE("TearDown ScannerTest fixture");
        fs::path log = _log_dir / "test.log.0";
        fs::remove(log);
    }
};

BOOST_FIXTURE_TEST_SUITE(ScannerTest, ScannerFixture);

BOOST_AUTO_TEST_CASE(constructor) {
    BOOST_CHECK_NO_THROW(Scanner(_partition, 60));
    BOOST_CHECK_NO_THROW(Scanner(_partition, 60*48));
    BOOST_CHECK_NO_THROW(Scanner(_partition, 60*100));
    BOOST_CHECK_THROW(Scanner(_partition, -60), L1::ScannerError);

    // Cannot check invalid partition because DAQ api doesn't throw
    // BOOST_CHECK_NO_THROW(Scanner("hello", -60));
}

BOOST_AUTO_TEST_CASE(process) {
    // called by parent class
}

BOOST_AUTO_TEST_CASE(get_images) {
    IMS::Store store(_partition.c_str());
    IMS::Folder folder(_folder.c_str(), store.catalog);

    Scanner scanner(_partition, 0);
    folder.traverse(scanner);

    // if there are images with corrupted metadata, this can fail.
    BOOST_CHECK_EQUAL(scanner.get_images().size(), 0);

    // timestamp is 2 years
    Scanner scanner2(_partition, 60*24*730);
    folder.traverse(scanner2);
    BOOST_CHECK_EQUAL(scanner2.get_images().size(), folder.length());
}

BOOST_AUTO_TEST_SUITE_END()
