 /*
 *  velocity.cpp
 *  
 *
 *  Created by Danielle Mersch on 11/20/10.
 *  Copyright 2010 __UNIL__. All rights reserved.
 *
 */

#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

#include "datfile.h"
#include "trackcvt.h"
#include "exception.h"
#include "tags3.h"

using namespace std;

const double DOUBLE_MAX = 1.79769e+308;  /// biggest number possible in double


struct position{
	int x;
	int y;
};


//=====================================================================
int main(int argc, char* argv[]){
try {
	if (argc != 4){
		string info = (string) argv[0] + " input.dat input.tags name_outfiles";
		throw Exception(USE, info);
	}

	
	// read tags input
	TagsFile tgs;
	tgs.read_file(argv[2]);
	
	string filename = argv[3];
	ofstream output[tag_count];
	for (int i(0); i< tag_count; i++){
		if (tgs.get_state(i)){
			stringstream ss;
			ss<< filename << "_" <<tag_list[i]<<".txt";
			string outfile;
			ss>>outfile;
			// test if output exists
			ifstream f;
			f.open(outfile.c_str());
			if (f.is_open()){
				f.close();
				throw Exception(OUTPUT_EXISTS, outfile);
			}
			ofstream g;
			g.open(outfile.c_str());
			output[i].open(outfile.c_str());
			output[i]<<"#frame,time,box,xcoor,ycoor"<<endl;
			if (!output[i].is_open()){
				throw Exception(CANNOT_OPEN_FILE, outfile);
			}
		}
	}
	
	
	// open dat input
	DatFile dat;
	dat.open(argv[1],0);
	
	// read through dat file
	cout<<"reading datfile ..."<<endl;
	while(!dat.eof()){
		
		framerec temp;
		dat.read_frame(temp);
		
		for (int i(0); i< tag_count; i++){
			if (tgs.get_state(i) && (tgs.get_death(i)== 0 || tgs.get_death(i) > temp.frame)){
				if (temp.tags[i].x != -1){
					output[i]<< temp.frame <<",";
					output[i].precision(12);
					output[i]<< temp.time <<",";
					output[i]<<(int)temp.tags[i].id<<",";
					output[i].precision(4);
					output[i]<<temp.tags[i].x<<","<<temp.tags[i].y<<endl;
				}
			}
		}
		
		
	} 
	for (int i(0); i< tag_count; i++){
		output[i].close();
	}
	
	return 0;
}catch (Exception e) {
	
}

}
