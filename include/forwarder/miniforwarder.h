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

#ifndef MINIFORWARDER_H
#define MINIFORWARDER_H

#include <map>
#include <functional>
#include <boost/filesystem.hpp>
#include <yaml-cpp/yaml.h>

#include <core/IIPBase.h>
#include <core/SimplePublisher.h>
#include <core/HeartBeat.h>

#include <forwarder/Scoreboard.h>
#include <forwarder/MessageBuilder.h>
#include <forwarder/HeaderFetcher.h>
#include <forwarder/Formatter.h>
#include <forwarder/FileSender.h>
#include <forwarder/ReadoutPattern.h>
#include <forwarder/Info.h>
#include <daq/Notification.h>
#include <daq/DAQFetcher.h>

class miniforwarder : public IIPBase {
    public:
        miniforwarder(const std::string& config,
                      const std::string& log);
        ~miniforwarder();

        void on_message(const std::string&);
        void run();

        void health_check(const YAML::Node&);
        void xfer_params(const YAML::Node&);
        void header_ready(const YAML::Node&);
        void end_readout(const YAML::Node&);
        void process_ack(const YAML::Node&);
        void associated(const YAML::Node&);
        void scan(const YAML::Node&);

        void assemble(const std::string&);
        void format_with_header(std::vector<std::string>& ccds,
                                const std::string header);
        void publish_completed_msgs(const std::string image_id,
                                    const std::string to,
                                    std::vector<std::string>& ccds,
                                    const std::string session_id,
                                    const std::string job_num);
        void cleanup(const std::string image_id,
                     std::vector<std::string>& ccds,
                     const std::string header);


        void publish_ack(const YAML::Node&);
        void publish_xfer_complete(
                const std::string& obsid,
                const std::string& to,
                const std::string& session_id,
                const std::string& job_num);
        void publish_image_retrieval_for_archiving(
                const int& error_code,
                const std::string& obsid,
                const std::string& raft,
                const std::string& ccd,
                const std::string& filename,
                const std::string& desc);
        boost::filesystem::path create_dir(const boost::filesystem::path&);
        bool check_valid_board(const std::vector<std::string>& locs);
        std::string get_name();
        void register_fwd();

    private:
        std::string _name;
        std::string _ip_addr;
        std::string _hostname;
        std::string _consume_q;
        std::string _archive_q;
        std::string _telemetry_q;
        std::string _amqp_url;
        std::string _partition;
        std::string _folder;
        std::string _association_key;
        std::string _forwarder_list;
        int _seconds_to_update;
        int _seconds_to_expire;
        heartbeat_params _hb_params;
        Info::MODE _mode;
        redis_connection_params _redis_params;

        boost::filesystem::path _header_path;
        boost::filesystem::path _fits_path;

        std::map<const std::string,
            std::function<void (const YAML::Node&)> > _actions;
        std::vector<std::string> _daq_locations;

        std::unique_ptr<SimplePublisher> _pub;
        std::unique_ptr<Scoreboard> _db;
        std::unique_ptr<Watcher> _watcher;
        std::unique_ptr<Beacon> _beacon;
        std::unique_ptr<FileSender> _sender;
        std::unique_ptr<Notification> _notification;
        std::unique_ptr<ReadoutPattern> _pattern;

        MessageBuilder _builder;
        HeaderFetcher _hdr;
        ReadoutPattern _readoutpattern;
};

#endif
