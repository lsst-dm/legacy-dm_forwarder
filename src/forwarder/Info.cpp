#include <forwarder/Info.h>

Info::MODE Info::encode(std::string mode) {
    if (mode == "live") return Info::MODE::LIVE;
    else if (mode == "catchup") return Info::MODE::CATCHUP;
    else return Info::MODE::UNDEFINED;
}
