/* @file trackconverter_modular.cpp
 *  
 *
 *  Created by Danielle Mersch.
 *  Copyright UNIL. All rights reserved.
 *
 *
 *  \brief The program trackconverter.cpp sorts the frames of the csv files and writes a ordered csv file (part 1)
 *		   and then converts the ordered csv files into a dat file (part 2).
 *	
 *  Part 1: 
 *	reads through all csv input files to determine wether the frames are 1. all present, 2. ordered and 3. complete 
 *  unordered frames are sorted and missing or incomplete frames filled with -1 data
 *  the program writes for each file with problems a corrected output file labeled with _corr before the extension and creates a list
 *  (framelist2) with the names of the files in order for part2
 *  Part 2:
 *	read sorted csv files, remove double detections and false postives within the same frame and converts the csv files to a single binary
 *	file. The program checks for each frame whether a given tag is detected twice. if a tag is detected twice in the same frame, 
 *	the program calculates the distance between the 2 detections, if the distance is smaller than P_TH -a precision threshold- 
 *	then the first detection is kept. If the distance exceeds P_TH then the distance and angle of each detection to the previous 
 *	detection is calculated and the detection resulting in the smaller distance and angle is kept. if the angle and distance give 
 *	conflicting information, the first detection is kept and a warning is written to the cout. The program outputs a resumee on 
 *	the number of false positives (this includes double detections due to overlaps!) number and times unkown markers were detected, etc. 
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <getopt.h>

#include <time.h>
#include "trackcvt.h"
#include "exception.h"
#include "tags3.h"
#include "utils.h"
#include "trackconverter_functions.h"



using namespace std;


const int testframe = 5011014;

/**\var const string VERSION
 * \brief Global constant specifying the version of the program
 */
const string VERSION = "TrackConverter 1.1.43";



//==================================================================================================
//															MAIN
//==================================================================================================

