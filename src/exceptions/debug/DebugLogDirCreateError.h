#pragma once

#include <exception>
#include <string>

#include "../../raytracer/Globals.h"

using namespace std;

class DebugLogDirCreateError : virtual public exception {
    public:
        explicit DebugLogDirCreateError(){};
        virtual ~DebugLogDirCreateError() throw(){};

        virtual const char* what() const throw() {
            return "Could not create debug log directory";
        };
};