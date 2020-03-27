//
// Created by Samuel Cappon on 3/5/20.
//

#ifndef FITSCOMPARATOR_H
#define FITSCOMPARATOR_H

#include <fitsio.h>
#include <vector>
#include <string>
#include <map>
using namespace std;

class FitsComparator{
    public:
        FitsComparator(string fitsfile1, string fitsfile2);

        ~FitsComparator();

        vector<string> get_segments(fitsfile *fptr);

        multimap<string, string> get_header_values(fitsfile *fptr);

        vector<vector<int32_t>> get_pixels(fitsfile *fptr);

        bool compare_by_hdu();

        bool compare_by_segments();

        bool compare_headers();

    private:
        fitsfile* _fptr;
        fitsfile* _fptr2;
};
#endif
