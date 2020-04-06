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

#include <cstdio>
#include <unistd.h> // gethostname
#include <netdb.h>
#include <future>

#include <ims/Image.hh>
#include <ims/ImageMetadata.hh>
#include <ims/Barrier.hh>

#include <core/Exceptions.h>
#include <core/Consumer.h>
#include <core/SimpleLogger.h>
#include <core/RedisConnection.h>
#include <forwarder/YAMLFormatter.h>
#include <forwarder/miniforwarder.h>

#define TIMEOUT 15*1000*1000

namespace fs = boost::filesystem;

miniforwarder::miniforwarder(const std::string& config,
                             const std::string& log) : IIPBase(config, log)
                                 , _hdr()
                                 , _readoutpattern(_config_root) {
    std::string work_dir, ip_host, redis_host, xfer_option;
    int redis_port, redis_db;
    YAML::Node pattern;
    try {
        work_dir = _config_root["WORK_DIR"].as<std::string>();
        ip_host = _config_root["BASE_BROKER_ADDR"].as<std::string>();
        redis_host = _config_root["REDIS_HOST"].as<std::string>();
        redis_port = _config_root["REDIS_PORT"].as<int>();
        redis_db = _config_root["REDIS_DB"].as<int>();
        xfer_option = _config_root["XFER_OPTION"].as<std::string>();

        // DAQ configurations
        _partition = _config_root["PARTITION"].as<std::string>();
        _folder = _config_root["FOLDER"].as<std::string>();

        // Forwarder configurations
        _name = _config_root["NAME"].as<std::string>();
        _daq_locations = _config_root[_partition]
                .as<std::vector<std::string>>();
        _consume_q = _config_root["CONSUME_QUEUE"].as<std::string>();
        _archive_q = _config_root["ARCHIVE_QUEUE"].as<std::string>();
        _telemetry_q = _config_root["TELEMETRY_QUEUE"].as<std::string>();
        _seconds_to_update = _config_root["SECONDS_TO_UPDATE"].as<int>();
        _seconds_to_expire = _config_root["SECONDS_TO_EXPIRE"].as<int>();

        // ReadoutPattern
        pattern = _config_root["PATTERN"];
    }
    catch (YAML::TypedBadConversion<std::string>& e) {
        LOG_CRT << "YAML bad conversion for std::string";
        exit(EXIT_FAILURE);
    }
    catch (YAML::TypedBadConversion<int>& e) {
        LOG_CRT << "YAML bad conversion for int";
        exit(EXIT_FAILURE);
    }
    catch (YAML::TypedBadConversion<std::vector<std::string>>& e) {
        LOG_CRT << "YAML bad conversion for vector<string>";
        exit(EXIT_FAILURE);
    }

    const std::string user = _credentials->get_user("service_user");
    const std::string passwd = _credentials->get_user("service_passwd");
    _amqp_url = "amqp://" + user + ":" + passwd + "@" + ip_host;

    try {
        const fs::path header_dir = fs::path(work_dir) / fs::path("header");
        const fs::path fits_dir = fs::path(work_dir) / fs::path("fits");
        _header_path = create_dir(header_dir);
        _fits_path = create_dir(fits_dir);
    }
    catch (L1::CannotCreateDir& e) {
        exit(EXIT_FAILURE);
    }

    // TODO
    const std::string redis_pwd = "meh";

    _actions = {
        { "AT_FWDR_HEALTH_CHECK", std::bind(&miniforwarder::health_check,
                this, std::placeholders::_1) },
        { "AT_FWDR_XFER_PARAMS", std::bind(&miniforwarder::xfer_params,
                this, std::placeholders::_1) },
        { "AT_FWDR_HEADER_READY", std::bind(&miniforwarder::header_ready,
                this, std::placeholders::_1) },
        { "AT_FWDR_END_READOUT", std::bind(&miniforwarder::end_readout,
                this, std::placeholders::_1) },

        { "CC_FWDR_HEALTH_CHECK", std::bind(&miniforwarder::health_check,
                this, std::placeholders::_1) },
        { "CC_FWDR_XFER_PARAMS", std::bind(&miniforwarder::xfer_params,
                this, std::placeholders::_1) },
        { "CC_FWDR_HEADER_READY", std::bind(&miniforwarder::header_ready,
                this, std::placeholders::_1) },
        { "CC_FWDR_END_READOUT", std::bind(&miniforwarder::end_readout,
                this, std::placeholders::_1) },

        { "FILE_TRANSFER_COMPLETED_ACK", std::bind(&miniforwarder::process_ack,
                this, std::placeholders::_1) },
        { "ASSOCIATED", std::bind(&miniforwarder::associated,
                this, std::placeholders::_1) },
    };

    try {
        _pub = std::unique_ptr<SimplePublisher>(new SimplePublisher(_amqp_url));
    }
    catch (L1::PublisherError& e) {
        exit(EXIT_FAILURE);
    }

    _db = std::unique_ptr<Scoreboard>(
            new Scoreboard("localhost", 6379, 0, redis_pwd));
    _sender = std::unique_ptr<FileSender>(new FileSender(xfer_option));
    _pattern = std::unique_ptr<ReadoutPattern>(new ReadoutPattern(pattern));

    _forwarder_list = "forwarder_list";
    _association_key = "f99_association";

    auto bound_register_fwd = std::bind(&miniforwarder::register_fwd, this);
    _hb_params.seconds_to_expire = _seconds_to_expire;
    _hb_params.seconds_to_update = _seconds_to_update;
    _hb_params.key = _association_key;
    _hb_params.redis_host = redis_host;
    _hb_params.redis_port = redis_port;
    _hb_params.redis_db = redis_db;
    _hb_params.action = bound_register_fwd;

    register_fwd();

    _beacon = std::unique_ptr<Beacon>(new Beacon(_hb_params));
    _watcher = std::unique_ptr<Watcher>(new Watcher());

    _store = std::unique_ptr<IMS::Store>(new IMS::Store(_partition.c_str()));
    _stream = std::unique_ptr<IMS::Stream>(new IMS::Stream(*_store, TIMEOUT));
}

