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

#include <ims/Decoder.hh>
#include <ims/science/Data.hh>
#include <ims/wavefront/Data.hh>
#include <ims/guiding/Data.hh>
#include "daq/Data.h"
#include "daq/DAQDecoder.h"
#include "daq/ScienceBuffer.h"
#include "daq/WavefrontBuffer.h"
#include "daq/GuidingBuffer.h"

#define SAMPLES 8192

DAQDecoder::DAQDecoder(IMS::Image& img, const DAQ::LocationSet& filter)
      : IMS::Decoder(img, filter) {
    _samples = 0;
}

void DAQDecoder::process(IMS::Science::Source& source,
                         uint64_t length,
                         uint64_t offset) {
    _samples += IMS::Science::Data::samples(length);

    uint64_t QUANTA = IMS::Science::Data::bytes(SAMPLES);
    uint64_t remaining = length;

    while (remaining) {
        uint64_t quanta = remaining > QUANTA ? QUANTA : remaining;
        uint64_t current_samples = IMS::Science::Data::samples(quanta);

        ScienceBuffer buffer(current_samples);
        Data ccds = buffer.process(source);
        _data.push_back(ccds);

        offset += quanta;
        remaining -= quanta;
    }
}

void DAQDecoder::process(IMS::Guiding::Source& source,
                           uint64_t length,
                           uint64_t offset) {
    _samples += IMS::Guiding::Data::samples(length);

    uint64_t QUANTA = IMS::Guiding::Data::bytes(SAMPLES);
    uint64_t remaining = length;

    while (remaining) {
        uint64_t quanta = remaining > QUANTA ? QUANTA : remaining;
        uint64_t current_samples = IMS::Guiding::Data::samples(quanta);

        GuidingBuffer buffer(current_samples);
        Data ccds = buffer.process(source);
        _data.push_back(ccds);

        offset += quanta;
        remaining -= quanta;
    }
}

void DAQDecoder::process(IMS::Wavefront::Source& source,
                           uint64_t length,
                           uint64_t offset) {
    _samples += IMS::Wavefront::Data::samples(length);

    uint64_t QUANTA = IMS::Wavefront::Data::bytes(SAMPLES);
    uint64_t remaining = length;

    while (remaining) {
        uint64_t quanta = remaining > QUANTA ? QUANTA : remaining;
        uint64_t current_samples = IMS::Wavefront::Data::samples(quanta);

        WavefrontBuffer buffer(current_samples);
        Data ccds = buffer.process(source);
        _data.push_back(ccds);

        offset += quanta;
        remaining -= quanta;
    }
}

uint64_t DAQDecoder::samples() {
    return _samples;
}

std::vector<Data> DAQDecoder::data() {
    return _data;
}

bool DAQDecoder::valid() {
    return (_data.size() != 0);
}
