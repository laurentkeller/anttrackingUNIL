/*
 * change_tagid.cpp
 * if a taglist (old tag, new tag) is specified, then the program changes the id of the tags (from old to new) specified in the taglist
 *
 *  Modified by Nathalie Stroeymeyt on June 8th 2015 based on a program created by  created by Danielle Mersch on 5/1/13.

 *  Copyright 2013 __UNIL__. All rights reserved.
 *
 */

#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <getopt.h>
#include <cstdlib> 

#include "exception.h"
#include "utils.h"
#include "datfile.h"
#include "trackcvt.h"
#include "tags3.h"


using namespace std;

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


// read tag list containing oldtag id, new tag id
void read_taglist(const string& taglist,vector <bool>& to_change,vector <int>& new_ids){
  ifstream f;
  f.open(taglist.c_str());
  if (!f.is_open()){
    throw Exception(CANNOT_OPEN_FILE, taglist);
  }
  while (!f.eof()){
    string s;
    getline(f,s);
    if (!s.empty()){
      if (s[0]!= '#'){
        stringstream ss;
        ss.str(s);
        int oldtag;
	int newtag;
        ss>> oldtag;
	//cout  << oldtag << endl;
        ss.ignore(1,',');
        ss>> newtag;
	//cout  << newtag << endl;
        // check validity of old and new tag, and new box ID
        if (is_valid_ID(oldtag,tag_list,tag_count) && is_valid_ID(newtag,tag_list,tag_count)){
		int old_idx(-1);
		if (!find_index(oldtag, old_idx)){
				ostringstream sss;
				sss<<oldtag;
				string info = sss.str();
			throw Exception(TAG_NOT_FOUND, info);
		}
		//cout << old_idx << endl;
		to_change[old_idx] = 1;
		new_ids[old_idx] = newtag;
        }else{
          throw Exception(PARAMETER_ERROR,"ID error in taglist input. Input is: " + s);
        }
      }
    }
  }
  f.close();

};


int main(int argc, char* argv[]){
	try{
	
		string datfile = "";
		string outfile = "";
	  	string tagsfile = "";
	  	string taglist = "";
		int box(0);
	  	bool change_tag = false;
		vector <bool> to_change(tag_count,0);
		vector <bool> wordy(tag_count,1);
		vector <int> new_ids(tag_count,-1);


		// process arguments
		char option;
		while ((option = getopt(argc, argv, ":o:d:t:b:l:")) != -1) {
			switch (option){
				case '?':
					return 1;
				case 'd':
					datfile = (string)optarg;
					break;
				case 'o':
					outfile = optarg;
					break;
	      			case 't':
					tagsfile = optarg;
					break;
	      			case 'l':
					taglist = optarg;
					change_tag = true;
					break;
	      			case 'b':
					box = atoi(optarg);
					break;
			}
		}
	
		// check if essential parameters are there
		if (argc != 11){
			string info = "Usage: " + (string)argv[0] + " -d input.dat -t tagsfile -o output.dat -b box -l taglist.txt";
			throw Exception (USE, info);
		}
		if (datfile == ""){
			throw Exception(PARAMETER_ERROR, "Input.dat file is missing.");
		}
		if (outfile == ""){
			throw Exception(PARAMETER_ERROR, "Name of output.dat file is missing.");
		}
	  	if (tagsfile == ""){
			throw Exception(PARAMETER_ERROR, "Name of input.tags file is missing.");
		}
	  	if (change_tag && taglist == ""){
			throw Exception(PARAMETER_ERROR, "Name of taglist.txt file is missing.");
		}
	  	if (!is_valid_ID(box, box_list, box_count)){
			throw Exception(BOX_NOT_FOUND, to_string(box));
		}


		// check whether output exists already and creates it, open input datfile, plume file, check coverage of plume file, validity of box,   
		DatFile dat_out;
	  	dat_out.create_dat(outfile);
	    
	  	DatFile dat;
		dat.open(datfile, 0);
	  
		TagsFile tgs;
		tgs.read_file(tagsfile.c_str());
	
	  	if (change_tag){
			read_taglist(taglist,to_change,new_ids);
	  	}
		 
		// read frames from datfile
		while(!dat.eof()){
			framerec temp;
			if (dat.read_frame(temp)){
				for (int i(0); i < tag_count; i++){
					if (tgs.get_state(i)){

		  				// tag detected in correct box
		  				if (temp.tags[i].id == box && temp.tags[i].x != -1 ){

		    					position p (temp.tags[i].x, temp.tags[i].y);
		    					//cerr<<"frame :" <<temp.frame<<", tag: "<<tag_list[i]<<endl;
		      					// check whether the current tag needs to be changed
		      					if (change_tag && to_change[i]){

		        					tag_pos tmp = temp.tags[i]; // copy info from old tag in temporay structure
		        					// find new tag ID 
								if (wordy[i]){
									cout << "Tag " << tgs.get_tag(i) << " becomes " << new_ids[i] << endl;
									wordy[i]=0;
								}
								int newidx(-1);
								if (!find_index(new_ids[i], newidx)){
									ostringstream sss;
									sss<<new_ids[i];
									string info = sss.str();
									throw Exception(TAG_NOT_FOUND, info);
								}
	            						tag_pos empty ; empty.x = -1; empty.y = -1; empty.a = -1; empty.id=255; empty.padding =0;
	            						temp.tags[newidx] = tmp; // assign new tag id the position
	            						temp.tags[i] = empty; // assign old tag id an empty position
		      					}//if (change_tag && tag_to_change[i])
		    				}//if (temp.tags[i].id == box && temp.tags[i].x != -1 )
		  			}//if (tgs.get_state(i))
				}//for (int i(0); i < tag_count; i++){
	      			dat_out.write_frame(&temp);
			}//if (dat.read_frame(temp))
		}//while(!dat.eof())
		dat.close();
		dat_out.close();
		return 0;
	}catch(Exception e){//try
		return 1;
	}//catch
}//int main
