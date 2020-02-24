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
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <yaml-cpp/yaml.h>
#include <core/IIPBase.h>
#include <core/Exceptions.h>
#include <daq/Location.hh>
#include <ims/Stripe.hh>
#include <ims/SourceMetadata.hh>
#include <rms/InstructionList.hh>
#include <rms/Instruction.hh>
#include <daq/DAQFetcher.h>
#include <daq/Data.h>

namespace fs = boost::filesystem;
namespace ut = boost::unit_test;

struct DAQFetcherFixture : IIPBase {

    std::unique_ptr<DAQFetcher> _daq;
    std::string _img, _sensor, _loc;

    // data creation
    static const int _sensors = 3;
    int _samples = 100;
    IMS::Stripe* _arr[_sensors];
    IMS::SourceMetadata _meta;

    DAQFetcherFixture() : IIPBase("ForwarderCfg.yaml", "test") {
        const std::string partition = _config_root["PARTITION"]
                .as<std::string>();
        const std::string folder = _config_root["FOLDER"].as<std::string>();
        const std::vector<std::string> daq_mapping = _config_root["DAQ_MAPPING"]
                .as<std::vector<std::string>>();
        const std::vector<std::string> hdr_mapping = _config_root["HDR_MAPPING"]
                .as<std::vector<std::string>>();
        _daq = std::unique_ptr<DAQFetcher>(new DAQFetcher(partition, folder,
                daq_mapping, hdr_mapping));


        BOOST_TEST_REQUIRE(ut::framework::master_test_suite().argc == 3);
        BOOST_TEST(ut::framework::master_test_suite().argv[1] == "--data");
        std::string path(ut::framework::master_test_suite().argv[2]);

        YAML::Node root = YAML::LoadFile(path);
        _img = root["image"].as<std::string>();
        _sensor = root["sensor"].as<std::string>();
        _loc = root["location"].as<std::string>();

        // data creation
        IMS::Stripe s;
        int32_t stripe_d[16] {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        };
        std::copy(std::begin(stripe_d), std::end(stripe_d),
            std::begin(s.segment));

        for (int i = 0; i < _sensors; i++) {
            _arr[i] = new IMS::Stripe[_samples];
            for (int j = 0; j < _samples; j++) {
                _arr[i][j] = s;
            }
        }

        RMS::InstructionList list = _meta.instructions();
        list.insert(RMS::Instruction::Opcode::PUT, 0, 2);
        list.insert(RMS::Instruction::Opcode::PUT, 1, 2);
        list.insert(RMS::Instruction::Opcode::PUT, 2, 3);
        list.insert(RMS::Instruction::Opcode::PUT, 3, 4);
        list.insert(RMS::Instruction::Opcode::PUT, 4, 2);
        list.insert(RMS::Instruction::Opcode::PUT, 5, 3);
        list.insert(RMS::Instruction::Opcode::PUT, 6, 7);
        list.insert(RMS::Instruction::Opcode::PUT, 7, 5);
        list.insert(RMS::Instruction::Opcode::PUT, 8, 9);
        list.insert(RMS::Instruction::Opcode::PUT, 9, 5);
        _meta = list;
    }

    ~DAQFetcherFixture() {

    }
};

BOOST_FIXTURE_TEST_SUITE(DAQFetcherTest, DAQFetcherFixture);

BOOST_AUTO_TEST_CASE(constructor) {
    // Cannot test invalid partition because daq segfaults
}

BOOST_AUTO_TEST_CASE(fetch) {

    // invalid filepath
    BOOST_CHECK_THROW(_daq->fetch("/home/", _img, _loc),
            L1::CannotFetchPixel);

    // invalid image name
    BOOST_CHECK_THROW(_daq->fetch("./", "helloworld", _loc),
            L1::CannotFetchPixel);

    // invalid location
    // BUG: DAQ uses first two characters of location string. So, 100/0, 110/0
    // is a valid location string, which shouldn't be.
    BOOST_CHECK_THROW(_daq->fetch("./", _img, "90/0"),
            L1::CannotFetchPixel);

    // filename
    std::string loc(_loc);
    size_t found = loc.find("/");
    loc.replace(found, 1, "S");
    std::string fname = _img + "-R" + loc;

    // valid function call should produce fitsfiles
    _daq->fetch("./", _img, _loc);
    if (_sensor == "Science") {
        BOOST_CHECK_EQUAL(true, fs::exists(fname + "0.fits"));
        BOOST_CHECK_EQUAL(true, fs::exists(fname + "1.fits"));
        BOOST_CHECK_EQUAL(true, fs::exists(fname + "2.fits"));
        fs::remove(fname + "0.fits");
        fs::remove(fname + "1.fits");
        fs::remove(fname + "2.fits");
    }
    else if (_sensor == "Guiding") {
        BOOST_CHECK_EQUAL(true, fs::exists(fname + "0.fits"));
        BOOST_CHECK_EQUAL(true, fs::exists(fname + "1.fits"));
        fs::remove(fname + "0.fits");
        fs::remove(fname + "1.fits");
    }
    else if (_sensor == "Wavefront") {
        BOOST_CHECK_EQUAL(true, fs::exists(fname + "0.fits"));
        fs::remove(fname + "0.fits");
    }
    else {
        BOOST_FAIL("Invalid sensor name");
    }
}

BOOST_AUTO_TEST_CASE(sensor) {

    // Location 55/0 does not exist in the daq
    DAQ::Location loc("55/0");
    BOOST_CHECK_THROW(_daq->sensor(loc), L1::InvalidLocation);

    // Valid location
    DAQ::Location loc2(_loc.c_str());
    BOOST_CHECK_NO_THROW(_daq->sensor(loc2));

    // Valid sensor
    DAQ::Sensor::Type sensor_t = _daq->sensor(loc2);
    if (_sensor == "Science")
        BOOST_CHECK_EQUAL(true, sensor_t == DAQ::Sensor::Type::SCIENCE);
    else if (_sensor == "Guiding")
        BOOST_CHECK_EQUAL(true, sensor_t == DAQ::Sensor::Type::GUIDE);
    else if (_sensor == "Wavefront")
        BOOST_CHECK_EQUAL(true, sensor_t == DAQ::Sensor::Type::WAVEFRONT);
}

BOOST_AUTO_TEST_CASE(declutter) {

    // Valid
    Data d(_sensors, _samples, _arr, _meta);
    std::vector<Data> vec {d, d, d};
    BOOST_CHECK_NO_THROW(_daq->declutter(vec, 300,
        DAQ::Sensor::Type::SCIENCE));
}

BOOST_AUTO_TEST_CASE(naxes) {

    Data d(_sensors, _samples, _arr, _meta);

    // valid naxes, given 100 samples, axes 10x10
    std::vector<long> o{10, 10};
    std::vector<long> a = _daq->naxes(d, _samples);
    BOOST_CHECK_EQUAL_COLLECTIONS(o.begin(), o.end(), a.begin(), a.end());

    // valid naxes, given 90 samples, which is not equal to what is being
    // defined in meta data, so should give 1x90.
    std::vector<long> o2{1, 90};
    std::vector<long> a2 = _daq->naxes(d, 90);
    BOOST_CHECK_EQUAL_COLLECTIONS(o2.begin(), o2.end(), a2.begin(), a2.end());
}

BOOST_AUTO_TEST_SUITE_END()