int main(int argc, char *argv[]){

try {
	
	/// tests whether all required arguments are present
    if (argc < 5) {
      string info = string(argv[0]) + " output.dat boxid postprocessing.log input1.csv [input2.csv [input3.csv [...]]] ";
      throw Exception(USE, info);
    }
	
	// tests whether specified .dat output exists
    ifstream f;
    f.open(argv[1], ios::in | ios::binary);
    if (f.is_open()) {
      f.close();
      throw Exception(OUTPUT_EXISTS, argv[1]);
    }

    // generates a filename with ".tags" extension from the .dat filename
    string tags_filename (argv[1]);
    tags_filename = tags_filename.substr(0, tags_filename.find_last_of('.'));
    tags_filename += ".tags";
    
    // tests whether .tags file exists
    f.open(tags_filename.c_str(), ios::in | ios::binary);
    if (f.is_open()) {
      f.close();
      throw Exception(OUTPUT_EXISTS, tags_filename);
    }
	
	// generate filename with .txt extension for coordinates that are deleted
    string txt_filename (argv[1]);
    txt_filename = txt_filename.substr(0, txt_filename.find_last_of('.'));
    txt_filename += "_double.txt";
	
	// tests whether double detected tags .txt file exists
    f.open(txt_filename.c_str(), ios::in | ios::binary);
    if (f.is_open()) {
		f.close();
		throw Exception(OUTPUT_EXISTS, txt_filename);
    }
		
	// gets the ID of the box and tests whether it is valid
	int box = atoi (argv[2]);
	if (!is_valid_box(box)){
		string b = (string) argv[2];
		throw Exception (BOX_NOT_FOUND,b);
	}
	
	// file with parameter infos: Program version, command and constants
	ofstream p;
	p.open(argv[3], ios_base::app);
	if(!p.is_open()){
		throw Exception (CANNOT_OPEN_FILE, argv[3]);
	}
	
	time_t t = time(NULL);
	string date = time_to_str(t);
	p<<VERSION<<" on "<<date<<endl;
	p<<"using command: ";
	for (int i (0); i< argc; i++){
		p<<argv[i]<<" ";
	}
	p<<endl;
	
		
	////////////////////////// Part 1: sort frames of csv files ///////////////////////////////////////
	
	vector <string> filelist2;				///< list containing filenames of ordered files for correction (part 2)
	map<int,bool> frames;					///< Map of frames read
	map<int, bool> delayed;					///< Map of frames that are delayed or missing
	vector <info> filelist;					///< contains list of files with respective first and last frame
	int files (0);							///< number of files
	list <frameline> framelist;				///< list of frames that need to be corrected
	list <frameline>::iterator iter;		///< iterator of framelist
	int firstframe(0);						///< first frame of experiment
	int lastframe(-1);						///< last frame of experiment
	int lastfr (-1);							///< last frame read
  
	map <int, bool> ordered_frames;			///< Map with all frames that are locally ordered
	list <timestamp> frames_read;		///< list containing for all frames read from the input their timestamp
	list <timestamp>::iterator frame_iter;
	
	// -------  reads one input file after the other ------------------
	for (int nfile(4); nfile < argc; nfile++) {
		
	  // opens input file that will be read
	  ifstream f;
	  f.open(argv[nfile], ios::in);
	  cout << "reading file: " << argv[nfile] << endl; 
	  if (!f.is_open()) {
      string info = string(argv[nfile]);
      throw Exception(CANNOT_OPEN_FILE, info);
	  }
	  files++;
		
	  // set variables for file reading
	  int unordered (0);  ///< nb unordered frames per file
	  int incomplete (0); ///< nb of incomplete frames per file
	  bool first (true);
    		
	  // read through file line after line to identify unordered and incomplete frames
    while (!f.eof()) {
      frameline my_frame;
      timestamp current; ///< timestamp (frame number and UNIX timestamp)
      if ( read_line(f, my_frame, frames, frames_read, current)){
        //cout<<"frame read"<<current.frame<<endl;
      
        update_filelist_limits(filelist, current, lastframe, firstframe, first, files, lastfr, (string)argv[nfile]); // for each file we define first and last frame
      
        if(filelist[files-1].start == 0){
          cout<<"file "<<files<<" before current frame read: "<<current.frame<<endl;
        }
        check_frame_order(filelist, framelist, delayed, ordered_frames, my_frame, current, lastfr, incomplete, unordered);
        if(filelist[files-1].start == 0){
          cout<<"file "<<files<<" after current frame read: "<<current.frame<<endl;
        }
      
        /// update lastframe read to current frame
        lastfr = current.frame;
        filelist[filelist.size()-1].end_time = current.time;
        }
    }
    cout<<"-> in file: "<<(string)argv[nfile]<<" : "<<unordered<<" unordered frames, "<<incomplete<<" incomplete frames"<<endl;
  
    cout<<"start: "<<filelist[files-1].start<<"-> "<<filelist[files-1].end<<endl;

    /// close file and reset file variables
    f.close();
    first = true;
    unordered = 0;
	  incomplete = 0;
	}
  cout<<"==================="<<endl;
  for (int i(0); i< filelist.size(); i++){
    cout<<"start: "<<filelist[i].start<<"-> "<<filelist[i].end<<endl;
  }
	/// check for missing frames
	cout<<"---------- checking for missing frames ---------- "<<endl<<flush;
	int nb_missing = check_missing_frames(filelist, frames, framelist, firstframe, lastframe);
	cout<<"--> there are "<<nb_missing<<" missing."<<endl;
	
	/// checks for frame overlap between two successive csv files and whether there are frames between 2 files
	/// if there is an overlap the startframe of the second file is set to the lastframe of first file plus 1
	/// if there are frames missing, calculate how many, if there are less than 8128, add them to the preceding file
	cout<<"-------- checking for overlap and missing frames between files ----------"<<endl<<flush;
	//cout<<"File: "<<filelist[0].name<<": start: "<<filelist[0].start<<", end: "<<filelist[0].end<<endl;
  check_overlap(filelist);


	/// sort frames and  merge parts of incomplete frames
	if (!framelist.empty() && framelist.size() > 1){
		framelist.sort(); 
		
		/*cout<<"List of frames to correct: "<<endl;
		for(iter = framelist.begin(); iter != (framelist.end()--) ; iter++){
			const frameline& frame1 = *iter;
			cout<<"Frame: "<<frame1.frame<<" has "<<frame1.segments<<" segments. Type: "<<frame1.type<<endl;
		}*/
		
		cout<<"-------- merging parts of incomplete frames --------"<<endl<<flush;
		merge_duplicates(framelist, iter); 
		/*
		// display frame list
		cout<<"List of frames to correct: "<<endl;
		for(iter = framelist.begin(); iter != (framelist.end()--) ; iter++){
			const frameline& frame1 = *iter;
			cout<<"Frame: "<<frame1.frame<<" has "<<frame1.segments<<" segments. Type: "<<frame1.type<<endl;
		}
		*/
	}
	
	// sorting all input frames, merging segments that were separated and checking for time coherence
	if (!frames_read.empty() && frames_read.size()> 1){
		frames_read.sort();
		merge_duplicates(frames_read,frame_iter);
	}
	
	
	/// correct files that have missing, incomplete or unordered frames
	cout<<"--------- correcting files with problems ---------"<<endl<<flush;
	if (!filelist.empty()){
    cout<<"--> "<<filelist.size()<<" files to correct"<<endl;
    correct_files(filelist, ordered_frames, frames_read, framelist, filelist2);
	}else{
		cout<<"->nothing to correct"<<endl;
		
	}
	
	
	////////////////////////// Part 2: convert sorted csv files to a dat file ///////////////////////////////
	
	cout<<"-------- reading ordered files and writing dat file -------"<<endl<<flush;
	
	/// open binary output file (.dat)
	fstream g;
	g.open(argv[1], ios::in | ios::out | ios::binary | ios::trunc);
	if (!g.is_open()) {
		throw Exception(CANNOT_OPEN_FILE, argv[1]);
	}
	
	/// create textfile with all coordinates that were thrown away
	ofstream g2; 
	g2.open(txt_filename.c_str());
	if(!g2.is_open()){
		throw Exception(CANNOT_OPEN_FILE, txt_filename);
	}
	g2<<"#frame,tag,box,x_coor,y_coor,angle"<<endl;
	
	/// set variables for reading the csv input files
	TagsFile tgs;											///< empty tags class
	frames.clear();											///< clearing map of frames read: will be refilled
	uint8_t id (0);											///< id of box
	previous lastdet[tag_count];							///< holds the last position of each tag
	memset(&lastdet, -1, sizeof(lastdet));					///< initializing the table with the last positions
	map<int,int>unknown_tags;								///< map associating id of unknown tags with a counter of occurences
	int false_positif (0);									///< number of false positives
	int freak_detections (0);								/// number of coordinates outside of camera detection range
	bool detection[tag_count];								///< keeps for each tag the detection status: true if detected once, false if detected twice or not at all
	memset(&detection, 0, sizeof(detection));				///< intializing to false for all tags
	
	/// reads one input file after the other from the filelist2
	for (int nfile(0); nfile < filelist2.size(); nfile++) {
		
		id = box;
		
		/// opens input file
		ifstream f;
		f.open(filelist2[nfile].c_str());
		cout << "File: " << filelist2[nfile] << endl;
		if (!f.is_open()) {
			string info = string(filelist2[nfile]);
			throw Exception(CANNOT_OPEN_FILE, info);
		}

		/// reads line after line from input file
		while (!f.eof()){
			
			string s;		///< string to read line from file
			
			getline(f, s);
			if (!f.fail()){
				line temp;		///< temporary line to store data from one frame
				
				/// parses input into temporary line
				if (!parse_line(s, temp)){
					string info = "Error reading input from line : "+ s;
					throw Exception (DATA_ERROR, info);
				}
			
				// adds the frame to known frames
				frames[temp.frame] = true;
			
				// creates framerec and remove double detections 
				framerec curr;
				memset(&curr, -1, sizeof(curr));
				curr.frame = temp.frame;
				curr.time = temp.time;
				// reset detection status fo false for all tags
				memset(&detection, 0, sizeof(detection));
				
				for (int i(0); i < temp.tags.size(); i++){
					int tag = temp.tags[i].id;	
					int idx (-1);				///< index of the tag in the table tag_list
					
					/// for unknown tags
					if(!find_index(tag, idx)){
						if (!unknown_tags.empty()){
							map<int, int>::iterator iter=unknown_tags.begin(); 
							if (iter-> first != tag){
								do{
									iter++;
								}while(iter-> first != tag && iter != unknown_tags.end());
							}
							if (iter == unknown_tags.end()){
								unknown_tags[tag]=1;
							}else{
								iter->second++;
							}
						}else{
							unknown_tags[tag]=1;
						}
					
					/// for known tags, checks whether first detection or if not (i.e. double detection) which detection is the correct one
					}else{
						if (curr.tags[idx].x == -1){  /// if the tag is detected for the first time in this frame, copy information
							curr.tags[idx].id = id;
							curr.tags[idx].x = temp.tags[i].p.x;
							curr.tags[idx].y = temp.tags[i].p.y;
							curr.tags[idx].a = temp.tags[i].a;
							detection[idx] = true;
						}else{ /// if the tag had already been detected in this frame, calculate position difference with the other detection
							position pos (curr.tags[idx].x, curr.tags[idx].y);
							double d = dist(pos, temp.tags[i].p);   ///< distance between detections within the same frame
							detection[idx] = false;
							/// if the position difference is smaller than P_TH, then it is the same detection, 
							/// the double detection is probably due to the segment overlap. we keep the position of the first detection
							if (d > P_TH){
								/// if the position difference exceeds P_TH, calculate the distance to the previous detection 
								/// and keep the one close to the previous detection, unless the angle change and distance give conflicting information
								false_positif++;
								/// calculate distance to previous detection
								double d1 = dist(lastdet[idx].p, pos);
								double d2 = dist(lastdet[idx].p, temp.tags[i].p);
								/// calculate angle change
								double a1 = abs(lastdet[idx].a - curr.tags[idx].a) / (double) 100;
								double a2 = abs(lastdet[idx].a - temp.tags[i].a) / (double) 100;
								convert(a1);
								convert(a2);
															
								if (d1 > d2 && a1 > a2){  /// if the second detection is closer to the last detection, hence replace the first detection by the second detection
									curr.tags[idx].x = temp.tags[i].p.x;
									curr.tags[idx].y = temp.tags[i].p.y;
									curr.tags[idx].a = temp.tags[i].a;
								/// if the angle change and distance give conplicting information, keep the first position and send a warning to the user
								}else if ((d1 < d2 && a1 > a2) || (d1 < d2 && a1 > a2)){
									// write tags with conflicting double detections to file
									g2<<temp.frame<<","<<temp.tags[i].id<<","<<box<<","<<temp.tags[i].p.x<<","<<temp.tags[i].p.y<<","<<temp.tags[i].a<<endl;
									g2<<curr.frame<<","<<tag_list[idx]<<","<<box<<","<<curr.tags[idx].x<<","<<curr.tags[idx].y<<","<<curr.tags[idx].a<<endl;
									p<<"WARNING !"<<endl;
									p<<"in frame "<<temp.frame<<" double detection for tag: "<<tag_list[idx]<<endl;
									p<<"angle difference and distance difference with last frame indicate conflicting information"<<endl;
									p<<"the first detected value is retained, please check in file."<<endl;
								}
							}
						}
						//finally checks whether the retained coordinates are within the correct range
						if ((curr.tags[idx].x > IMAGE_WIDTH) || (curr.tags[idx].y > IMAGE_HEIGHT) || ((curr.tags[idx].x >=0) && (curr.tags[idx].y <0) ) || ((curr.tags[idx].y >=0) && (curr.tags[idx].x <0))){
							curr.tags[idx].x = -1;
							curr.tags[idx].y = -1;
							curr.tags[idx].a = -1;
							detection[idx] = false;
							freak_detections++;
						}

					}
				}
			
		
				/// writes a framerec to the dat file
				g.write((char *) &curr, sizeof(curr));
					
				/// updates last detection for each tag and tgs for .tags file
				for (int i (0); i < tag_count; i++){
					if (curr.tags[i].x != -1){
						// update count and last detection for tag in .tags file
						if (detection[i]){
							tgs.set_count(i, tgs.get_count(i)+1);
							if (curr.frame > lastdet[i].frame){
								tgs.set_last_det(i, curr.frame);
							}
						}
						// update last detection for tag
						lastdet[i].frame = curr.frame;
						lastdet[i].p.x = curr.tags[i].x;
						lastdet[i].p.y = curr.tags[i].y;
						lastdet[i].a = curr.tags[i].a;
					}
				}
			}
		} // end of while
		
		/// close input file
		f.close();
	}// end of for (nfile)
        
	/// marks all tags that have been detected at least once
  for (int i(0); i<tag_count; i++) {
    if (tgs.get_count(i)!=0) {
      tgs.set_state(i, true);
    }
  }

    /// writes tags file
	tgs.write_file(tags_filename.c_str());
	g2.close();
	
	  
	// resumee of data quality
	unsigned int count(0);
	for(map<int, int>::iterator iter=unknown_tags.begin(); iter != unknown_tags.end(); iter++){
		count += iter ->second;
	}
	p <<"In box "<<box<<", "<<unknown_tags.size()<<" unknown tags generate "<<count<<" false detections.";
	p<<"In box "<<box<<", there are "<<false_positif<<" false positives or intermarker confusions with known tags.";
	p<<"-------------------------------------------------------------------"<<endl;
	p.close();
	
	cout<<unknown_tags.size()<<" unknown tags generating "<<count<<" false detections."<<endl;
	cout<<"Number of false positives or intermarker confusions with known tags: "<<false_positif<<endl;
	
	cout<<"Number of freak coordinates removed: "<<freak_detections<<endl;

  } catch(Exception e) { 
    return 1;
  }
  return 0;
}

