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

#include <daq/Location.hh>
#include <daq/ScienceSet.hh>
#include <daq/WavefrontSet.hh>
#include <daq/GuidingSet.hh>
#include <core/Exceptions.h>
#include <core/SimpleLogger.h>
#include <forwarder/ReadoutPattern.h>

typedef DAQ::Sensor::Type SensorType;

std::vector<std::string> DATA_SEGMENT_NAME{
    "00", "01", "02", "03", "04", "05", "06", "07",
    "10", "11", "12", "13", "14", "15", "16", "17",
};

std::vector<int> DATA_SEGMENT {
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 13, 14, 15
};

ReadoutPattern::ReadoutPattern(const YAML::Node& n) : _root{n} {
}

DAQ::Sensor::Type ReadoutPattern::sensor(const std::string& location) {
    DAQ::Location loc(location.c_str());

    DAQ::ScienceSet s;
    DAQ::GuidingSet g;
    DAQ::WavefrontSet w;

    if (!loc) {
        std::ostringstream err;
        err << "Invalid Location " << location;
        LOG_CRT << err.str();
        throw L1::InvalidLocation(err.str());
    }

    if (s.has(loc)) return DAQ::Sensor::Type::SCIENCE;
    else if (g.has(loc)) return DAQ::Sensor::Type::GUIDE;
    else if (w.has(loc)) return DAQ::Sensor::Type::WAVEFRONT;
    else {
        std::ostringstream err;
        err << "DAQ Location is undefined sensor type.";
        LOG_CRT << err.str();
        throw L1::InvalidLocation(err.str());
    }
}

std::vector<std::string> ReadoutPattern::data_segment_name(SensorType& sensor) {
    std::string sensor_name = DAQ::Sensor::encode(sensor);
    YAML::Node node = _root["DATA_SEGMENT_NAME"][sensor_name];
    if (!node) {
        std::ostringstream err;
        err << "Config file does not contain DATA_SEGMENT_NAME::"
            << sensor_name;
        LOG_CRT << err.str();
        throw L1::InvalidReadoutPattern(err.str());
    }

    std::vector<std::string> names;
    try {
        names = node.as<std::vector<std::string>>();
    }
    catch (YAML::TypedBadConversion<std::vector<std::string>>& e) {
        std::ostringstream err;
        err << "DATA_SEGMENT_NAME:: " << sensor_name
            << " is invalid data type. Expecting strings of 00-07, 10-17";
        LOG_CRT << err.str();
        throw L1::InvalidReadoutPattern(err.str());
    }

    // check if DATA_SEGMENT_NAME given is valid pattern
    std::vector<std::string> other = names;
    std::sort(other.begin(), other.end());
    bool eq = std::equal(other.begin(), other.end(),
            DATA_SEGMENT_NAME.begin());

    if (other.size() != DATA_SEGMENT_NAME.size() || !eq) {
        std::ostringstream err;
        err << "DATA_SEGMENT_NAME::" << sensor_name << " is not valid pattern."
            << "Sample is 00-07, 10-17";
        LOG_CRT << err.str();
        throw L1::InvalidReadoutPattern(err.str());
    }

    return names;
}

std::vector<int> ReadoutPattern::data_segment(SensorType& sensor) {
    std::string sensor_name = DAQ::Sensor::encode(sensor);
    YAML::Node node = _root["DATA_SEGMENT"][sensor_name];
    if (!node) {
        std::ostringstream err;
        err << "Config file does not contain DATA_SEGMENT::"
            << sensor_name;
        LOG_CRT << err.str();
        throw L1::InvalidReadoutPattern(err.str());
    }

    std::vector<int> data;
    try {
        data = node.as<std::vector<int>>();
    }
    catch (YAML::TypedBadConversion<std::vector<int>>& e) {
        std::ostringstream err;
        err << "DATA_SEGMENT:: " << sensor_name
            << " is invalid data type. Expecting int from 0-15";
        LOG_CRT << err.str();
        throw L1::InvalidReadoutPattern(err.str());
    }

    // check if DATA_SEGMENT is valid pattern
    std::vector<int> other = data;
    std::sort(other.begin(), other.end());
    bool eq = std::equal(other.begin(), other.end(),
            DATA_SEGMENT.begin());

    if (other.size() != DATA_SEGMENT.size() || !eq) {
        std::ostringstream err;
        err << "DATA_SEGMENT::" << sensor_name << " is not valid pattern."
            << "Sample is 0-15";
        LOG_CRT << err.str();
        throw L1::InvalidReadoutPattern(err.str());
    }

    return data;
}

std::vector<int> ReadoutPattern::sensor_order(SensorType& sensor) {
    std::string sensor_name = DAQ::Sensor::encode(sensor);
    try {
        return _root["DATA_SENSOR"][sensor_name].as<std::vector<int>>();
    }
    catch (YAML::TypedBadConversion<std::vector<int>>& e) {
        std::ostringstream err;
        err << "Cannot read either DATA_SENSOR from pattern or "
            << " Sensor " << sensor_name << " is not valid";
        LOG_CRT << err.str();
        throw L1::InvalidReadoutPattern(err.str());
    }
}

int ReadoutPattern::get_xor(SensorType& sensor) {
    std::string sensor_name = DAQ::Sensor::encode(sensor);

    YAML::Node node;
    try {
        node = _root["XOR"][sensor_name];
    }
    catch (YAML::BadSubscript& e) {
        std::ostringstream err;
        err << "Trying to access XOR::" << sensor_name << " but node does not "
            << "exist.";
        LOG_CRT << err.str();
        throw L1::InvalidReadoutPattern(err.str());
    }

    if (!node) {
        std::ostringstream err;
        err << "Config file does not contain XOR::"
            << sensor_name;
        LOG_CRT << err.str();
        throw L1::InvalidReadoutPattern(err.str());
    }

    int xor_pttn;
    try {
        xor_pttn = node.as<int>();
    }
    catch (YAML::TypedBadConversion<int>& e) {
        std::ostringstream err;
        err << "XOR:: " << sensor_name
            << " is invalid data type. Expecting int";
        LOG_CRT << err.str();
        throw L1::InvalidReadoutPattern(err.str());
    }

    return xor_pttn;
}
