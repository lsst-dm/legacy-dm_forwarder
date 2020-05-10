#include <string>

class Info { 
    public:
        enum class MODE { 
            LIVE,
            CATCHUP,
            UNDEFINED
        };

        static MODE encode(std::string);
};
