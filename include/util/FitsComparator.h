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

#ifndef FITSCOMPARATOR_H
#define FITSCOMPARATOR_H

#include <fitsio.h>
#include <vector>
#include <string>
#include <map>
#include <memory>

class FitsComparator{
    public:
       FitsComparator(std::string fitsfile1, std::string fitsfile2);

        ~FitsComparator();

        std::vector<std::string> get_segments(fitsfile *fptr);

        std::multimap<std::string, std::string> get_header_values(fitsfile *fptr);

        std::vector<std::vector<int32_t>> get_pixels(fitsfile *fptr);

        bool compare_segments(std::string seg1, std::string seg2);

        bool compare_by_hdu();

        bool compare_by_segments();

        bool compare_headers();

    private:
        std::shared_ptr<fitsfile*> _fptr{new fitsfile*};
        std::shared_ptr<fitsfile*> _fptr2{new fitsfile*};
};
#endif