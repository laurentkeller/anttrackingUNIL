/*
 *  exception.cpp
 *  
 *
 *  Created by Danielle Mersch on 10/31/08.
 *  Copyright 2008 __UNIL__. All rights reserved.
 *
 */

#include "exception.h"

Exception::Exception(int error, string info){
	display_error(error, info);
}

void Exception::display_error(int e, string info){
	switch (e){
	case TAG_NOT_FOUND:
		error_message = "Unknown tag: " + info;
		break;
	case CANNOT_OPEN_FILE:
		error_message = "Cannot open file: " + info;
		break;
	case CANNOT_READ_FILE:
		error_message = "Cannot read file: " + info;
		break;
	case NOT_ENOUGH_BYTES:
	  error_message = "Not enough bytes " + info;
		break;
	case INVALID_FRAME:
	  error_message = "Invalid frame: " + info;
		break;
	case USE:
	  error_message = "Usage: " + info;
		break;
	case OUTPUT_EXISTS:
		error_message = "Output file " + info + " exists.";
		break;
	case BUFFER:
	  error_message = "Buffer problem";
		break;	
	case PARAMETER_ERROR: 
	case DATA_ERROR:
	case OPTION_MISSING:
	  error_message = info;
		break;
	case CANNOT_WRITE_FILE:
	  error_message = "Cannot write output file: " + info;
		break;
	case UNKNOWN_OPTION:
	  error_message = "Unknown option: " + info;
		break;
	case ARGUMENT_MISSING:
		error_message = "Option "+ info + " requires an argument.";
		break;
	case BOX_NOT_FOUND:
		error_message = "Unknown experimental box: " + info;
		break;
	default:
		error_message = "Unknown error";
		break;
	}	
	cerr<<error_message<<endl;
}


string Exception::get_error(){
	return error_message;
}
