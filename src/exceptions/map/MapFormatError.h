#pragma once

#include <exception>
#include <string>

using namespace std;

class MapFormatError : virtual public exception {
    protected:
        int format;

    public:
        explicit MapFormatError(int format_val):
            format(format_val)
        {};

        virtual ~MapFormatError() throw(){};

        virtual const char* what() const throw() {
            string ret_val = "Map format should be 0 or 1: " + to_string(format);
            return ret_val.c_str();
        };
};