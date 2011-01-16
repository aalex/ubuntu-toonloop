/*
 * Toonloop
 *
 * Copyright (c) 2010 Alexandre Quessy <alexandre@quessy.net>
 * Copyright (c) 2010 Tristan Matthews <le.businessman@gmail.com>
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

/** Main entry for the Toonloop program.
 */

#include <iostream>

#include "application.h"

int main(int argc, char* argv[])
{
    Application app;
    try 
    {
        app.run(argc, argv);
    }
    catch(const std::logic_error& e) 
    {
        std::cerr << "ERROR (std::logic_error): " << e.what() << "\n";
        return 1;
    }
    catch(const std::exception& e) 
    {
        std::cerr << "ERROR (std::exception): " << e.what() << "\n";
        return 1;
    }
    catch (...) 
    {
        std::cerr << "Exception of unknown type!\n";
        return 1;
    }
    return 0;
}

