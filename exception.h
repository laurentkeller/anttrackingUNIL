/*
 *  The exeception class allow to manage errors in a standardized way with error codes.
 *  It also permits to access error mesage from a graphic interface through a get_error method.
 *
 *  Created by Danielle Mersch on 10/31/08.
 *  Copyright 2008 __UNIL__. All rights reserved.
 *
 */

#ifndef __exception__
#define __exception__


#include <cstring>
#include <iostream>
#include <string>


using namespace std;


const int USE = 1;
const int CANNOT_OPEN_FILE = 2;
const int CANNOT_READ_FILE = 3;
const int OUTPUT_EXISTS = 4;
const int CANNOT_WRITE_FILE = 11;

const int INVALID_FRAME = 5;
const int TAG_NOT_FOUND = 6;
const int PARAMETER_ERROR = 9;
const int DATA_ERROR = 10;
const int NOT_ENOUGH_BYTES = 7;
const int BUFFER = 8;
const int UNKNOWN_OPTION = 12;
const int ARGUMENT_MISSING = 13;
const int OPTION_MISSING = 14;
const int BOX_NOT_FOUND = 15;
const int INTERNAL_ERROR = 16;

class Exception{

public:

Exception(int error, string info ="");

void display_error(int e, string info);
string get_error();

private:
string error_message;

};

#endif //__exception__

