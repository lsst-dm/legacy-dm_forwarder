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

#ifndef PIXEL3D_H
#define PIXEL3D_H

#include <cstdint>

class Pixel3d {
    public:
        Pixel3d(uint64_t d1, uint64_t d2, uint64_t d3);
        ~Pixel3d();

        int32_t*** get();
        uint64_t d1();
        uint64_t d2();
        uint64_t d3();

    private:
        int32_t*** _arr;
        uint64_t _d1, _d2, _d3;
};

#endif
