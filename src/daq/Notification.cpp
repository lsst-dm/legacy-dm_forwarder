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
#include <core/Exceptions.h>
#include <core/SimpleLogger.h>
#include <daq/Notification.h>

#define TIMEOUT_SECONDS 15
#define TIMEOUT TIMEOUT_SECONDS*1000*1000

Notification::Notification(const std::string partition, int barrier_timeout) {
    _barrier_timeout = barrier_timeout;
    _store = std::unique_ptr<IMS::Store>(new IMS::Store(partition.c_str()));
    _stream = std::unique_ptr<IMS::Stream>(new IMS::Stream(*_store,
                _barrier_timeout));
}

void Notification::start() {
    _stream.reset();
    _stream = std::unique_ptr<IMS::Stream>(new IMS::Stream(*_store,
                _barrier_timeout));
    LOG_INF << "Notification stream started with barrier_timeout = " << _barrier_timeout;
}

void Notification::block(Info::MODE mode, const std::string image_id, const std::string folder) {

    if (mode != Info::MODE::LIVE) {
        return;
    }

    // We make the assumption that we open the stream on start up of the Forwarder.  This
    // is before getting an endReadout (which calls this method), so we can't be 
    // "ahead" of reading the data coming from the stream, only behind. Any images that
    // were on the stream that we skip will be handled by the catchup archiver.
    //
    while (true) {
        //
        // Call IMS::IMAGE to attempt to set up an image from the stream. If the image 
        // is null, there were no pending images after TIMEOUT so we thrown an exception.
        //
        LOG_DBG << "Trying to read stream to find image " << image_id;
        IMS::Image image(*_store, *_stream, _barrier_timeout);
        if (!image)  {
            std::ostringstream err;
            err << "Wasn't able to read image in "<< _barrier_timeout*1000*1000 << " seconds";
            LOG_CRT << err.str();
            throw L1::CannotFetchPixel(err.str());
        }

        //
        // Examine the metadata.  If it doesn't match, move on and try
        // to read next image; not matching can happen if we didn't get an endReadout
        // about an image that made it onto the stream, which in practice means the stream
        // has two or more images pending to read; print out a message and continue 
        // with the loop.
        //
        LOG_DBG << "Acquired image ";
    
        IMS::ImageMetadata meta = image.metadata();
        std::string meta_name = std::string(meta.name());
        std::string meta_folder = std::string(meta.folder());
        LOG_INF << "dumping elements(); check stdout";
        LOG_INF << "number of elements in image.metadata.elements() is " <<  image.metadata().elements().numof();
        image.metadata().elements().dump();
    
        if (meta_name != image_id || meta_folder != folder) {
            LOG_INF << "Currently streaming image " << meta_name << " in "
                    << meta_folder << " is not what is being expected "
                    << image_id << " from " << folder;
            continue;
        }
    
        // We have the Image, the metadata matches, so now we block for the pixels.
        LOG_DBG << "Barrier blocking for image " << image_id;
        IMS::Barrier barrier(image);
        barrier.block();
        LOG_DBG << "Barrier released for image " << image_id;
        //
        // if image is null, then there was a TIMEOUT, which means the pixels
        // never materialized.  Print an error, and throw an exception.
        //
        if (!image) {
            std::ostringstream err;
            err << "image " << image_id << " was not found after blocking " << _barrier_timeout*1000*1000 << " blocking at barrier";
            LOG_CRT << err.str();
            throw L1::CannotFetchPixel(err.str());
        }
        // We have the image, return
        return;
    }
}