miniforwarder::~miniforwarder() {
}

void miniforwarder::on_message(const std::string& message) {
    try {
        LOG_DBG << "Received message " << message;
        const YAML::Node n = YAML::Load(message);
        const std::string message_type = n["MSG_TYPE"].as<std::string>();
        _actions[message_type](n);
    }
    catch(std::exception& e) {
        LOG_CRT << e.what();
    }
}

void miniforwarder::run() {
    try {
        Consumer consumer(_amqp_url, _consume_q);
        auto on_msg = bind(&miniforwarder::on_message, this,
                std::placeholders::_1);
        consumer.run(on_msg);
    }
    catch (L1::ConsumerError& e) { exit(-1); }
    catch (std::exception& e) {
        LOG_CRT << e.what();
        exit(-1);
    }
}

void miniforwarder::health_check(const YAML::Node& n) {
    publish_ack(n);
}

void miniforwarder::xfer_params(const YAML::Node& n) {
    try {
        publish_ack(n);

        auto locations = n["XFER_PARAMS"]["RAFT_CCD_LIST"]
                .as<std::vector<std::string>>();
        // auto locations = n["LOCATIONS"].as<std::vector<std::string>>();
        bool valid = check_valid_board(locations);
        if (!valid) {
            std::ostringstream loc_str;
            for (auto&& loc : locations) {
                loc_str << loc << " ";
            }

            std::ostringstream err;
            err << "Locations provided by xfer_params " << loc_str.str()
                << " is not defined in " << _partition;
            LOG_CRT << err.str();
            return;
        }

        const std::string image_id = n["IMAGE_ID"].as<std::string>();
        xfer_info xfer;
        xfer.target = n["TARGET_LOCATION"].as<std::string>();
        xfer.session_id = n["SESSION_ID"].as<std::string>();
        xfer.job_num = n["JOB_NUM"].as<std::string>();
        xfer.locations = locations;

        _db->add_xfer(image_id, xfer);
    }
    catch (std::exception& e) {
        LOG_CRT << e.what();
    }
}

