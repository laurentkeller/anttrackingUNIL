/*
 *  extrapolate_coordinates_step2.cpp
 *
 *  Modified by Nathalie Stroeymeyt in 2016 based on a program created by Danielle Mersch.
 *  Copyright UNIL. All rights reserved. *
 */


#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <cstdlib>
#include <getopt.h>

#include "datfile.h"
#include "tags3.h"
#include "exception.h"
#include "trackcvt.h"



struct tag_position{
	bool state;
	int box;
	int x;
	int y;
	double a;
};

typedef tag_position tag_table[tag_count];

//==========================================================
/* \ finds the index of a tag in the table
 * \param tag Tag for which the index needs to be found
 * \param idx Index of tag in tag_list table, by default the index is -1
 * \return True if index has been found, false otherwise
 */
bool find_index(int tag, int& idx){
	idx=-1;
	int k(0);
	do {
		if (tag_list[k] == tag){
			idx = k;
			return true;
		}
		k++;
	}while(k<tag_count && idx == -1);
	return false;
}



//==========================================================
int main(int argc, char* argv[]){
try{

	// test whether all parameters are presen
	if (argc != 4){
		string info = (string) argv[0] + " input.dat input.txt output.dat";
		throw Exception(USE, info);
	}

	// opens input files
	DatFile datin;
	datin.open((string) argv[1], 0);

	// opens text file
	ifstream f;
	f.open(argv[2]);
	if(!f.is_open()){
		throw(CANNOT_OPEN_FILE, (string) argv[2]);
	}

	// test whether output datfile exists already, and creates output file
	DatFile datout;
	// datout.create_dat((string) argv[3]);
  datout.copy((string) argv[1], (string) argv[3]);
  datout.open((string) argv[3], true);

  datin.close();

	// structures
	map<int,tag_table*> frames_to_correct;

	// read input text file
	cout<<"reading frames to correct from input file ..."<<endl;
	while(!f.eof()){
		string s;
		istringstream ss;
		/*int frame (0);
		int tag (-1);
		int id (-1);
		int x(-1);
		int y(-1);
		int a(-1);*/
    unsigned int frame = 0;
    int boxid = 0;
		// read data from input
		getline(f,s);
		if (!s.empty()){
			if (s[0]=='#'){
				getline(f,s);
			}
      tag_pos temp_tag;
			//cout<<"line read: "<<s<<endl;
			ss.str(s);
			ss>>frame;
			ss.ignore(1, ',');
			ss>>temp_tag.id;
			//cout<<"Tag "<<tag<<endl;
			ss.ignore(1,',');
			ss>>boxid;
			ss.ignore(1,',');
			ss>>temp_tag.x;
			ss.ignore(1,',');
			ss>>temp_tag.y;
			ss.ignore(1,',');
			ss>>temp_tag.a;
      datout.write_tag(frame, temp_tag);
			// create new tag_table and initialize with -1
			//tag_table* temp = new tag_table[1];
			//memset (temp, 0, sizeof(tag_table));
			//find index of tag and fill temp structure
			/*int idx(-1);
			if (!find_index(tag, idx)){
				ostringstream ss;
				ss<<tag;
				string info = ss.str();
				throw Exception(TAG_NOT_FOUND, info);
			}*/
			/*(*temp)[idx].state = true;
			(*temp)[idx].box = id;
			(*temp)[idx].x = x;
			(*temp)[idx].y = y;
			(*temp)[idx].a = a;*/
			// check whether frame is already among frames to correct, if not add it
			/*map <int, tag_table*>::iterator it_tags;
			it_tags = frames_to_correct.find(frame);*/
			/*if (it_tags == frames_to_correct.end()){
				frames_to_correct[frame] = temp;
			// if the frame is already in the list, add another tag for correction
			}else{
				(*frames_to_correct[frame])[idx].state = true;
				(*frames_to_correct[frame])[idx].box = id;
				(*frames_to_correct[frame])[idx].x = x;
				(*frames_to_correct[frame])[idx].y = y;
				(*frames_to_correct[frame])[idx].a = a;
			}*/
		}
	}
	f.close();
  datout.close();

	// read input datfile and write corrected output
	/*cout<<"Input file read. Now about to open dat file..."<<endl;

	while(!datin.eof()){
		framerec temp;
		if (datin.read_frame(temp)){
      // This frame from datin needs to be corrected
      // Datin is only used to loop through the frame numbers, the read frame is directly written to the output if it does not need to be corrected
			if (frames_to_correct.find(temp.frame) != frames_to_correct.end()){
				//cout<<"tag(s) being changed in frame "<<temp.frame<<endl;
				for (int i(0); i < tag_count; i++){
					if ((*frames_to_correct[temp.frame])[i].state){
						//cout<<"tag "<<tag_list[i]<<endl;
						temp.tags[i].id = (*frames_to_correct[temp.frame])[i].box;
						temp.tags[i].x = (*frames_to_correct[temp.frame])[i].x;
						temp.tags[i].y = (*frames_to_correct[temp.frame])[i].y;
						temp.tags[i].a = (*frames_to_correct[temp.frame])[i].a;
					}
				}
			}
			datout.write_frame(&temp);
		}
	}*/

	return 0;
}catch (Exception e) {
}
}
