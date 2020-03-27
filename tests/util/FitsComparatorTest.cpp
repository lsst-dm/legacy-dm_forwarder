#include <boost/test/unit_test.hpp>
#include <core/Exceptions.h>
#include <util/FitsComparator.h>
#include <core/IIPBase.h>
#include <fitsio.h>
#include <map>
#include <iostream>

namespace ut = boost::unit_test;

struct FitsComparatorFixture : IIPBase {
    unique_ptr<FitsComparator> _fc;
    unique_ptr<FitsComparator> _fc2;
    string _file1;
    string _file2;
    string _log_dir;
    fitsfile* _fptr;
    int _status = 0;
    FitsComparatorFixture() : IIPBase("ForwarderCfg.yaml", "FitsComparatorTest"){
        BOOST_TEST_MESSAGE("Setup FitsComparator test");
        _log_dir = _config_root["LOGGING_DIR"].as<std::string>();
        BOOST_TEST_REQUIRE(ut::framework::master_test_suite().argc == 3);
        BOOST_TEST(ut::framework::master_test_suite().argv[1] == "--data");
        string path(ut::framework::master_test_suite().argv[2]);
        _file1 = path + "/test1.fits";
        _file2 = path + "/test2.fits";
        fits_open_file(&_fptr, _file1.c_str(), READONLY, &_status);
        _fc = unique_ptr<FitsComparator>(new FitsComparator(_file1, _file2));
        _fc2 = unique_ptr<FitsComparator>(new FitsComparator(path + "/test3.fits", path + "/test4.fits"));
    }
    ~FitsComparatorFixture(){
        BOOST_TEST_MESSAGE("Teardown FitsComparatorTest fixture");
        fits_close_file(_fptr, &_status);
    }
};

BOOST_FIXTURE_TEST_SUITE(FitsComparatorTest, FitsComparatorFixture);

BOOST_AUTO_TEST_CASE(constructor){
    BOOST_CHECK_NO_THROW(FitsComparator f(_file1, _file2));

    BOOST_CHECK_THROW(FitsComparator f("_file1", _file2), L1::CfitsioError);

    BOOST_CHECK_THROW(FitsComparator f(_file1, "file.f"), L1::CfitsioError);
}

BOOST_AUTO_TEST_CASE(compare_by_hdu){
    BOOST_CHECK_EQUAL(_fc->compare_by_hdu(), true);

    BOOST_CHECK_EQUAL(_fc2->compare_by_hdu(), false);
}

BOOST_AUTO_TEST_CASE(compare_by_segments){
        BOOST_CHECK_EQUAL(_fc->compare_by_segments(), true);

        BOOST_CHECK_EQUAL(_fc2->compare_by_segments(), false);
}

BOOST_AUTO_TEST_CASE(compare_headers){
        BOOST_CHECK_EQUAL(_fc->compare_headers(), true);

        BOOST_CHECK_EQUAL(_fc2->compare_headers(), false);
}

BOOST_AUTO_TEST_CASE(get_segments){
        vector<string> s;
        BOOST_CHECK_NO_THROW(s = _fc->get_segments(_fptr));

        BOOST_CHECK_EQUAL(s.size(), 16);
}

BOOST_AUTO_TEST_CASE(get_header_values){
        BOOST_CHECK_NO_THROW(_fc->get_header_values(_fptr));
}

BOOST_AUTO_TEST_CASE(get_pixels){
    fits_movabs_hdu(_fptr, 2, NULL, &_status);
    BOOST_CHECK_NO_THROW(_fc->get_pixels(_fptr));
}

BOOST_AUTO_TEST_SUITE_END()