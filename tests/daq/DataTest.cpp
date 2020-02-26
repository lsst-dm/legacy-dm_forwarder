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

#include <memory>
#include <boost/test/unit_test.hpp>
#include <ims/Stripe.hh>
#include <ims/SourceMetadata.hh>
#include <rms/InstructionList.hh>
#include <rms/Instruction.hh>
#include <core/IIPBase.h>
#include <daq/Data.h>

struct DataFixture : IIPBase {

    static const int _sensors = 3;
    int _samples = 5;

    IMS::Stripe* _arr[_sensors];
    std::unique_ptr<Data> _d;
    IMS::SourceMetadata _meta;

    DataFixture() : IIPBase("ForwarderCfg.yaml", "test") {

        IMS::Stripe s;
        int32_t stripe_d[16] {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        };
        std::copy(std::begin(stripe_d), std::end(stripe_d),
            std::begin(s.segment));

        for (int i = 0; i < _sensors; i++) {
            _arr[i] = new IMS::Stripe[_samples];
            for (int j = 0; j < _samples; j++) {
                _arr[i][j] = s;
            }
        }

        RMS::InstructionList list = _meta.instructions();
        list.insert(RMS::Instruction::Opcode::PUT, 0, 1);
        list.insert(RMS::Instruction::Opcode::PUT, 1, 2);
        list.insert(RMS::Instruction::Opcode::PUT, 2, 3);
        list.insert(RMS::Instruction::Opcode::PUT, 3, 4);
        list.insert(RMS::Instruction::Opcode::PUT, 4, 5);
        list.insert(RMS::Instruction::Opcode::PUT, 5, 6);
        list.insert(RMS::Instruction::Opcode::PUT, 6, 7);
        list.insert(RMS::Instruction::Opcode::PUT, 7, 8);
        list.insert(RMS::Instruction::Opcode::PUT, 8, 9);
        list.insert(RMS::Instruction::Opcode::PUT, 9, 10);
        _meta = list;

        _d = std::unique_ptr<Data>(new Data(_sensors, _samples, _arr, _meta));
    }

    ~DataFixture() {
    }
};

BOOST_FIXTURE_TEST_SUITE(DataTest, DataFixture);

BOOST_AUTO_TEST_CASE(constructor) {

    // valid constructor shouldn't throw exception
    // Edge case in the constructor is there is no bound checking for Stripe
    // array. if sensor/samples are less or greater than Stripe[sensor][samples]
    // , the program will crush. If those values are messed up, it means the
    // error originates from DAQ and not from user-defined code Data.
    BOOST_CHECK_NO_THROW(Data(_sensors, _samples, _arr, _meta));
}

BOOST_AUTO_TEST_CASE(pixel) {

    // Valid pixels
    // Putting invalid indices in this function is undefined behavior.
    BOOST_CHECK_EQUAL(_d->pixel(_samples-1, _sensors-1, 15), 15);
}

BOOST_AUTO_TEST_CASE(naxes) {

    // Valid naxes calculations
    long naxis1 = 1 + 3 + 5 + 6;
    long naxis2 = 8 + 10;
    std::vector<long> axes{ naxis1, naxis2 };
    std::vector<long> o = _d->naxes();
    BOOST_CHECK_EQUAL_COLLECTIONS(axes.begin(), axes.end(),
            o.begin(), o.end());
}

BOOST_AUTO_TEST_CASE(samples) {

    // Valid samples
    BOOST_CHECK_EQUAL(_d->samples(), _samples);
}

BOOST_AUTO_TEST_SUITE_END()
