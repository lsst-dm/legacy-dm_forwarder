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

#include <sstream>
#include <rms/InstructionList.hh>
#include <core/SimpleLogger.h>
#include <core/Exceptions.h>
#include <daq/Data.h>

Data::Data(int num_ccds,
           int64_t samples,
           IMS::Stripe** stripes,
           const IMS::SourceMetadata& meta) :
        _samples{ samples },
        _meta{ meta } {
    for (int i = 0; i < num_ccds; i++) {
        Stripe1d ccd{ stripes[i], stripes[i]+samples };
        _pix.push_back(ccd);
    }
}

int32_t Data::pixel(uint64_t index, int sensor, int segment) {
    return _pix[sensor][index].segment[segment];
}

std::vector<long> Data::naxes() {
    RMS::InstructionList instructions = _meta.instructions();

    uint32_t undercols = instructions.lookup(0)->operand();
    uint32_t precols = instructions.lookup(1)->operand();
    uint32_t readcols = instructions.lookup(2)->operand();
    uint32_t postcols = instructions.lookup(3)->operand();
    uint32_t readcols2 = instructions.lookup(4)->operand();
    uint32_t overcols = instructions.lookup(5)->operand();

    uint32_t prerows = instructions.lookup(6)->operand();
    uint32_t readrows = instructions.lookup(7)->operand();
    uint32_t postrows = instructions.lookup(8)->operand();
    uint32_t overrows = instructions.lookup(9)->operand();

    long naxis1 = undercols + readcols + readcols2 + overcols;
    long naxis2 = readrows + overrows;

    std::vector<long> axes{ naxis1, naxis2 };
    return axes;
}

int64_t Data::samples() {
    return _samples;
}