void miniforwarder::header_ready(const YAML::Node& n) {
    publish_ack(n);

    std::string filename, image_id, reply_q, ack_id;
    try {
        filename = n["FILENAME"].as<std::string>();
        image_id = n["IMAGE_ID"].as<std::string>();
        reply_q = n["REPLY_QUEUE"].as<std::string>();
        ack_id = n["ACK_ID"].as<std::string>();
    }
    catch (YAML::TypedBadConversion<std::string>& e) {
        LOG_CRT << "Cannot read field from YAML Node";
        return;
    }

    try {
        fs::path header = _header_path / fs::path(image_id);
        _hdr.fetch(filename, header);
        _db->add_header(image_id, header.string());

        assemble(image_id);
    }
    catch (L1::CannotFetchHeader& e) {
        int error_code = 5610;
        publish_image_retrieval_for_archiving(error_code, image_id, "",
                e.what());
    }
    catch (std::exception& e) {
        LOG_CRT << e.what();
        int error_code = 5610;
        publish_image_retrieval_for_archiving(error_code, image_id, "",
                e.what());
    }
}

void miniforwarder::end_readout(const YAML::Node& n) {
    publish_ack(n);

    std::string image_id;
    try {
        image_id = n["IMAGE_ID"].as<std::string>();
    }
    catch (YAML::TypedBadConversion<std::string>& e) {
        LOG_CRT << "Cannot read IMAGE_ID from YAML Node";
        return;
    }

    LOG_DBG << "IMS::Image blocking for image " << image_id;
    IMS::Image image(*_store, *_stream, TIMEOUT);
    LOG_DBG << "Acquired image " << image_id;

    IMS::ImageMetadata meta = image.metadata();
    std::string name = std::string(meta.name());
    std::string folder = std::string(meta.folder());

    if (name != image_id || folder != _folder) {
        LOG_CRT << "Currently streaming image " << name << " in " << folder
            << " is not what is being expected " << image_id << " from "
            << _folder;
        return;
    }

    LOG_DBG << "Barrier blocking for image " << image_id;
    IMS::Barrier barrier(image);
    barrier.block();
    LOG_DBG << "Barrier released for image " << image_id;

    try {
        std::vector<std::future<void>> tasks;
        std::vector<std::string> locations = _db->locations(image_id);
        for (auto&& location : locations) {
            // get sensor type
            DAQ::Sensor::Type sensor = _pattern->sensor(location);
            std::vector<int> data_segment = _pattern->data_segment(sensor);
            int xor_pattern = _pattern->get_xor(sensor);

            std::unique_ptr<DAQFetcher> daq = std::unique_ptr<DAQFetcher>(
                    new DAQFetcher(_partition, _folder, data_segment,
                        xor_pattern));
            std::future<void> job = std::async(std::launch::async,
                    &DAQFetcher::fetch,
                    std::move(daq),
                    _fits_path,
                    image_id,
                    location);
            tasks.push_back(std::move(job));
        }

        for (auto&& task : tasks) {
            task.get();
        }

        assemble(image_id);
    }
    catch (L1::RedisError& e) {
        int error_code = 5611;
        publish_image_retrieval_for_archiving(error_code, image_id, "",
                e.what());
    }
    catch (L1::CannotFetchPixel& e) {
        int error_code = 5611;
        publish_image_retrieval_for_archiving(error_code, image_id, "",
                e.what());
    }
    catch (std::exception& e) {
        LOG_CRT << e.what();
        int error_code = 5611;
        publish_image_retrieval_for_archiving(error_code, image_id, "",
                e.what());
    }
}

void miniforwarder::process_ack(const YAML::Node& n) {
    try {
        const std::string msg_type = n["MSG_TYPE"].as<std::string>();
        LOG_INF << "Got ack for message type " << msg_type;
    }
    catch (std::exception& e) {
        LOG_CRT << e.what();
    }
}

