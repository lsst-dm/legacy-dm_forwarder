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

#include <yaml-cpp/yaml.h>
#include <forwarder/MessageBuilder.h>

std::string MessageBuilder::build_ack(const std::string& msg_type,
                                      const std::string& image_id,
                                      const std::string& component,
                                      const std::string& ack_id,
                                      const std::string& ack_bool) {
    YAML::Emitter msg;
    msg << YAML::DoubleQuoted;
    msg << YAML::Flow;
    msg << YAML::BeginMap;
    msg << YAML::Key << "MSG_TYPE" << YAML::Value << msg_type + "_ACK";
    msg << YAML::Key << "IMAGE_ID" << YAML::Value << image_id;
    msg << YAML::Key << "COMPONENT" << YAML::Value << component;
    msg << YAML::Key << "ACK_ID" << YAML::Value << ack_id;
    msg << YAML::EndMap;
    return msg.c_str();
}

std::string MessageBuilder::build_scan_ack() {
    YAML::Emitter msg;
    msg << YAML::DoubleQuoted;
    msg << YAML::Flow;
    msg << YAML::BeginMap;
    msg << YAML::Key << "MSG_TYPE" << YAML::Value << "SCAN_ACK";
    msg << YAML::EndMap;
    return msg.c_str();
}

std::string MessageBuilder::build_xfer_complete(const std::string& filename,
                                                const std::string& obsid,
                                                const std::string& raft,
                                                const std::string& ccd,
                                                const std::string& session_id,
                                                const std::string& job_num,
                                                const std::string& reply_q) {
    YAML::Emitter msg;
    msg << YAML::DoubleQuoted;
    msg << YAML::Flow;
    msg << YAML::BeginMap;
    msg << YAML::Key << "MSG_TYPE" << YAML::Value << "FILE_TRANSFER_COMPLETED";
    msg << YAML::Key << "FILENAME" << YAML::Value << filename;
    msg << YAML::Key << "OBSID" << YAML::Value << obsid;
    msg << YAML::Key << "RAFT" << YAML::Value << raft;
    msg << YAML::Key << "SENSOR" << YAML::Value << ccd;
    msg << YAML::Key << "SESSION_ID" << YAML::Value << session_id;
    msg << YAML::Key << "JOB_NUM" << YAML::Value << job_num;
    msg << YAML::Key << "REPLY_QUEUE" << YAML::Value << reply_q;
    msg << YAML::EndMap;
    return msg.c_str();
}

std::string MessageBuilder::build_associated_ack(const std::string& key,
                                                 const std::string& ack_id) {
    YAML::Emitter msg;
    msg << YAML::DoubleQuoted;
    msg << YAML::Flow;
    msg << YAML::BeginMap;
    msg << YAML::Key << "MSG_TYPE" << YAML::Value << "ASSOCIATED_ACK";
    msg << YAML::Key << "ASSOCIATION_KEY" << YAML::Value << key;
    msg << YAML::Key << "ACK_ID" << YAML::Value << ack_id;
    msg << YAML::EndMap;
    return msg.c_str();
}

std::string MessageBuilder::build_fwd_info(const std::string& hostname,
                                           const std::string& ip_addr,
                                           const std::string& consume_q) {
    YAML::Emitter msg;
    msg << YAML::DoubleQuoted;
    msg << YAML::Flow;
    msg << YAML::BeginMap;
    msg << YAML::Key << "hostname" << YAML::Value << hostname;
    msg << YAML::Key << "ip_address" << YAML::Value << ip_addr;
    msg << YAML::Key << "consume_queue" << YAML::Value << consume_q;
    msg << YAML::EndMap;
    return msg.c_str();
}

std::string MessageBuilder::build_image_retrieval_for_archiving(
        const int& code,
        const std::string& obsid,
        const std::string& raft,
        const std::string& ccd,
        const std::string& filename,
        const std::string& desc) {
    YAML::Emitter msg;
    msg << YAML::DoubleQuoted;
    msg << YAML::Flow;
    msg << YAML::BeginMap;
    msg << YAML::Key << "MSG_TYPE"
        << YAML::Value << "IMAGE_RETRIEVAL_FOR_ARCHIVING";
    msg << YAML::Key << "OBSID" << YAML::Value << obsid;
    msg << YAML::Key << "RAFT" << YAML::Value << raft;
    msg << YAML::Key << "SENSOR" << YAML::Value << ccd;
    msg << YAML::Key << "FILENAME" << YAML::Value << filename;
    msg << YAML::Key << "STATUS_CODE" << YAML::Value << code;
    msg << YAML::Key << "DESCRIPTION" << YAML::Value << desc;
    msg << YAML::EndMap;
    return msg.c_str();
}
