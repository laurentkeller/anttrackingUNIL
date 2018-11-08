/*
 *  zone2id.cpp
 *  takes all detections (in a box) in a given zone defined in a plume file, and writes a new dat files in which these detections are labelled with a distinct ID
 *
 *  Created by Danielle Mersch on 5/1/13.
 *  Copyright 2013 __UNIL__. All rights reserved.
 *
 */

#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <getopt.h>
#include <cstdlib> 

#include "exception.h"
#include "utils.h"
#include "plume.h"
#include "datfile.h"
#include "trackcvt.h"

using namespace std;

int main(int argc, char* argv[]){
try{
	
	string datfile = "";
	string outfile = "";
	string plumefile = "";
	int new_id (0);
  int box(0);
	string zone;
	
	// process arguments
	char option;
	while ((option = getopt(argc, argv, ":p:i:z:o:d:b:")) != -1) {
		switch (option)
		{
			case '?':
				return 1;
			case 'd':
				datfile = (string)optarg;
				break;
			case 'p':
				plumefile = (string)optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'i':
				new_id = atoi(optarg);
				break;
      case 'b':
				box = atoi(optarg);
				break;
      case 'z': {
				zone = (string)optarg;
				break;
			}
		}
	}
	
	// check if essential parameters are there
	if (argc < 7){
		string info = "Usage: " + (string)argv[0] + " -d input.dat -p input.plume -z zone -i new_id -o output.dat -b box";
		throw Exception (USE, info);
	}
	
	if (datfile == ""){
		throw Exception(PARAMETER_ERROR, "Input.dat file is missing.");
	}
	if (outfile == ""){
		throw Exception(PARAMETER_ERROR, "Name of output.dat file is missing.");
	}
	if (plumefile == ""){
		throw Exception(PARAMETER_ERROR, "Input.plume file is missing.");
	}
	if (!is_valid_ID(new_id, box_list, box_count)){
		throw Exception(BOX_NOT_FOUND, to_string(new_id));
	}
  if (!is_valid_ID(box, box_list, box_count)){
		throw Exception(BOX_NOT_FOUND, to_string(box));
	}


	// check whether output exists already and creates it, open input datfile, plume file, check coverage of plume file, validity of box,   
	DatFile dat_out;
  dat_out.create_dat(outfile);
    
  DatFile dat;
	dat.open(datfile, 0);
	
	Plume plm;
	plm.read_plume(plumefile);
	if (plm.get_firstframe() > dat.get_last_frame() || plm.get_lastframe() < dat.get_first_frame()){
		throw Exception (PARAMETER_ERROR, "The frames covered by the plume file are not in the dat file.");
	}
	// check if zone code is valid
  bool zonestate[NUMBER_LINES_COLOR];
	memset(&zonestate, 0, sizeof(zonestate));
  if (zone.empty()){
    throw Exception(PARAMETER_ERROR,"Parameter of option -z is missing.");
  }else if(plm.exists(zone)== -1){
    throw Exception(PARAMETER_ERROR, "Zone: " + zone + " does not exist in plume file.");
  }else{
    zonestate[plm.exists(zone)-1] = true;
  }
	 
	// read frames from datfile
	while(!dat.eof()){
		framerec temp;
		if (dat.read_frame(temp)){
			if (temp.frame>=plm.get_firstframe()&& temp.frame<=plm.get_lastframe()){
				for (int i(0); i < tag_count; i++){
				
					// tag detected in correct box
					if (temp.tags[i].id == box && temp.tags[i].x != -1 ){
						  position p (temp.tags[i].x, temp.tags[i].y);
						  int code = plm.get_code(p);
						  if (code <= NUMBER_LINES_COLOR && code >0 && zonestate[code-1]){
						    temp.tags[i].id = new_id;
						  }
		
					}
				}
			}
      		dat_out.write_frame(&temp);
		}
	}
	dat.close();
	dat_out.close();
	return 0;
}catch(Exception e){
	return 1;
}
}
