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

#include <util/FitsComparator.h>
#include <fitsio.h>
#include <core/Exceptions.h>
#include <iostream>
#include <memory>
#include <algorithm>
#include <iterator>
#include <core/SimpleLogger.h>

FitsComparator::FitsComparator(std::string fitsfile1, std::string fitsfile2) {
    int status = 0;
    int hdunum;

    fits_open_file(_fptr.get(), fitsfile1.c_str(), READONLY, &status);
    fits_open_file(_fptr2.get(), fitsfile2.c_str(), READONLY, &status);

    fits_get_num_hdus(*_fptr, &hdunum, &status);
    if (hdunum != 17){
        std::string wrn = "File " + fitsfile1 + " does not have 17 hdu's. Total hdu = " + std::to_string(hdunum);
        LOG_WRN << wrn;
    }

    fits_get_num_hdus(*_fptr2, &hdunum, &status);
    if (hdunum != 17){
            std::string wrn = "File " + fitsfile2 + " does not have 17 hdu's. Total hdu = " + std::to_string(hdunum);
            LOG_WRN << wrn;
        }

    if (status){
        std::string err = "Failed to open files " + fitsfile1 + " and " + fitsfile2 + ", check paths and extensions.";
        LOG_CRT << err;
        throw L1::CfitsioError(err);
    }
}

FitsComparator::~FitsComparator() {
    int status = 0;
    fits_close_file(*_fptr, &status);
    fits_close_file(*_fptr2, &status);
}

std::vector<std::string> FitsComparator::get_segments(fitsfile *fptr) {
    int status = 0, i;
    char segment_value[FLEN_VALUE], segment_comment[FLEN_COMMENT];
    std::vector<std::string> segments;

    for (i = 2; i <= 17; i++) {
        fits_movabs_hdu(fptr, i, NULL, &status);
        fits_read_key(fptr, TSTRING, "EXTNAME", segment_value, segment_comment, &status);
        if (status){
            std::string err = "Cannot read EXTNAME key from hdu " + std::to_string(i);
            LOG_CRT << err;
            throw L1::CfitsioError(err);
        }
        std::string segment = std::string(segment_value);
        segments.push_back(segment);
    }

    return segments;
}

std::multimap<std::string, std::string> FitsComparator::get_header_values(fitsfile *fptr){
    int status = 0, i, nkeys;
    char keyname[FLEN_KEYWORD], value[FLEN_VALUE], comment[FLEN_COMMENT];

    fits_get_hdrspace(fptr, &nkeys, NULL, &status);

    std::multimap<std::string, std::string> header_values;

    for (i = 1; i <= nkeys; i++){
        fits_read_keyn(fptr, i, keyname, value, comment, &status);
        header_values.insert(std::pair<std::string, std::string>(keyname, value));
    }

    if (status){
        std::string err = "Error getting header values";
        LOG_CRT << err;
        throw L1::CfitsioError(err);
    }

    return header_values;
}


std::vector<std::vector<int32_t>> FitsComparator::get_pixels(fitsfile *fptr){
    int bitpix, naxis, i, status = 0;
    long naxes[2];
    long fpixel[2];

    fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status);

    std::vector<std::vector<int32_t>> pixels;
    std::vector<int32_t> row;

    fpixel[0] = 1;
    fpixel[1] = 1;

    std::unique_ptr<int32_t[]> pix(new int32_t[naxes[0]]);

    for (i = 0; i < naxes[1]; i++) {
        fits_read_pix(fptr, TINT, fpixel, naxes[0], NULL, pix.get(), NULL, &status);
        row.assign(pix.get(), pix.get() + naxes[0]);
        pixels.push_back(row);
        fpixel[0]++;
    }

    if (status){
        std::string err = "Error getting pixels.";
        LOG_CRT << err;
        throw L1::CfitsioError(err);
    }

    return pixels;
}


