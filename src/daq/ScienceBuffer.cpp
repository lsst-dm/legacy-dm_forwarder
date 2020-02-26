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
#include "core/Exceptions.h"
#include "core/SimpleLogger.h"
#include "daq/ScienceBuffer.h"

ScienceBuffer::ScienceBuffer(int64_t samples) :
        _samples{samples},
        _buffer(new char[IMS::Science::Data::bytes(samples)]),
        _data(_buffer, samples),
        _ccds(new IMS::Stripe[samples * 3]) {
    _ccd[0] = _ccds;
    _ccd[1] = _ccds + samples;
    _ccd[2] = _ccds + samples + samples;
}

ScienceBuffer::~ScienceBuffer() {
    delete[] _buffer;
    delete[] _ccds;
}

Data ScienceBuffer::process(IMS::Science::Source& source) {
    int32_t err_code = _data.read(source);
    if (err_code) {
        std::ostringstream err;
        err << "Invalid data from ScienceBuffer";
        LOG_CRT << err.str();
        throw L1::InvalidData(err.str());
    }
    _data.decode012(_ccd[0], _ccd[1], _ccd[2]);
    Data d(3, _samples, _ccd, source.metadata());
    return d;
}
