#ifndef _OSC_RECEIVER_H_
#define _OSC_RECEIVER_H_

#include "lo/lo.h"
#include <string>

class OscReceiver {
    public:
        OscReceiver(const std::string &port);
        ~OscReceiver();
        void listen();
        const char * port() const { return port_.c_str(); }
        void addHandler(const char *path, 
                const char *types, lo_method_handler handler, 
                void *user_data);
    private:
        std::string toString() const;
        std::string port_;
        lo_server_thread server_;
        static int genericHandler(const char *path, 
                const char *types, lo_arg **argv, 
                int argc, void *data, void *user_data);

        static void error(int num, const char *msg, const char *path);
};

#endif // _OSC_RECEIVER_H_

