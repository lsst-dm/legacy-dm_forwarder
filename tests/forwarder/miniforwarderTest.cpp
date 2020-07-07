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

#include <stdlib.h> // setenv
#include <string.h>
#include <thread>
#include <chrono>
#include <memory>
#include <vector>
#include <functional>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <yaml-cpp/yaml.h>
#include <core/IIPBase.h>
#include <core/Exceptions.h>
#include <core/Consumer.h>
#include <forwarder/miniforwarder.h>

namespace fs = boost::filesystem;

struct miniforwarderFixture : IIPBase {

    std::unique_ptr<miniforwarder> _fwd;
    YAML::Node _d;
    std::string _log_dir, _amqp_url, _telemetry_q;

    miniforwarderFixture() : IIPBase("ForwarderCfg.yaml", "test"){
        BOOST_TEST_MESSAGE("Setup miniforwarder fixture");

        std::string user = _credentials->get_user("service_user");
        std::string passwd = _credentials->get_user("service_passwd");
        std::string ip_host = _config_root["BASE_BROKER_ADDR"]
                .as<std::string>();

        _amqp_url = "amqp://" + user + ":" + passwd + "@" + ip_host;
        _log_dir = _config_root["LOGGING_DIR"].as<std::string>();
        _telemetry_q = _config_root["TELEMETRY_QUEUE"].as<std::string>();

        _fwd = std::unique_ptr<miniforwarder>(
                new miniforwarder("ForwarderCfg.yaml", "test"));
    }

    void on_message(const std::string& message) {

    }

    void run_consumer(const std::string queue) {
        Consumer c(_amqp_url, queue);
        auto on_msg = std::bind(&miniforwarderFixture::on_message, this,
                std::placeholders::_1);
        c.run(on_msg);
    }

    YAML::Node build_xfer_params(std::string& image_id,
                                 std::string& raft,
                                 std::vector<std::string>& ccds) {
        YAML::Node sub;
        sub["RAFT_LIST"] = raft;
        sub["RAFT_CCD_LIST"] = ccds;
        sub["AT_FWDR"] = "f99";

        YAML::Node d;
        d["MSG_TYPE"] = "AT_FWDR_XFER_PARAMS";
        d["SESSION_ID"] = _d["SESSION_ID"];
        d["IMAGE_ID"] = image_id;
        d["DEVICE"] = "AT";
        d["JOB_NUM"] = _d["JOB_NUM"];
        d["ACK_ID"] = 0;
        d["REPLY_QUEUE"] = "at_foreman_ack_publish";
        d["TARGET_LOCATION"] = _d["TARGET_LOCATION"];
        d["XFER_PARAMS"] = sub;
        return d;
    }

    YAML::Node build_end_readout(std::string& image_id) {
        YAML::Node d;
        d["MSG_TYPE"] = "AT_FWDR_END_READOUT";
        d["JOB_NUM"] = _d["JOB_NUM"];
        d["SESSION_ID"] = _d["SESSION_ID"];
        d["IMAGE_ID"] = image_id;
        d["ACK_ID"] = 0;
        d["REPLY_QUEUE"] = "at_foreman_ack_publish";
        d["IMAGE_SEQUENCE_NAME"] = "seq_1";
        d["IMAGES_IN_SEQUENCE"] = 100;
        return d;
    }

    ~miniforwarderFixture() {
        BOOST_TEST_MESSAGE("TearDown miniforwarder fixture");
        std::string log = _log_dir + "/test.log.0";
        std::remove(log.c_str());
    }
};

BOOST_FIXTURE_TEST_SUITE(miniforwarderTest, miniforwarderFixture);

BOOST_AUTO_TEST_CASE(on_message) {

}

BOOST_AUTO_TEST_CASE(set_name) {
    std::array<char, 128> buffer, buffer2;
    std::string result, result2;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("hostname", "r"), pclose),
    pipe2(popen("hostname --ip-address | awk '{print $1}'", "r"), pclose);

    fgets(buffer.data(), buffer.size(), pipe.get());
    int len = strlen(buffer.data());
    buffer[len-1] = '\0';
    result += buffer.data();

    fgets(buffer2.data(), buffer2.size(), pipe2.get());

    len = strlen(buffer2.data());
    buffer2[len-1] = '\0';
    result2 += buffer2.data();

    std::string name = result + ":" + result2;
    BOOST_CHECK_EQUAL(name, _fwd->get_name());
}

BOOST_AUTO_TEST_CASE(run) {

}

BOOST_AUTO_TEST_CASE(health_check) {

}

BOOST_AUTO_TEST_CASE(xfer_params) {

}

BOOST_AUTO_TEST_CASE(header_ready) {

}

