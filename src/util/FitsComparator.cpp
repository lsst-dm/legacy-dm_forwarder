//
// Created by Samuel Cappon on 3/5/20.
//

#include <util/FitsComparator.h>
#include <fitsio.h>
#include <core/Exceptions.h>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <core/SimpleLogger.h>

using namespace std;

FitsComparator::FitsComparator(string fitsfile1, string fitsfile2) {
    int status = 0;
    int hdunum;

        fits_open_file(&_fptr, fitsfile1.c_str(), READONLY, &status);
        fits_open_file(&_fptr2, fitsfile2.c_str(), READONLY, &status);
        if(status){
            string err = "Failed to open files " + fitsfile1 + " and " + fitsfile2 + ", check paths and extensions.";
            LOG_CRT << err;
            throw L1::CfitsioError(err);
        }
        fits_get_num_hdus(_fptr, &hdunum, &status);
        if(hdunum != 17){
            string warning = "File " + fitsfile1 + " does not have 17 hdu's. Total hdu = " + to_string(hdunum);
            LOG_CRT << warning;
        }
        fits_get_num_hdus(_fptr2, &hdunum, &status);
        if(hdunum != 17){
            string warning = "File " + fitsfile2 + " does not have 17 hdu's. Total hdu = " + to_string(hdunum);
            LOG_CRT << warning;
        }
}

FitsComparator::~FitsComparator() {
    int status = 0;
    fits_close_file(_fptr, &status);
    fits_close_file(_fptr2, &status);
}

vector<string> FitsComparator::get_segments(fitsfile *fptr) {
    int status = 0, i;
    char segment_value[FLEN_VALUE], segment_comment[FLEN_COMMENT];
    vector<string> segments;

    for (i = 2; i <= 17; i++) {
        fits_movabs_hdu(fptr, i, NULL, &status);
        fits_read_key(fptr, TSTRING, "EXTNAME", segment_value, segment_comment, &status);
        if(status){
            string err = "Cannot read EXTNAME key from hdu " + to_string(i);
            LOG_CRT << err;
            throw L1::CfitsioError(err);
        }
        string segment = string(segment_value);
        segments.push_back(segment);
    }
    return segments;
}

multimap<string, string> FitsComparator::get_header_values(fitsfile *fptr){
    int status = 0, i, nkeys;
    char keyname[FLEN_KEYWORD], value[FLEN_VALUE], comment[FLEN_COMMENT];

    fits_get_hdrspace(fptr, &nkeys, NULL, &status);

    multimap<string, string> header_values;

    for(i = 1; i <= nkeys; i++){
        fits_read_keyn(fptr, i, keyname, value, comment, &status);
        header_values.insert(pair<string, string>(keyname, value));
    }
    return header_values;
}


vector<vector<int32_t>> FitsComparator::get_pixels(fitsfile *fptr){
    int bitpix, naxis, i, status = 0;
    long naxes[2];
    long fpixel[2];

    fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status);
    if(status){
        string err = "Error getting image parameters.";
        LOG_CRT << err;
        throw L1::CfitsioError(err);
    }

    vector<vector<int32_t>> pixels;
    vector<int32_t> row;

    fpixel[0] = 1;
    fpixel[1] = 1;

    unique_ptr<int32_t[]> pix(new int32_t[naxes[0]]);

    for(i = 0; i < naxes[1]; i++) {
        fits_read_pix(fptr, TINT, fpixel, naxes[0], NULL, pix.get(), NULL, &status);
        row.assign(pix.get(), pix.get() + naxes[0]);
        pixels.push_back(row);
        fpixel[0]++;
    }

    return pixels;
}


bool FitsComparator::compare_by_segments(){
    int status = 0, i, ii;
    vector<string> file1_segments = get_segments(_fptr);

    for (i = 2; i < 18; i++) {
        char segment_value[FLEN_VALUE], segment_comment[FLEN_COMMENT];

        fits_movabs_hdu(_fptr, i, NULL, &status);
        fits_movabs_hdu(_fptr2, i, NULL, &status);

        fits_read_key(_fptr2, TSTRING, "EXTNAME", segment_value, segment_comment, &status);
        if(status){
            string err = "Cannot read EXTNAME key from hdu " + to_string(i);
            LOG_CRT << err;
            throw L1::CfitsioError(err);
        }

        string file2_segment = string(segment_value);

        if (file2_segment != file1_segments[i - 2]) {
            for (int ii = 0; ii < file1_segments.size(); ii++) {
                if (file2_segment == file1_segments[ii]) {
                    fits_movabs_hdu(_fptr, ii+2, NULL, &status);
                    break;
                }
            }
        }
        if (get_pixels(_fptr) != get_pixels(_fptr2)) return false;
    }
    return true;
}


bool FitsComparator::compare_by_hdu(){
    int status = 0, i;

    for (i = 2; i < 18; i++) {
        fits_movabs_hdu(_fptr, i, NULL, &status);
        fits_movabs_hdu(_fptr2, i, NULL, &status);

        if(get_pixels(_fptr) != get_pixels(_fptr2)) return false;
    }

    return true;
}


bool FitsComparator::compare_headers(){
    int status = 0, i, ii;
    bool r = true;
    vector<string> file1_segments = get_segments(_fptr);

    for(i = 1; i < 18; i++) {
        char segment_value[FLEN_VALUE], segment_comment[FLEN_COMMENT];
        string file2_segment;

        fits_movabs_hdu(_fptr, i, NULL, &status);
        fits_movabs_hdu(_fptr2, i, NULL, &status);

        if(i > 1) {
            fits_read_key(_fptr2, TSTRING, "EXTNAME", segment_value, segment_comment, &status);
            if(status){
                string err = "Cannot read EXTNAME key from hdu " + to_string(i);
                LOG_CRT << err;
                throw L1::CfitsioError(err);
            }
            file2_segment = string(segment_value);

            if (file2_segment != file1_segments[i - 2]) {
                for (int ii = 0; ii < file1_segments.size(); ii++) {
                    if (file2_segment == file1_segments[ii]) {
                        fits_movabs_hdu(_fptr, ii + 2, NULL, &status);
                        break;
                    }
                }
            }
            cout << "\n=== " << file2_segment << "\n";
        } else cout << "=== " << "PRIMARY" << "\n";

        multimap<string, string> file1_header_values = get_header_values(_fptr);
        multimap<string, string> file2_header_values = get_header_values(_fptr2);
        map<string, string> symmetric_difference;

        set_symmetric_difference(file1_header_values.begin(), file1_header_values.end(),
                                file2_header_values.begin(), file2_header_values.end(),
                                inserter(symmetric_difference, symmetric_difference.end()));

        if(symmetric_difference.size() > 0) {
            for (pair <string, string> k : symmetric_difference) {
                cout << k.first << "\n";
            }
            r = false;
        } else cout << "Segments are equal\n";
    }
    return r;
}