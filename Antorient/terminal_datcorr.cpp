/*
 *  terminal_datcorr.cpp
 *  
 *
 *  Created by Danielle Mersch on 10/29/10.
 *  Copyright 2010 __UNIL__. All rights reserved.
 *
 */


#include <iostream>
#include <string>
#include "datcorr.h"
#include "exception.h"


/**\brief Displays the progression of the datcorr execution in the terminal
 * \param percent Percentage that is done
 * \param user_pointer
 */
void display_status(int percent, void* user_pointer){
	cout<<percent<<"% done"<<'\r'<<flush;
}


int main(int argc, char* argv[]){
	try{
	
		/// tests whether all required arguments are present
		if (argc != 5){
			string info = string(argv[0]) + " input.dat input.tags output.dat postprocessing.log";
			throw Exception(USE, info);
		}
		
		string datfile = (string)argv[1];
		string tagsfile = (string)argv[2];
		string output = (string)argv[3];
		string logfile = (string) argv[4];
		
		// starts the dat file correction
		ostringstream out;
		if (!execute_datcorr(datfile, tagsfile, output, logfile, &display_status, NULL)){
			cerr<<"The dat file correction encountered a problem and needs to close."<<endl;
		}
		
	
	}catch (Exception e) { }
	return 0;
}