BOOST_AUTO_TEST_CASE(end_readout) {
    std::string image_id = _d["IMAGE_ID"].as<std::string>();
    std::string raft = _d["RAFT_LIST"].as<std::string>();
    std::vector<std::string> ccds = _d["RAFT_CCD_LIST"].as<std::vector<std::string>>();
    std::string work_dir = _config_root["WORK_DIR"].as<std::string>();
    std::string fitspath = work_dir + "/fits";

    /**
     * end readout without start integration
     */
    YAML::Node er_without_xfer = build_end_readout(image_id);
    BOOST_CHECK_NO_THROW(_fwd->end_readout(er_without_xfer));

    /**
     * non-existing image_id
     */
    std::string bad_img = "aaa";
    YAML::Node xfer_bad_img = build_xfer_params(bad_img, raft, ccds);
    _fwd->xfer_params(xfer_bad_img);

    YAML::Node er_bad_img = build_end_readout(bad_img);
    BOOST_CHECK_NO_THROW(_fwd->end_readout(er_bad_img));

    /**
     * bad raft
     */
    std::string bad_raft = "1111";
    YAML::Node xfer_bad_raft = build_xfer_params(image_id, bad_raft, ccds);
    _fwd->xfer_params(xfer_bad_raft);

    YAML::Node er_bad_raft = build_end_readout(image_id);
    BOOST_CHECK_NO_THROW(_fwd->end_readout(er_bad_raft));

    /**
     * bad ccd
     * This must be a question for SLAC. In a board, there are 3 ccds and 9
     * should throw an exception of some kind but maybe this is because NCSA's
     * DAQ is a simulator.
     */
    std::vector<std::string> bad_ccds{ "09" };
    YAML::Node xfer_bad_ccds = build_xfer_params(image_id, raft, bad_ccds);
    _fwd->xfer_params(xfer_bad_ccds);

    YAML::Node er_bad_ccd = build_end_readout(image_id);
    BOOST_CHECK_NO_THROW(_fwd->end_readout(er_bad_ccd));

    /*
     * valid image
     */
    YAML::Node d = build_xfer_params(image_id, raft, ccds);
    _fwd->xfer_params(d);

    YAML::Node n = build_end_readout(image_id);
    _fwd->end_readout(n);

    for (auto& ccd : ccds) {
        std::string filename = image_id + "--R" + raft + "S" + ccd + ".fits";
        fs::path filepath = fs::path(fitspath) / fs::path(filename);

        // sleep for file creation
        std::this_thread::sleep_for(std::chrono::seconds(2));

        BOOST_CHECK_EQUAL(fs::exists(filepath), true);
        std::remove(filepath.string().c_str());
    }
}


BOOST_AUTO_TEST_CASE(process_ack) {

}

BOOST_AUTO_TEST_CASE(associated) {

}

BOOST_AUTO_TEST_CASE(assemble) {

}

BOOST_AUTO_TEST_CASE(format_with_header) {

}

BOOST_AUTO_TEST_CASE(publish_completed_msgs) {

}

BOOST_AUTO_TEST_CASE(cleanup) {

}

BOOST_AUTO_TEST_CASE(publish_ack) {

}

BOOST_AUTO_TEST_CASE(publish_xfer_complete) {

}

BOOST_AUTO_TEST_CASE(publish_image_retrieval_for_archiving) {
    BOOST_CHECK_NO_THROW(_fwd->publish_image_retrieval_for_archiving(
                0,
                "AT_0_20191205_00001",
                "22",
                "01",
                "/data/AT_0_20191205_000001.fits",
                "hello world")
    );
    BOOST_CHECK_NO_THROW(_fwd->publish_image_retrieval_for_archiving(
                5610,
                "AT_0_20191205_00001",
                "22",
                "22",
                "",
                "hello world")
    );
}

BOOST_AUTO_TEST_CASE(create_dir) {
    BOOST_CHECK_NO_THROW(_fwd->create_dir(fs::path("/tmp/fwd_test")));
    fs::remove(fs::path("/tmp/fwd_test"));

    BOOST_CHECK_THROW(_fwd->create_dir(fs::path("/opt/fwd_test")),
            L1::CannotCreateDir);
}

BOOST_AUTO_TEST_CASE(check_valid_board) {
    // valid
    std::vector<std::string> good { "22/1", "22/2", "22/0" };
    BOOST_CHECK_EQUAL(_fwd->check_valid_board(good), true);

    // bad
    std::vector<std::string> bad { "00/0" };
    BOOST_CHECK_EQUAL(_fwd->check_valid_board(bad), false);

    // bad2
    std::vector<std::string> bad2 { "22/0", "99" };
    BOOST_CHECK_EQUAL(_fwd->check_valid_board(bad2), false);
}

BOOST_AUTO_TEST_CASE(register_fwd) {

}

BOOST_AUTO_TEST_SUITE_END()
