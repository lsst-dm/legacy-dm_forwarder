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

#ifndef DAQDECODER_H
#define DAQDECODER_H

#include <vector>
#include <daq/LocationSet.hh>
#include <ims/Image.hh>
#include <ims/Decoder.hh>
#include <ims/science/Source.hh>
#include <ims/guiding/Source.hh>
#include <ims/wavefront/Source.hh>
#include "daq/Data.h"

class DAQDecoder : public IMS::Decoder {
  public:
    DAQDecoder(IMS::Image& image, const DAQ::LocationSet& filter);
    void process(IMS::Science::Source&, uint64_t length, uint64_t offset);
    void process(IMS::Wavefront::Source&, uint64_t length, uint64_t offset);
    void process(IMS::Guiding::Source&, uint64_t length, uint64_t offset);
    uint64_t samples();
    std::vector<Data> data();
    bool valid();

  private:
    std::vector<Data> _data;
    uint64_t _samples;
};

#endif
