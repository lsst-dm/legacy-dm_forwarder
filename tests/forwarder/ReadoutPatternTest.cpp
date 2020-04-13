
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
#include <core/IIPBase.h>
#include <core/Exceptions.h>
#include <forwarder/ReadoutPattern.h>

struct ReadoutPatternFixture : IIPBase {

    std::string _log_dir;

    ReadoutPatternFixture() : IIPBase("ForwarderCfg.yaml", "test") {
        BOOST_TEST_MESSAGE("Setup ReadoutPatternTest fixture");
        _log_dir = _config_root["LOGGING_DIR"].as<std::string>();
    }

    ~ReadoutPatternFixture() {
        BOOST_TEST_MESSAGE("TearDown ReadoutPatternTest fixture");
        std::string log = _log_dir + "/test.log.0";
        std::remove(log.c_str());
    }
};

BOOST_FIXTURE_TEST_SUITE(ReadoutPatternTest, ReadoutPatternFixture);

BOOST_AUTO_TEST_CASE(constructor) {
    YAML::Node n = YAML::Load("{ hello: world }");
    BOOST_CHECK_NO_THROW(ReadoutPattern r(n));
}

BOOST_AUTO_TEST_CASE(data_segment_name) {
    DAQ::Sensor::Type science = DAQ::Sensor::Type::SCIENCE;

    // invalid data_segment_name pattern
    YAML::Node bad = YAML::Load("[00, 01, 02, 03]");
    ReadoutPattern pt(bad);
    BOOST_CHECK_THROW(
            pt.data_segment_name(science), L1::InvalidReadoutPattern);

    // no keyword data segment name
    YAML::Node bad2 = YAML::Load("{ 1: a, 2: b}");
    ReadoutPattern pt2(bad2);
    BOOST_CHECK_THROW(
            pt2.data_segment_name(science), L1::InvalidReadoutPattern);

    // missing segment typo
    YAML::Node bad3 = YAML::Load("{ DATA_SEGMENT_NAME: { GUIDE: [ 00 ] }}");
    ReadoutPattern pt3(bad3);
    BOOST_CHECK_THROW(
            pt3.data_segment_name(science), L1::InvalidReadoutPattern);

    // bad data type
    YAML::Node bad4 = YAML::Load("{ DATA_SEGMENT_NAME: { \
            science: [ 00, 01, 02, 03, 04, 05, 06, 07, 10, 11, 12, 13, 14, \
            15, 17 ] }}");
    ReadoutPattern pt4(bad4);
    BOOST_CHECK_THROW(
            pt4.data_segment_name(science), L1::InvalidReadoutPattern);

    // good data
    YAML::Node good = YAML::Load("{ DATA_SEGMENT_NAME: { \
            science: [ 10, 11, 12, 13, 14, 15, 16, 17, \
                       00, 01, 02, 03, 04, 05, 06, 07 ] }}");
    std::vector<std::string> other{
        "10", "11", "12", "13", "14", "15", "16", "17",
        "00", "01", "02", "03", "04", "05", "06", "07"
    };

    ReadoutPattern pt5(good);
    std::vector<std::string> names = pt5.data_segment_name(science);
    bool eq = std::equal(other.begin(), other.end(), names.begin());
    BOOST_CHECK_EQUAL(eq, true);

    // good data
    YAML::Node good2 = YAML::Load("{ DATA_SEGMENT_NAME: { \
            science: [ 10, 11, 12, 13, 14, 15, 16, 17, \
                       00, 01, 02, 03, 04, 05, 06, 07 ] }}");
    std::vector<std::string> other2{
        "00", "01", "02", "03", "04", "05", "06", "07"
        "10", "11", "12", "13", "14", "15", "16", "17",
    };

    ReadoutPattern pt6(good2);
    std::vector<std::string> names2 = pt6.data_segment_name(science);
    bool eq2 = std::equal(other2.begin(), other2.end(), names2.begin());
    BOOST_CHECK_EQUAL(eq2, false);
}

BOOST_AUTO_TEST_CASE(data_segment) {
    DAQ::Sensor::Type science = DAQ::Sensor::Type::SCIENCE;

    // invalid data_segment_name pattern
    YAML::Node bad = YAML::Load("[00, 01, 02, 03]");
    ReadoutPattern pt(bad);
    BOOST_CHECK_THROW(
            pt.data_segment(science), L1::InvalidReadoutPattern);

    // no keyword data segment
    YAML::Node bad2 = YAML::Load("{ 1: a, 2: b}");
    ReadoutPattern pt2(bad2);
    BOOST_CHECK_THROW(
            pt2.data_segment(science), L1::InvalidReadoutPattern);

    // missing segment typo
    YAML::Node bad3 = YAML::Load("{ DATA_SEGMENT: { GUIDE: [ 0 ] }}");
    ReadoutPattern pt3(bad3);
    BOOST_CHECK_THROW(
            pt3.data_segment(science), L1::InvalidReadoutPattern);

    // bad data type
    YAML::Node bad4 = YAML::Load("{ DATA_SEGMENT: { \
            science: [ 0, 2, 3, 4, 5, 6, 7, 8, 9, 10 ] }}");
    ReadoutPattern pt4(bad4);
    BOOST_CHECK_THROW(
            pt4.data_segment(science), L1::InvalidReadoutPattern);

    // good data
    YAML::Node good = YAML::Load("{ DATA_SEGMENT: { \
            science: [ 0, 1, 2, 3, 4, 5, 6, 7, \
                       8, 9, 10, 11, 12, 13, 14, 15 ] }}");
    std::vector<int> other{
        0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15
    };

    ReadoutPattern pt5(good);
    std::vector<int> names = pt5.data_segment(science);
    bool eq = std::equal(other.begin(), other.end(), names.begin());
    BOOST_CHECK_EQUAL(eq, true);

    // good data
    YAML::Node good2 = YAML::Load("{ DATA_SEGMENT: { \
            science: [ 15, 14, 13, 12, 11, 10, 9, 8, \
                       0, 1, 2, 3, 4, 5, 6, 7 ] }}");
    std::vector<int> other2{
        0, 1, 2, 3, 4, 5, 6, 7,
        15, 14, 13, 12, 11, 10, 9, 8,
    };

    ReadoutPattern pt6(good2);
    std::vector<int> names2 = pt6.data_segment(science);
    bool eq2 = std::equal(other2.begin(), other2.end(), names2.begin());
    BOOST_CHECK_EQUAL(eq2, false);
}

BOOST_AUTO_TEST_CASE(sensor_order) {
    // we do not use this function currently.
}

BOOST_AUTO_TEST_CASE(get_xor) {
    DAQ::Sensor::Type science = DAQ::Sensor::Type::SCIENCE;

    // missing keyword
    YAML::Node n = YAML::Load("{ XOR: 0x22222 }");
    ReadoutPattern ptr(n);
    BOOST_CHECK_THROW(ptr.get_xor(science), L1::InvalidReadoutPattern);

    // wrong dtype does not work because .as<int>() does internal conversion
    // to integer even when string is given
    YAML::Node n2 = YAML::Load("{ XOR: { science: \"0x22222\" }}");

    // good
    YAML::Node n3 = YAML::Load("{ XOR: { science: \"0x22222\" }}");
    ReadoutPattern ptr3(n3);
    BOOST_CHECK_EQUAL(ptr3.get_xor(science), 0x22222);
}

BOOST_AUTO_TEST_SUITE_END()
