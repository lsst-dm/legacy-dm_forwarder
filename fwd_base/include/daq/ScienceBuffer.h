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

#ifndef SCIENCEBUFFER_H
#define SCIENCEBUFFER_H

#include <ims/science/Data.hh>
#include <ims/science/Source.hh>
#include <ims/Stripe.hh>
#include "daq/Data.h"

class ScienceBuffer {
  public:
    ScienceBuffer(int64_t samples);
    ~ScienceBuffer();
    Data process(IMS::Science::Source& source);

  private:
    char* _buffer;
    IMS::Science::Data _data;
    IMS::Stripe* _ccds;
    IMS::Stripe* _ccd[3];
    int64_t _samples;
};

#endif
