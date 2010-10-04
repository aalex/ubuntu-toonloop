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
#include <iostream>
#include <cstdlib>
#include <stk/RtMidi.h>
#include "application.h"
#include "configuration.h"
#include "controller.h"
#include "midi.h"
#include "message.h"

/**
 * Callback for incoming MIDI messages.  Called in its thread.
 * 
 * - Pressing the sustain pedal down grabs a frame.
 *   MIDI controller 64 is the sustain pedal controller. It looks like this:
 *   <channel and status> <controller> <value>
 *   Where the controller number is 64 and the value is either 0 or 127.
 *
 * - The MIDI controller 80 is also a pedal on the Roland GFC-50.
 *   It controls video grabbing. (on / off)
 *
 * - The program change should allow the user to choose another instrument. 
 *   This way, the Roland GFC-50 allows to select any of ten clips.
 *   The MIDI spec allows for 128 programs, numbered 0-127.
 *
 * - Main volume is control 7. It controls the playback speed.
 *   Volume is from 0 to 127.
 */
void MidiInput::input_message_cb(double delta_time, std::vector< unsigned char > *message, void *user_data )
{
    MidiInput* context = static_cast<MidiInput*>(user_data);
    unsigned int nBytes = message->size();
    if (context->verbose_) 
    {
        std::cout << "(";
        for (unsigned int i = 0; i < nBytes; i++)
            std::cout << (int)message->at(i) << " ";
        std::cout << ")";
        if (nBytes > 0)
            std::cout << " stamp = " << delta_time << std::endl;
    }
    if (message->size() >= 3)
    {
        if ((int)message->at(1) == 64) // 64: sustain pedal
        {
            if ((int)message->at(2) != 0) 
                context->push_message(Message(Message::ADD_IMAGE));
                //context->on_pedal_down();
        }
        else if ((int)message->at(1) == 80) // 80: general-purpose control. The Roland GFC-50 provides an on/off pedal input with that number
        {
            if ((int)message->at(2) == 127) 
                context->push_message(Message(Message::VIDEO_RECORD_ON));
                //context->on_ctrl_80_changed(true);
            else
                context->push_message(Message(Message::VIDEO_RECORD_OFF));
                //context->on_ctrl_80_changed(false);
        } 
        //else if ((int)message->at(1) == 7) // 7: volume expression pedal
        //    context->on_volume_control((int)message->at(2));
    }
    else if (message->size() >= 2)
    {
        if (((int)message->at(0) & 192) == 192)
        {
            // Program change control (chooses a clip)
            unsigned int clip_number = (unsigned int)message->at(1);
            //context->on_program_change();
            if (clip_number >= 10)
                std::cout << "Cannot choose a clip greater or equal to 10." << std::endl; 
            else
                context->push_message(Message(Message::SELECT_CLIP, clip_number));
        }
    }
}

void MidiInput::enumerate_devices() const
{
    // List inputs.
    std::cout << std::endl << "MIDI input devices: " << ports_count_ << " found." << std::endl;
    std::string portName;
    for (unsigned int i=0; i < ports_count_; i++) 
    {
        try {
            portName = midi_in_->getPortName(i);
        }
        catch (RtError &name_error) {
            name_error.printMessage();
        }
        std::cout << " * " << i << " : " << portName << '\n';
    }
}

bool MidiInput::is_open() const
{
    return opened_;
}

void MidiInput::push_message(Message message)
{
    // TODO: pass this message argument by reference?
    messaging_queue_.push(message);
}

/**
 * Takes action!
 *
 * Should be called when it's time to take action, before rendering a frame, for example.
 */
void MidiInput::consume_messages()
{
    // TODO:2010-10-03:aalex:Move this handling to Application.
    bool success = true;
    while (success)
    {
        Message message;
        success = messaging_queue_.try_pop(message);
        if (success)
        {
            unsigned int value = message.get_value();
            switch (message.get_command())
            {
                case Message::ADD_IMAGE:
                    owner_->get_controller()->add_frame();
                    break;
                case Message::REMOVE_IMAGE:
                    owner_->get_controller()->add_frame();
                    break;
                case Message::VIDEO_RECORD_ON:
                    owner_->get_controller()->enable_video_grabbing(true);
                    break;
                case Message::VIDEO_RECORD_OFF:
                    owner_->get_controller()->enable_video_grabbing(false);
                    break;
                case Message::SELECT_CLIP:
                    owner_->get_controller()->choose_clip(value);
                    break;
            }
        }
    }
}

MidiInput::MidiInput(Application* owner) : 
        owner_(owner),
        messaging_queue_()
{
    verbose_ = false;
    //verbose_ = true;
    midi_in_ = 0;
    // RtMidiIn constructor
    try {
        midi_in_ = new RtMidiIn; // FIXME: should be use a pointer?
    }
    catch (RtError &error) {
        error.printMessage();
        exit(EXIT_FAILURE);
    }
  
    // Check available ports vs. specified.
    port_ = 0;
    ports_count_ = midi_in_->getPortCount();
    opened_ = false;
}

bool MidiInput::open(unsigned int port)
{
    ports_count_ = midi_in_->getPortCount();
    if (port >= ports_count_) 
    {
        //TODO: raise excepion !
        // delete midi_in_;
        std::cout << "Invalid input MIDI port!" << std::endl;
        // usage();
        opened_ = false;
        return false;
    }
    try 
    {
        midi_in_->openPort(port);
    }
    catch (RtError &error) 
    {
        std::cout << "Error opening MIDI port " << port << std::endl;
        error.printMessage();
        opened_ = false;
        return false;
    }
  
    // Set our callback function.  This should be done immediately after
    // opening the port to avoid having incoming messages written to the
    // queue instead of sent to the callback function.
    // TODO:2010-10-03:aalex:Pass only a pointer to the concurrent queue, no the whole MidiInput instance, 
    // which is not thread safe
    midi_in_->setCallback(&input_message_cb, (void *) this);
    // Don't ignore sysex, timing, or active sensing messages.
    //midi_in_->ignoreTypes(false, false, false);
    // ignore sysex, timing, or active sensing messages.
    midi_in_->ignoreTypes(true, true, true);
    opened_ = true;
    return true;
}

void MidiInput::set_verbose(bool verbose)
{
    verbose_ = verbose;
}

MidiInput::~MidiInput()
{
    delete midi_in_;
}

