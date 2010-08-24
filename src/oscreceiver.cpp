#include "./oscreceiver.h"
#include <iostream>
#include <cstdio>

OscReceiver::OscReceiver(const std::string &port) :
    port_(port), 
    server_(lo_server_thread_new(port_.c_str(), error))
{
    /* add method that will match any path and args */
    lo_server_thread_add_method(server_, NULL, NULL, genericHandler, this);
}


OscReceiver::~OscReceiver()
{
    std::cout << "Freeing server thread\n";
    lo_server_thread_free(server_);
}


void OscReceiver::addHandler(const char *path, const char *types, lo_method_handler handler, void *userData)
{
    lo_server_thread_add_method(server_, path, types, handler, userData);
}

void OscReceiver::listen()
{
    std::cout << "Listening on port " << port_ << std::endl;
    lo_server_thread_start(server_);

}


void OscReceiver::error(int num, const char *msg, const char *path)
{
    std::cerr << "liblo server error " << num << " in path " << path 
        << ": " << msg << std::endl;
}

/* catch any incoming messages and display them. returning 1 means that the 
 *  * message has not been fully handled and the server should try other methods */
int OscReceiver::genericHandler(const char *path, 
        const char *types, lo_arg **argv, 
        int argc, void * /*data*/, void * /*user_data*/) 
{ 
    //OscReceiver *context = static_cast<OscReceiver*>(user_data);

#ifdef CONFIG_DEBUG
    printf("path: <%s>\n", path); 
    for (int i = 0; i < argc; ++i) 
    { 
        printf("arg %d '%c' ", i, types[i]); 
        lo_arg_pp(static_cast<lo_type>(types[i]), argv[i]); 
        printf("\n"); 
    } 
    printf("\n"); 
    fflush(stdout); 
#endif // CONFIG_DEBUG

    return 1; 
} 


std::string OscReceiver::toString() const
{
    return "port:" + port_;
}

