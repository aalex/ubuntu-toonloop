/*
 * Toonloop
 *
 * Copyright 2010 Alexandre Quessy
 * <alexandre@quessy.net>
 * http://www.toonloop.com
 *
 * Toonloop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Toonloop is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the gnu general public license
 * along with Toonloop.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <cstdio>
#include <iostream>
#include <tr1/memory>

#include "lo/lo.h"
#include "oscinterface.h"
#include "application.h"

OscInterface::OscInterface(
        const std::string &listen_port)//,
        //const std::string &send_host,
        //const std::string &send_port) 
    :
    receiver_(listen_port) //,
    //sender_(serverHost, serverListenPort),
    //tryToSubscribe_(true)
{
    std::cout << "Listening osc_udp://localhost:" << listen_port << std::endl;
    receiver_.addHandler("/ping", "", pingCb, this);
    receiver_.addHandler("/pong", "", pongCb, this);
    receiver_.addHandler("/toon/quit", "", quitCb, this);
    receiver_.addHandler("/toon/frame/add", "", addFrameCb, this);
    receiver_.addHandler("/toon/frame/remove", "", removeFrameCb, this);
    std::cout << "OSC message handlers:" << std::endl;
    std::cout << " * /ping : Answers with /pong" << std::endl;
    std::cout << " * /pong" << std::endl;
    std::cout << " * /toon/quit : Quits" << std::endl;
    std::cout << " * /toon/frame/add : Grabs a frame" << std::endl;
    std::cout << " * /toon/frame/remove : Removes a frame" << std::endl;
}

int OscInterface::pingCb(
        const char *path, 
        const char *types, lo_arg **argv, 
        int argc, void *data, void *user_data) 
{ 
    //OscInterface *context = static_cast<OscInterface*>(user_data);
#ifdef CONFIG_DEBUG
    std::cout << "Got " << path << std::endl << std::endl;
#endif
    std::cout << "Got ping" << std::endl;
    return 0;
} 

int OscInterface::pongCb(
        const char *path, 
        const char *types, lo_arg **argv, 
        int argc, void *data, void *user_data)
{
    std::cout << "Got /pong" << std::endl;
    return 0;
}
int OscInterface::addFrameCb(
        const char *path, 
        const char *types, lo_arg **argv, 
        int argc, void *data, void *user_data)
{
    std::cout << "Got /toon/frame/add" << std::endl;
    Application::get_instance().get_pipeline().grab_frame();
    return 0;
}
int OscInterface::removeFrameCb(
        const char *path, 
        const char *types, lo_arg **argv, 
        int argc, void *data, void *user_data)
{
    std::cout << "Got /toon/frame/remove" << std::endl;
    Application::get_instance().get_pipeline().remove_frame();
    return 0;
}
/**
 * Handles /toon/quit
 *
 * Quits the application.
 */
// TODO:2010-08-15:aalex:Should be able to disable this handler
int OscInterface::quitCb(
        const char *path, 
        const char *types, lo_arg **argv, 
        int argc, void *data, void *user_data)
{
    std::cout << "Got /toon/quit" << std::endl;
    //TODO:2010-08-15:aalex:Get rid of the application singleton
    //SamplerServer *context = static_cast<SamplerServer*>(user_data);
    //context->owner_->quit();
    Application::get_instance().quit();
    return 0;
}

OscInterface::~OscInterface()
{
}

void OscInterface::start()
{
    // start a thread to try and subscribe us
    //boost::thread trySubscribe(boost::bind<void>(&StateClient::subscribe, this));
    receiver_.listen(); // start listening in separate thread
}

