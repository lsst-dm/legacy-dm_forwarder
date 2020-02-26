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

#ifndef DATA_H
#define DATA_H

#include <vector>
#include <ims/Stripe.hh>
#include <ims/SourceMetadata.hh>

typedef std::vector<IMS::Stripe> Stripe1d;

class Data {
    public:
        Data(int num_ccds,
             int64_t samples,
             IMS::Stripe** stripes,
             const IMS::SourceMetadata& meta);

        int32_t pixel(uint64_t index, int sensor, int segment);

        std::vector<long> naxes();

        int64_t samples();


    private:
        IMS::SourceMetadata _meta;
        std::vector<Stripe1d> _pix;
        int64_t _samples;
};

#endif
