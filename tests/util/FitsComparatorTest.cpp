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
#include <core/Exceptions.h>
#include <util/FitsComparator.h>
#include <core/IIPBase.h>
#include <fitsio.h>
#include <map>
#include <iostream>


struct FitsComparatorFixture : IIPBase {
    std::unique_ptr<FitsComparator> _fc;
    std::unique_ptr<FitsComparator> _fc2;
    std::unique_ptr<FitsComparator> _fc3;
    std::string _log_dir;
    int _status = 0;
    FitsComparatorFixture() : IIPBase("ForwarderCfg.yaml", "test"){
        BOOST_TEST_MESSAGE("Setup FitsComparator test");
        _log_dir = _config_root["LOGGING_DIR"].as<std::string>();
        int i, ii;
        long naxes[] = {2,2}, fpixel[2] = {1, 1},pix[] = {100, 101, 102, 103}, pix2[] = {0, 1, 2, 3,},
                pix3[] = {10, 10000, 8976, 987103};
        std::string segments[] = {"Segment10", "Segment11", "Segment12", "Segment13", "Segment14", "Segment15",
                                  "Segment16", "Segment17", "Segment07", "Segment06", "Segment05", "Segment04",
                                  "Segment03", "Segment02", "Segment01", "Segment00"};

        std::unique_ptr<fitsfile*> fptr(new fitsfile*);
        std::unique_ptr<fitsfile*> fptr2(new fitsfile*);
        std::unique_ptr<fitsfile*> fptr3(new fitsfile*);
        std::unique_ptr<fitsfile*> fptr4(new fitsfile*);

        fits_create_file(fptr.get(), "test1.fits", &_status);
        fits_create_file(fptr2.get(), "test2.fits", &_status);
        fits_create_file(fptr3.get(), "extra-headers.fits", &_status);
        fits_create_file(fptr4.get(), "out-of-order.fits", &_status);

        for(i = 0; i < 17; i++) {
            fits_create_img(*fptr, LONG_IMG, 2, naxes, &_status);
            fits_create_img(*fptr2, LONG_IMG, 2, naxes, &_status);
            fits_create_img(*fptr3, LONG_IMG, 2, naxes, &_status);

            if (i != 0){
                fits_write_key_str(*fptr, "EXTNAME", segments[i - 1].c_str(), " ", &_status);
                fits_write_key_str(*fptr2, "EXTNAME", segments[i - 1].c_str(), " ", &_status);
                fits_write_key_str(*fptr3, "EXTNAME", segments[i - 1].c_str(), " ", &_status);
            }

            fits_write_key_str(*fptr3, "Test Key", "Test Value", " ", &_status);

            if(i < 6) {
                fits_write_pix(*fptr, TINT, fpixel, naxes[0] * naxes[1], pix, &_status);
                fits_write_pix(*fptr2, TINT, fpixel, naxes[0] * naxes[1], pix, &_status);
                fits_write_pix(*fptr3, TINT, fpixel, naxes[0] * naxes[1], pix, &_status);
            } else if(i < 12){
                fits_write_pix(*fptr, TINT, fpixel, naxes[0] * naxes[1], pix2, &_status);
                fits_write_pix(*fptr2, TINT, fpixel, naxes[0] * naxes[1], pix2, &_status);
                fits_write_pix(*fptr3, TINT, fpixel, naxes[0] * naxes[1], pix2, &_status);
            } else {
                fits_write_pix(*fptr, TINT, fpixel, naxes[0] * naxes[1], pix3, &_status);
                fits_write_pix(*fptr2, TINT, fpixel, naxes[0] * naxes[1], pix3, &_status);
                fits_write_pix(*fptr3, TINT, fpixel, naxes[0] * naxes[1], pix3, &_status);
            }
        }


        int n[] = {1, 2, 15, 9, 5, 6, 7, 8, 11, 10, 3, 12, 13, 14, 4, 16, 17};
        for (int a : n){
            fits_movabs_hdu(*fptr, a, NULL, &_status);
            fits_copy_hdu(*fptr, *fptr4, 3, &_status);
        }


        fits_close_file(*fptr, &_status);
        fits_close_file(*fptr2, &_status);
        fits_close_file(*fptr3, &_status);
        fits_close_file(*fptr4, &_status);



        _fc = std::unique_ptr<FitsComparator>(new FitsComparator("test1.fits", "test2.fits"));
        _fc2 = std::unique_ptr<FitsComparator>(new FitsComparator("test1.fits", "extra-headers.fits"));
        _fc3 = std::unique_ptr<FitsComparator>(new FitsComparator("test1.fits", "out-of-order.fits"));
    }
    ~FitsComparatorFixture(){
        BOOST_TEST_MESSAGE("Teardown FitsComparatorTest fixture");
        std::string log = _log_dir + "/test.log.0";
        std::remove(log.c_str());
        std::remove("test1.fits");
        std::remove("test2.fits");
        std::remove("extra-headers.fits");
        std::remove("out-of-order.fits");

    }
};

BOOST_FIXTURE_TEST_SUITE(FitsComparatorTest, FitsComparatorFixture);

BOOST_AUTO_TEST_CASE(constructor){
    BOOST_CHECK_NO_THROW(FitsComparator f("test1.fits", "test2.fits"));

    BOOST_CHECK_THROW(FitsComparator f("_file1", "test2.fits"), L1::CfitsioError);

    BOOST_CHECK_THROW(FitsComparator f("file1.fits", "file.f"), L1::CfitsioError);
}

BOOST_AUTO_TEST_CASE(compare_by_hdu){
    BOOST_CHECK_EQUAL(_fc->compare_by_hdu(), true);

    BOOST_CHECK_EQUAL(_fc2->compare_by_hdu(), true);

    BOOST_CHECK_EQUAL(_fc3->compare_by_hdu(), false);
}

BOOST_AUTO_TEST_CASE(compare_by_segments){
    BOOST_CHECK_EQUAL(_fc->compare_by_segments(), true);

    BOOST_CHECK_EQUAL(_fc2->compare_by_segments(), true);

    BOOST_CHECK_EQUAL(_fc3->compare_by_segments(), true);
}

BOOST_AUTO_TEST_CASE(compare_headers){
    BOOST_CHECK_EQUAL(_fc->compare_headers(), true);

    BOOST_CHECK_EQUAL(_fc2->compare_headers(), false);

    BOOST_CHECK_EQUAL(_fc3->compare_headers(), true);
}

BOOST_AUTO_TEST_CASE(get_segments){
    std::vector<std::string> s;
    std::unique_ptr<fitsfile*> fptr(new fitsfile*);
    fits_open_file(fptr.get(), "test1.fits", READONLY, &_status);

    BOOST_CHECK_NO_THROW(s = _fc->get_segments(*fptr));

    BOOST_CHECK_EQUAL(s.size(), 16);
}

BOOST_AUTO_TEST_CASE(get_header_values){
    std::unique_ptr<fitsfile*> fptr(new fitsfile*);
    fits_open_file(fptr.get(), "test1.fits", READONLY, &_status);

    BOOST_CHECK_NO_THROW(_fc->get_header_values(*fptr));
}

BOOST_AUTO_TEST_CASE(get_pixels){
    std::unique_ptr<fitsfile*> fptr(new fitsfile*);
    fits_open_file(fptr.get(), "test1.fits", READONLY, &_status);
    fits_movabs_hdu(*fptr, 2, NULL, &_status);

    BOOST_CHECK_NO_THROW(_fc->get_pixels(*fptr));
}

BOOST_AUTO_TEST_SUITE_END()