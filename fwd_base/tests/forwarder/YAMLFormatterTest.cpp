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
#include <boost/optional/optional_io.hpp>
#include <boost/filesystem.hpp>
#include <yaml-cpp/yaml.h>
#include <fitsio.h>
#include <core/IIPBase.h>
#include <core/Exceptions.h>
#include <forwarder/Formatter.h>
#include <forwarder/YAMLFormatter.h>

namespace ut = boost::unit_test;
namespace fs = boost::filesystem;

struct YAMLFormatterFixture : IIPBase {

    std::unique_ptr<YAMLFormatter> _yml;
    std::string _log_dir;
    YAML::Node _root;
    fs::path _hdrpath;

    YAMLFormatterFixture() : IIPBase("ForwarderCfg.yaml", "test") {
        BOOST_TEST_MESSAGE("Setup YAMLFormatterTest fixture");

        _log_dir = _config_root["LOGGING_DIR"].as<std::string>();
        std::vector<std::string> map = _config_root["DAQ_MAPPING"].as<
                std::vector<std::string>>();

        BOOST_TEST_REQUIRE(ut::framework::master_test_suite().argc == 3);
        BOOST_TEST(ut::framework::master_test_suite().argv[1] == "--data");
        BOOST_TEST_MESSAGE(ut::framework::master_test_suite().argv[2]);

        std::string path(ut::framework::master_test_suite().argv[2]);
        _hdrpath = fs::path(path);
        _root = YAML::LoadFile(path);

        _yml = std::unique_ptr<YAMLFormatter>(new YAMLFormatter(map));
    }

    ~YAMLFormatterFixture() {
        BOOST_TEST_MESSAGE("TearDown YAMLFormatterTest fixture");
        std::string log = _log_dir + "/test.log.0";
        std::remove(log.c_str());
    }
};

BOOST_FIXTURE_TEST_SUITE(YAMLFormatterTest, YAMLFormatterFixture);

BOOST_AUTO_TEST_CASE(constructor) {

}

BOOST_AUTO_TEST_CASE(get) {
    auto bool_key1 = _yml->get<bool>(_root, "bool_key1");
    BOOST_CHECK_EQUAL(*bool_key1, true);

    auto bool_key2 = _yml->get<bool>(_root, "bool_key2");
    BOOST_CHECK_EQUAL(*bool_key2, true);

    auto str_key1 = _yml->get<std::string>(_root, "str_key1");
    BOOST_CHECK_EQUAL(*str_key1, "SIMPLE");

    auto str_key3 = _yml->get<std::string>(_root, "str_key2");
    BOOST_CHECK_EQUAL(*str_key3, "3");

    // int
    auto int_key1 = _yml->get<int>(_root, "int_key");
    BOOST_CHECK_EQUAL(*int_key1, 3);

    // testing int as string and string as int
    // These test cases are covered in write_header where the data types are
    // checked bool, int, string in that order.

    // positive int32_t
    auto int32_key = _yml->get<int32_t>(_root, "int32_key");
    BOOST_CHECK_EQUAL(*int32_key, 2147483647);

    // negative int32_t
    auto nint32_key = _yml->get<int32_t>(_root, "nint32_key");
    BOOST_CHECK_EQUAL(*nint32_key, -2147483647);

    // int32_t overflow
    auto bint32_key = _yml->get<int32_t>(_root, "bint32_key");
    BOOST_CHECK_NE(*bint32_key, 2147483648);

    auto long_key = _yml->get<int64_t>(_root, "long_key");
    BOOST_CHECK_EQUAL(*long_key, 9223372036854775807);

    auto nlong_key = _yml->get<int64_t>(_root, "nlong_key");
    BOOST_CHECK_EQUAL(*nlong_key, -9223372036854775807);

    // tried long overflow but boost unit test does not like it
    auto blong_key = _yml->get<int64_t>(_root, "blong_key");
    // BOOST_CHECK_EQUAL(*blong_key, 9223372036854775808);

    // double
    auto dbl_key = _yml->get<double>(_root, "dbl_key");
    BOOST_CHECK_EQUAL(*dbl_key, 3.14159265359);

    // null key
    auto nll_key = _yml->get<std::string>(_root, "nll_key");
    BOOST_CHECK_EQUAL(bool(nll_key), false);

    // empty string
    auto emty_key = _yml->get<std::string>(_root, "emty_key");
    BOOST_CHECK_EQUAL(*emty_key, "");
}

