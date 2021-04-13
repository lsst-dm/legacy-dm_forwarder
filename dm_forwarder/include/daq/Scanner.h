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

#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include <ims/Image.hh>
#include <ims/Store.hh>
#include <ims/Catalog.hh>
#include <ims/Processor.hh>
#include <osa/TimeStamp.hh>

/**
 * Corrupted metadata images can have wrong timestamps and the behavior is
 * undefined for processing those images - meaning there is no guarantee
 * catchuparchiver will catchup those images or not.
 */
class Scanner : public IMS::Processor {
    public:
        Scanner(const std::string partition, const int minutes);
        void process(const IMS::Id& id);
        std::vector<std::string> get_images();

    private:
        // DAQ images store
        IMS::Store _store;

        // images older than this time are catchuped
        OSA::TimeStamp _timestamp;

        // list of images to catchup
        std::vector<std::string> _images;
};

#endif
