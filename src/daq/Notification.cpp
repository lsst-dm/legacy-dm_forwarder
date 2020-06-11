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

#include <ims/Image.hh>
#include <ims/ImageMetadata.hh>
#include <ims/Barrier.hh>
#include <core/SimpleLogger.h>
#include <daq/Notification.h>

#define TIMEOUT 15*1000*1000

Notification::Notification(const std::string partition) {
    _store = std::unique_ptr<IMS::Store>(new IMS::Store(partition.c_str()));
    _stream = std::unique_ptr<IMS::Stream>(new IMS::Stream(*_store,
                TIMEOUT));
}

void Notification::start() {
    _stream.reset();
    _stream = std::unique_ptr<IMS::Stream>(new IMS::Stream(*_store,
                TIMEOUT));
    LOG_INF << "Notification stream started";
}

void Notification::block(Info::MODE mode,
                         const std::string image_id,
                         const std::string folder) {

    if (mode != Info::MODE::LIVE) {
        return;
    }

    LOG_DBG << "IMS::Image blocking for image " << image_id;
    IMS::Image image(*_store, *_stream, TIMEOUT);
    LOG_DBG << "Acquired image " << image_id;

    IMS::ImageMetadata meta = image.metadata();
    std::string meta_name = std::string(meta.name());
    std::string meta_folder = std::string(meta.folder());

    if (meta_name != image_id || meta_folder != folder) {
        LOG_CRT << "Currently streaming image " << meta_name << " in "
                << meta_folder << " is not what is being expected "
                << image_id << " from " << folder;
        return;
    }

    LOG_DBG << "Barrier blocking for image " << image_id;
    IMS::Barrier barrier(image);
    barrier.block();
    LOG_DBG << "Barrier released for image " << image_id;
}
