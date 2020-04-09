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

#include <util/FitsComparator.h>
#include <core/Exceptions.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>
#include <memory>


namespace po = boost::program_options;

int main(int ac, char *av[]) {
    std::vector<std::string> n;

    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("input-file", po::value < std::vector < std::string >> (), "input file (2 required)")
    ("segments,s", "compare by segments")
    ("headers,h", "compare by headers")
    ("hdu,u", "compare by header data units");

    po::positional_options_description p;
    p.add("input-file", 2);

    po::variables_map vm;
    po::store(po::command_line_parser(ac, av).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    if (!vm.count("input-file")) {
        std::cout << "No input files were entered" << std::endl << "try --help option";
        exit(0);
    } else {
        n = vm["input-file"].as < std::vector < std::string >> ();
        if (n.size() != 2) {
            std::cout << n.size() << " input files were provided. 2 fits files must be provided." << std::endl;
            exit(0);
        }
    }

    try {
        FitsComparator fc = FitsComparator(n[0], n[1]);

        try {
            if (vm.count("headers")) {
                if (fc.compare_headers()) {
                    std::cout << std::endl << "All headers are equal" << std::endl;
                } else {
                    std::cout << std::endl << "Not all headers are equal" << std::endl;
                }
            }
        } catch (L1::CfitsioError e) {
            std::cerr << "Error while trying to compare headers: " << e.what() << std::endl;
        }
        try {
            if (vm.count("segments")) {
                if (fc.compare_by_segments()) {
                    std::cout << "Segments are equal" << std::endl;
                } else {
                    std::cout << "Segments are not equal" << std::endl;
                }
            }
        } catch (L1::CfitsioError e) {
            std::cerr << "Error while trying to compare by segments: " << e.what() << std::endl;
        }
        try {
            if (vm.count("hdu")) {
                if (fc.compare_by_hdu()) {
                    std::cout << "HDU's are equal" << std::endl;
                } else {
                    std::cout << "HDU's are not equal" << std::endl;
                }
            }
        } catch (L1::CfitsioError e) {
            std::cerr << "Error while trying to compare by hdu: " << e.what() << std::endl;
        }
    } catch (L1::CfitsioError e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}