void miniforwarder::associated(const YAML::Node& n) {
    try {
        const std::string key = n["ASSOCIATION_KEY"].as<std::string>();
        const std::string reply_q = n["REPLY_QUEUE"].as<std::string>();
        const std::string ack_id = n["ACK_ID"].as<std::string>();
        const std::string msg = _builder.build_associated_ack(_association_key,
                ack_id);

        _pub->publish_message(reply_q, msg);

        heartbeat_params params = _hb_params;
        params.key = key;
        _watcher->start(params);
    }
    catch(std::exception& e) {
        LOG_CRT << e.what();
    }
}

void miniforwarder::publish_ack(const YAML::Node& n) {
    try {
        const std::string msg_type = n["MSG_TYPE"].as<std::string>();
        const std::string ack_id = n["ACK_ID"].as<std::string>();
        const std::string reply_q = n["REPLY_QUEUE"].as<std::string>();
        const std::string msg = _builder.build_ack(msg_type, _name,
                ack_id, "True");
        _pub->publish_message(reply_q, msg);
        LOG_DBG << "Published ack for " << msg_type << " with values: " << msg;
    }
    catch (L1::PublisherError& e) { }
    catch (std::exception& e) {
        LOG_CRT << e.what();
    }
}

void miniforwarder::publish_xfer_complete(const std::string& obsid,
                                          const std::string& to,
                                          const std::string& session_id,
                                          const std::string& job_num) {
    try {
        const std::string msg = _builder.build_xfer_complete(to, obsid,
                session_id, job_num, _consume_q);
        _pub->publish_message(_archive_q, msg);
    }
    catch (L1::PublisherError& e) { }
    catch (std::exception& e) {
        LOG_CRT << e.what();
    }
}

void miniforwarder::publish_image_retrieval_for_archiving(
        const int& error_code,
        const std::string& obsid,
        const std::string& filename,
        const std::string& desc) {
    const std::string msg = _builder.build_image_retrieval_for_archiving(
            error_code,
            obsid,
            filename,
            desc);
    try {
        _pub->publish_message(_telemetry_q, msg);
    }
    catch (L1::PublisherError& e) {}
}

void miniforwarder::assemble(const std::string& image_id) {
    try {
        if (_db->ready(image_id)) {

            xfer_info params = _db->get_xfer(image_id);
            std::string session_id = params.session_id;
            std::string job_num = params.job_num;
            std::string to = params.target;

            std::string header = _db->header(image_id);
            std::vector<std::string> ccds = _db->ccds(image_id);

            format_with_header(ccds, header);
            _sender->send(ccds, fs::path(to));
            publish_completed_msgs(image_id, to, ccds, session_id, job_num);

            LOG_INF << "********* READOUT COMPLETE for " << image_id;

            cleanup(image_id, ccds, header);
        }
    }
    catch (L1::RedisError& e) {
        int error_code = 5612;
        publish_image_retrieval_for_archiving(error_code, image_id, "",
                e.what());
    }
    catch (L1::CannotFormatFitsfile& e) {
        int error_code = 5612;
        publish_image_retrieval_for_archiving(error_code, image_id, "",
                e.what());
    }
    catch (L1::CannotCopyFile& e) {
        int error_code = 5612;
        publish_image_retrieval_for_archiving(error_code, image_id, "",
                e.what());
    }
    catch (std::exception& e) {
        std::ostringstream err;
        err << "Cannot assemble fitsfile because " << e.what();
        LOG_CRT << err.str();
        int error_code = 5612;
        publish_image_retrieval_for_archiving(error_code, image_id, "",
                err.str());
    }
}