bool FitsComparator::compare_by_segments(){
    int status = 0, i, ii;
    std::vector<std::string> file1_segments = get_segments(*_fptr);

    for (i = 2; i < 18; i++) {
        char segment_value[FLEN_VALUE], segment_comment[FLEN_COMMENT];

        fits_movabs_hdu(*_fptr, i, NULL, &status);
        fits_movabs_hdu(*_fptr2, i, NULL, &status);

        fits_read_key(*_fptr2, TSTRING, "EXTNAME", segment_value, segment_comment, &status);
        if (status){
            std::string err = "Cannot read EXTNAME key from hdu " + std::to_string(i);
            LOG_CRT << err;
            throw L1::CfitsioError(err);
        }

        std::string file2_segment = std::string(segment_value);

        if (file2_segment != file1_segments[i - 2]) {
            for (int ii = 0; ii < file1_segments.size(); ii++) {
                if (file2_segment == file1_segments[ii]) {
                    fits_movabs_hdu(*_fptr, ii+2, NULL, &status);
                    break;
                }
            }
        }
        if (get_pixels(*_fptr) != get_pixels(*_fptr2)) return false;

    }

    if (status){
        std::string err = "Error trying to compare by segments";
        LOG_CRT << err;
        throw L1::CfitsioError(err);
    }

    return true;
}


bool FitsComparator::compare_by_hdu(){
    int status = 0, i;

    for (i = 2; i < 18; i++) {
        fits_movabs_hdu(*_fptr, i, NULL, &status);
        fits_movabs_hdu(*_fptr2, i, NULL, &status);

        if(get_pixels(*_fptr) != get_pixels(*_fptr2)) return false;
    }

    if (status){
        std::string err = "Error trying to compare by hdu";
        LOG_CRT << err;
        throw L1::CfitsioError(err);
    }

    return true;
}


bool FitsComparator::compare_headers(){
    int status = 0, i, ii;
    bool r = true;
    std::vector<std::string> file1_segments = get_segments(*_fptr);

    for(i = 1; i < 18; i++) {
        char segment_value[FLEN_VALUE], segment_comment[FLEN_COMMENT];
        std::string file2_segment;

        fits_movabs_hdu(*_fptr, i, NULL, &status);
        fits_movabs_hdu(*_fptr2, i, NULL, &status);

        if (i > 1) {
            fits_read_key(*_fptr2, TSTRING, "EXTNAME", segment_value, segment_comment, &status);
            if (status){
                std::string err = "Cannot read EXTNAME key from hdu " + std::to_string(i);
                LOG_CRT << err;
                throw L1::CfitsioError(err);
            }
            file2_segment = std::string(segment_value);

            if (file2_segment != file1_segments[i - 2]) {
                for (int ii = 0; ii < file1_segments.size(); ii++) {
                    if (file2_segment == file1_segments[ii]) {
                        fits_movabs_hdu(*_fptr, ii + 2, NULL, &status);
                        break;
                    }
                }
            }
            std::cout << std::endl << "=== " << file2_segment << std::endl;
        } else std::cout << "=== " << "PRIMARY" << std::endl;

        std::multimap<std::string, std::string> file1_header_values = get_header_values(*_fptr);
        std::multimap<std::string, std::string> file2_header_values = get_header_values(*_fptr2);
        std::map<std::string, std::string> symmetric_difference;

        set_symmetric_difference(file1_header_values.begin(), file1_header_values.end(),
                                file2_header_values.begin(), file2_header_values.end(),
                                inserter(symmetric_difference, symmetric_difference.end()));

        if (symmetric_difference.size() > 0) {
            for (std::pair <std::string, std::string> k : symmetric_difference) {
                std::cout << k.first << std::endl;
            }
            r = false;
        } else std::cout << "Segments are equal" << std::endl;
    }

    if (status){
        std::string err = "Error trying to compare headers";
        LOG_CRT << err;
        throw L1::CfitsioError(err);
    }

    return r;
}