BOOST_AUTO_TEST_CASE(contains) {
    BOOST_CHECK_EQUAL(_yml->contains("SIMPLE"), true);
    BOOST_CHECK_EQUAL(_yml->contains("NAXIS"), true);
    BOOST_CHECK_EQUAL(_yml->contains("NAXIS1"), true);
    BOOST_CHECK_EQUAL(_yml->contains("SOMETHING"), false);
    BOOST_CHECK_EQUAL(_yml->contains("XSION"), false);
    BOOST_CHECK_EQUAL(_yml->contains(""), false);
}

BOOST_AUTO_TEST_CASE(write_key) {
    int status = -1;
    YAML::Node n = _root["write_key"];

    FitsOpener pix_file(fs::path("./test.fits"), FILE_MODE::WRITE_ONLY);
    fitsfile* pix = pix_file.get();
    fits_movabs_hdu(pix, 1, nullptr, &status);

    for (auto&& x : n) {
        _yml->write_key(pix, x);
    }

    char* comment;
    int int32_k, double_k, int64_k, null_k;

    bool simple;
    fits_read_key(pix, TLOGICAL, "NOSIMPLE", &simple, comment, &status);
    BOOST_CHECK_EQUAL(simple, true);

    char str[6];
    fits_read_key(pix, TSTRING, "STR", str, comment, &status);
    BOOST_CHECK_EQUAL(str, "string");

    char str2[1];
    fits_read_key(pix, TSTRING, "STR2", str2, comment, &status);
    BOOST_CHECK_EQUAL(std::string(str2), "3");

    // For edge case like string "3", cfitsio lets you read it as an int and
    // it is a valid operation. The code below works.
    // int str3;
    // fits_read_key(pix, TINT, "STR2", &str3, comment, &status);

    int32_t i;
    fits_read_key(pix, TINT, "INT32", &i, comment, &status);
    BOOST_CHECK_EQUAL(i, 3000);

    // bug in cfitsio. There is a floating point precision error in cfitsio and
    // for some reason, when the number gets too large, it is evaluating as
    // string instead of long long or int64_t.
    // Err message: "evaluating string as a long integer: 9.22337203685478E+18"
    // max size of long is 9223372036854775807 and cfitsio doesn't like it. So,
    // last digit is removed to preserve test case validity.
    int64_t l;
    fits_read_key(pix, TLONGLONG, "INT64", &l, comment, &status);
    BOOST_CHECK_EQUAL(l, 92233720368547);

    double d;
    fits_read_key(pix, TDOUBLE, "DOUBLE", &d, comment, &status);
    BOOST_CHECK_EQUAL(d, 0.819181);

    // cfitsio lets you access every value but if the value is null,
    // this routine simply returns error without even error
    // message. So, the person who writes the keywords must know this value is
    // null or not. and when he/she retrieves it, he/she must use the same
    // data type that he/she puts in.
    // char null_key[80];
    // fits_read_key(pix, TSTRING, "NULLKEY", null_key, comment, &status);

    // key has comment field missing
    YAML::Node bad_key = _root["BAD_KEY"];
    YAML::Node kk = bad_key["KEY"];
    BOOST_CHECK_THROW(_yml->write_key(pix, kk), L1::CannotFormatFitsfile);

    // fs::remove("test.fits");
}

BOOST_AUTO_TEST_CASE(write_header) {

    // non-existing files
    fs::path pix("pix.fits");
    fs::path hdr("header.fits");
    BOOST_CHECK_THROW(_yml->write_header(pix, hdr), L1::CfitsioError);

    // file exists but pixel filename is not valid
    FitsOpener pix_file(pix, FILE_MODE::WRITE_ONLY);
    BOOST_CHECK_THROW(_yml->write_header(pix, hdr),
            L1::CannotFormatFitsfile);
    fs::remove(pix);

    // header file does not exist
    fs::path file("image_00-R00S00.fits");
    FitsOpener pix_file2(file, FILE_MODE::WRITE_ONLY);
    BOOST_CHECK_THROW(_yml->write_header(file, hdr), L1::CannotFormatFitsfile);

    // header file is not valid yaml
    BOOST_CHECK_THROW(_yml->write_header(file, file), L1::CannotFormatFitsfile);
}

BOOST_AUTO_TEST_SUITE_END()