void miniforwarder::format_with_header(std::vector<std::string>& ccds,
                                       const std::string header) {
    std::vector<std::future<void>> tasks;
    for (auto&& ccd : ccds) {
        // find -R22S00.fits
        size_t hyphen = ccd.find_last_of("-");
        size_t dot = ccd.find_last_of(".");

        if (hyphen == std::string::npos || dot == std::string::npos) {
            std::ostringstream err;
            err << "Extracting bay board from sensor " << ccd << " is invalid "
                << "because filename does not conform to Image-R00S00.fits";
            LOG_CRT << err.str();
            throw L1::CannotFormatFitsfile(err.str());
        }
        std::string sensor_name = ccd.substr(hyphen+1, dot-hyphen-1);
        std::string bay = sensor_name.substr(1, 2);
        std::string board = sensor_name.substr(4, 1);
        std::string bay_board = bay + "/" + board;

        DAQ::Sensor::Type sensor = ReadoutPattern::sensor(bay_board);

        std::vector<std::string> mapping = _pattern->data_segment_name(sensor);
        auto fmt = std::unique_ptr<YAMLFormatter>(
                new YAMLFormatter(mapping));
        std::future<void> job = std::async(
                std::launch::async,
                &YAMLFormatter::write_header,
                std::move(fmt),
                fs::path(ccd),
                fs::path(header)
            );
        tasks.push_back(std::move(job));
    }

    for (auto&& task : tasks) {
        task.get();
    }
}

void miniforwarder::publish_completed_msgs(const std::string image_id,
                                           const std::string to,
                                           std::vector<std::string>& ccds,
                                           const std::string session_id,
                                           const std::string job_num){
    for (auto&& ccd : ccds) {
        // to: ARC@xxx.xxx.xxx.xxx:/tmp/data
        // to_dir: /tmp/data
        // to_fullpath: /tmp/data/xxx.fits
        std::string filename = ccd.substr(ccd.find_last_of("/")+1);
        std::string to_dir = to.substr(to.find_last_of(":")+1);
        std::string to_fullpath = to_dir + "/" + filename;

        std::ostringstream msg;
        msg << filename << " is successfully transferred to " << to;

        publish_xfer_complete(image_id, to_fullpath, session_id,
                job_num);
        publish_image_retrieval_for_archiving(0, image_id, to_fullpath,
                msg.str());
    }
}

void miniforwarder::cleanup(const std::string image_id,
                            std::vector<std::string>& ccds,
                            const std::string header) {
    _db->remove(image_id);
    for (auto&& ccd : ccds) {
        std::remove(ccd.c_str());
    }
    std::remove(header.c_str());
}

fs::path miniforwarder::create_dir(const fs::path& file_path) {
    boost::system::error_code err;

    // The status is not that useful. status == false doesn't mean directory
    // creation failed but it means boost::filesystem didn't create that
    // directory. In other words, if the directory already exists, status will
    // be false, regardless of who created it.
    bool status = create_directories(file_path, err);

    if (err.value()) {
        std::ostringstream err_msg;
        err_msg << "Cannot create directory for path " << file_path.string()
                << " because " << err.message();
        LOG_CRT << err_msg.str();
        throw L1::CannotCreateDir(err_msg.str());
    }
    return file_path;
}

void miniforwarder::register_fwd() {
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);

    struct addrinfo addr;
    struct addrinfo *infoptr = NULL;
    addr.ai_family = AF_INET;
    // TODO: RAII
    int response = getaddrinfo(hostname, NULL, NULL, &infoptr);
    if (response) {
        LOG_CRT << "Cannot get hostname";
        throw L1::L1Exception("Cannot get hostname");
    }

    struct addrinfo *p;
    char host[256];
    std::string ip_addr;
    for (p = infoptr; p != NULL; p = p->ai_next) {
        getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host), NULL, 0,
                NI_NUMERICHOST);
        ip_addr = host;
        break;
    }
    freeaddrinfo(infoptr);

    // set fowarder in archiver database
    const std::string msg = _builder.build_fwd_info(hostname, ip_addr,
            _consume_q);

    std::string redis_host = _hb_params.redis_host;
    int port = _hb_params.redis_port;
    int db = _hb_params.redis_db;
    std::string passwd = _hb_params.redis_passwd;

    try {
        RedisConnection con(redis_host, port, db);
        con.lpush(_forwarder_list, { msg });
        con.exec();
        LOG_INF << "Set forwarder in redis list";
    }
    catch (L1::RedisError& e) { }
}

bool miniforwarder::check_valid_board(const std::vector<std::string>& locs) {
    for (auto&& loc : locs) {
        auto found = std::find(_daq_locations.begin(), _daq_locations.end(),
                loc);
        if (found == _daq_locations.end()) {
            return false;
        }
    }
    return true;
}
