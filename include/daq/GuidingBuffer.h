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


#ifndef GUIDINGBUFFER_H
#define GUIDINGBUFFER_H

#include <ims/guiding/Data.hh>
#include <ims/guiding/Source.hh>
#include <ims/Stripe.hh>
#include "daq/Data.h"

class GuidingBuffer {
  public:
    GuidingBuffer(int64_t samples);
    ~GuidingBuffer();
    Data process(IMS::Guiding::Source& source);

  private:
    char* _buffer;
    IMS::Guiding::Data _data;
    IMS::Stripe* _ccds;
    IMS::Stripe* _ccd[2];
    int64_t _samples;
};

#endif
