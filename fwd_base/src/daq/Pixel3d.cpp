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

#include "daq/Pixel3d.h"

Pixel3d::Pixel3d(uint64_t d1, uint64_t d2, uint64_t d3) :
        _d1{d1},
        _d2{d2},
        _d3{d3} {
    _arr = new int32_t**[_d1];
    for (uint64_t i = 0; i < _d1; i++) {
        _arr[i] = new int32_t*[_d2];
        for (uint64_t j = 0; j < _d2; j++) {
            _arr[i][j] = new int32_t[_d3];
        }
    }
}

Pixel3d::~Pixel3d() {
    for (uint64_t i = 0; i < _d1; i++) {
        for (uint64_t j = 0; j < _d2; j++) {
            delete[] _arr[i][j];
        }
        delete[] _arr[i];
    }
    delete[] _arr;
}

int32_t*** Pixel3d::get() {
    return _arr;
}

uint64_t Pixel3d::d1() {
    return _d1;
}

uint64_t Pixel3d::d2() {
    return _d2;
}

uint64_t Pixel3d::d3() {
    return _d3;
}

