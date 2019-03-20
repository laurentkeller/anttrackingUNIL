/*
 *  time_investment.cpp
 *  counts how many timesteps a given ant invests in the visit of various zones requested by the user and defined in a plume file
 *
 *  Created by Danielle Mersch on 4/8/11.
 *  Copyright 2011 __UNIL__. All rights reserved.
 *
 */

#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <getopt.h>

#include "exception.h"
#include "utils.h"
#include "plume.h"
#include "datfile.h"
#include "tags3.h"
#include "trackcvt.h"

using namespace std;

int main(int argc, char* argv[]){
try{

	string tagsfile = "";
	string datfile = "";
	string outfile = "";
	string plumefile = "";
	int duration(0);
	int startframe (0);
	int box (0);
	string zones[NUMBER_LINES_COLOR];
	int Nzones(0);

	// process arguments
	char option;
	while ((option = getopt(argc, argv, ":p:i:z:t:o:b:s:d:")) != -1) {
		switch (option)
		{
			case '?':
				return 1;
			case 'i':
				datfile = (string)optarg;
				break;
			case 't':
				tagsfile = (string)optarg;
				break;
			case 'p':
				plumefile = (string)optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'b':
				box = atoi(optarg);
				break;
			case 'd':
				duration = atoi(optarg);
				break;
			case 's':
				startframe = atoi(optarg);
				break;
			case 'z': {
				if (Nzones >= NUMBER_LINES_COLOR){
					cerr<<"Too many zones. Only "<<NUMBER_LINES_COLOR<<" zones possible."<<endl;
					return 1;
				}
				zones[Nzones] = optarg;
				Nzones++;
				break;
			}
		}
	}

	// check if essential parameters are there
	if (argc < 9){
		string info = "Usage: " + (string)argv[0] + " -i input.dat -t input.tags -p input.plume -z zone1 -b box -s startframe -d duration(frames) -o output.txt";
		throw Exception (USE, info);
	}

	if (datfile == ""){
		throw Exception(PARAMETER_ERROR, "Input.dat file is missing.");
	}
	if (tagsfile == ""){
		throw Exception(PARAMETER_ERROR, "Input.tags file is missing.");
	}
	if (outfile == ""){
		throw Exception(PARAMETER_ERROR, "Name of output.txt file is missing.");
	}
	if (plumefile == ""){
		throw Exception(PARAMETER_ERROR, "Input.plume file is missing.");
	}

	// test if output exists already
	ifstream f;
	f.open(outfile.c_str());
	if (f.is_open()){
		throw Exception(OUTPUT_EXISTS, outfile);
	}

	// open datfile, plume file, check coverage of plume file, validity of box, startframe and duration
	DatFile dat;
	dat.open(datfile, 0);

	TagsFile tgs;
	tgs.read_file(tagsfile.c_str());

	Plume plm;
	plm.read_plume(plumefile);

	if (plm.get_firstframe() > dat.get_last_frame() || plm.get_lastframe() < dat.get_first_frame()){
		throw Exception (PARAMETER_ERROR, "The frames covered by the plume file are not in the dat file.");
	}

	if (!is_valid_ID(box, box_list, box_count)){
		ostringstream os;
		os <<box;
		throw Exception(BOX_NOT_FOUND, os.str());
	}

	if (startframe < plm.get_firstframe() || startframe > plm.get_lastframe()){
		throw Exception(PARAMETER_ERROR, "Startframe is not within the range of frames covered by the plume file.");
	}

	if (duration <= 0 ){
		throw Exception(PARAMETER_ERROR, "The duration entered is invalid.");
	}

	// check if codes are valid
	bool zonestate[NUMBER_LINES_COLOR];
	memset(&zonestate, 0, sizeof(zonestate));
	string zonenames[NUMBER_LINES_COLOR];
	for (int i(0); i<Nzones; i++){
		if (zones[i].empty()){
			throw Exception(PARAMETER_ERROR,"Parameter of option -z is missing.");
		}else if(plm.exists(zones[i])== -1){
			throw Exception(PARAMETER_ERROR, "Zone: " + zones[i] + " does not exist in plume file.");
		}else{
			zonestate[plm.exists(zones[i])-1] = true;
			zonenames[plm.exists(zones[i])-1]= zones[i];
		}
	}

	/*for (int i(0); i < NUMBER_LINES_COLOR; i++){
		if (zonestate[i]){
			cout<<"Zone "<<zonenames[i]<<"is in "<<i+1<<endl;
		}
	}*/

	// create and init tables for counters
	int ctr_zone[tag_count][NUMBER_LINES_COLOR]; ///< table with counter of timestep spend in different zones
	memset(&ctr_zone, 0, sizeof(ctr_zone));
	int ctr_outzone[tag_count];///< table with counter of timespect spend outside the requested zones
	memset(&ctr_outzone, 0, sizeof(ctr_outzone));
	int ctr_nozone[tag_count]; ///< table with counter for timesteps spent in a not defined zone (subsample of outzone)
	memset(&ctr_nozone, 0, sizeof(ctr_nozone));
	int ctr_absent[tag_count]; ///< table with counter for timesteps where the ant was absent
	memset(&ctr_absent, 0, sizeof(ctr_absent));
	int ctr_nan[tag_count]; ///< table with counter if timesteps spend undetected
	memset(&ctr_nan, 0, sizeof(ctr_nan));
	int	fctr (0); ///< counter for number of frames read

	// go to startframe
	dat.go_to_frame(startframe);

	// open outfile
	ofstream g;
	g.open(outfile.c_str());
	if (!g.is_open()){
		throw Exception(CANNOT_OPEN_FILE, outfile);
	}

	// calculate size of each zone
	int zone_size[NUMBER_LINES_COLOR + 2]; // size of zones and outzone(idx = NUMBER_LINES_COLOR ) and nozone (idx = NUMBER_LINES_COLOR +1)
	memset(&zone_size, 0, sizeof(zone_size));
	//uint8_t* bitmap = plm.get_bitmap();
	for (int i(0); i<plm.size();i++){
		int code = plm.get_code(i);
		if (code <= NUMBER_LINES_COLOR && code >0){
			if (zonestate[code-1]){
				zone_size[code-1]++;		// zones
			}else{
				zone_size[NUMBER_LINES_COLOR]++;  // outzone
			}
		}else{
			zone_size[NUMBER_LINES_COLOR]++;	// outzone
			zone_size[NUMBER_LINES_COLOR + 1]++;  // nozone
		}
	}

  cout<<"You entered "<<Nzones<<" zones."<<endl;

	cout<<"-----------> Zones sizes (pixels) <----------"<<endl;
	for (int i(0); i<(NUMBER_LINES_COLOR); i++){  // why plus 2 ?
		if (zonestate[i]){
			cout<<zonenames[i]<<": "<<zone_size[i] * 25<<endl;
		}
	}
	cout<<"Outzone: "<<zone_size[NUMBER_LINES_COLOR] * 25 <<endl;
	cout<<"Nozone: "<<zone_size[NUMBER_LINES_COLOR+1] * 25 <<endl;
	cout<<"---------------------------------------------"<<endl;

	// read frames from datfile
	while(!dat.eof() && fctr < duration){
		framerec temp;
		if (dat.read_frame(temp)){
			for (int i(0); i < tag_count; i++){
				if (tgs.get_state(i) && (tgs.get_death(i) == 0 || tgs.get_death(i) > temp.frame)){
					// tag detected in correct box
					if (temp.tags[i].id == box && temp.tags[i].x != -1 ){
						position p (temp.tags[i].x, temp.tags[i].y);
						int code = plm.get_code(p);
						if (code <= NUMBER_LINES_COLOR && code >0 && zonestate[code-1]){
							ctr_zone[i][code-1]++;
						}else{
							ctr_outzone[i]++;  // contains every time step in which ant detected but outside of specified zones
							if (code > NUMBER_LINES_COLOR){
								ctr_nozone[i]++; // times steps in which ant detected in area not defined at all as zone (subsample of outzone)
							}
						}

					// tag absent / not visible
					}else if (temp.tags[i].y == -2){
						ctr_absent[i]++;
					// tag undetected but supposed but supposed to be visible
					}else{
						ctr_nan[i]++;
					}

				}
			}
		}
		fctr++;
	}
	dat.close();

	if (fctr < duration){
		cerr<<"Warning: Could only read "<<fctr<<" frames."<<endl;
	}


	//write proportion of time spend in different zones to outfile
	g<<"tag ";
	for (int j(0); j< NUMBER_LINES_COLOR; j++){
		if (zonestate[j]){
			g<<zonenames[j]<<" ";
		}
	}
	g<<"outzone no_zone absent undetected"<<endl;

	for (int i(0); i < tag_count; i++){
		if (tgs.get_state(i) && (tgs.get_death(i) == 0 || tgs.get_death(i) > startframe)){
			g<<tag_list[i]<<" ";
      int tot(0);
			for (int j(0); j< NUMBER_LINES_COLOR; j++){
				if (zonestate[j]){
					g<<ctr_zone[i][j]<<" ";
          tot += ctr_zone[i][j];
				}
			}
			g<<ctr_nozone[i]<<" "<<ctr_nozone[i]<<" "<<ctr_absent[i]<<" "<<ctr_nan[i]<<endl;
		}
	}

	g.close();
	return 0;
}catch(Exception e){
	return 1;
}
}
