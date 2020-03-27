//
// Created by Samuel Cappon on 3/9/20.
//

#include <util/FitsComparator.h>
#include <core/Exceptions.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>

namespace po = boost::program_options;

main(int ac, char *av[]) {
    vector<string> n;

    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("input-file", po::value < vector < string >> (), "input file (2 required)")
    ("segments,s", "compare by segments")
    ("headers,h", "compare by headers")
    ("hdu,u", "compare by header data units");

    po::positional_options_description p;
    p.add("input-file", 2);

    po::variables_map vm;
    po::store(po::command_line_parser(ac, av).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }

    if (!vm.count("input-file")) {
        cout << "No input files were entered\n\n try --help option";
        exit(0);
    } else {
        n = vm["input-file"].as < vector < string >> ();
        if (n.size() != 2) {
            cout << n.size() << " input files were provided. 2 fits files must be provided.\n";
            exit(0);
        }
    }

    try {
        FitsComparator fc = FitsComparator(n[0], n[1]);

        try {
            if (vm.count("headers")) {
                if (fc.compare_headers()) {
                    cout << "\nAll headers are equal\n";
                } else {
                    cout << "\nNot all headers are equal\n";
                }
            }
        } catch (L1::CfitsioError e) {
            cerr << "Error while trying to compare headers: " << e.what() << "\n";
        }
        try {
            if (vm.count("segments")) {
                if (fc.compare_by_segments()) {
                    cout << "Segments are equal\n";
                } else {
                    cout << "Segments are not equal\n";
                }
            }
        } catch (L1::CfitsioError e) {
            cerr << "Error while trying to compare by segments: " << e.what() << "\n";
        }
        try {
            if (vm.count("hdu")) {
                if (fc.compare_by_hdu()) {
                    cout << "HDU's are equal\n";
                } else {
                    cout << "HDU's are not equal\n";
                }
            }
        } catch (L1::CfitsioError e) {
            cerr << "Error while trying to compare by hdu: " << e.what() << "\n";
        }
    } catch (L1::CfitsioError e) {
        cerr << "Error: " << e.what() << "\n";
    }
}
