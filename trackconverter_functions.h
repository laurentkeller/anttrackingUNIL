/** @file trackconverter.cc
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



using namespace std;


/**\var const double P_TH
 * \brief Global constant specifiying the position difference threshold, 
 * for double detections with a greater position difference, one of the detections is a false positif
 */
const double P_TH = 2.5; 

const int MISS = 8128; // number of missing frames accepted between 2 files, the missing frames will be added at the beginning of the seconde file.
						// if more frames are missing, then a new file will be created.

/// Structure describing each csv input file
struct info{
	string name;	///< name of the input file
	int start;		///< first frame in the input file
	int end;		///< last frame in the input file
	bool state;		///< state of the input file: true if frames all present, complete and in order, false otherwise
	double start_time;	///< unix time of last frame in file
	double end_time;	///< unix time of first frame in file

	bool operator ==(const info& file_to_find);
};


/// Structure describing each line in an csv input file
struct frameline{
	int frame;		///< number of the frame
	double time;	///< time of the frame acquisition
	int segments;	///< number of image segments acquired: if all segments are presents, segments = 15
	int tags;		///< number of tags detected in this frame
	char type;		///< type of frame if unordered(U), incomplete (I) or missing (M)
	string content;	///< content of frame (id of tags, positions, boxes and angles)
  frameline(int a,double b,int c,int d,char e,string f):frame(a),time(b),segments(c),tags(d),type(e),content(f){}
  frameline():frame(0),time(0),segments(0),tags(0),type('N'),content(""){}
};

/// Structure defining the location of a marker
struct marker {
	int id;		///< id of box
	position p;  ///< position
	short a;   ///< angle
};

/// Structure defining each line of the file (like frameline) but the information of tags are stored in a vector, 
struct line{
	double time;		///< unix time of framerec
	unsigned int frame;	///< number of frame
	int seg;			///< number of segmants analysed in the frame. normally 15 segments, if less then there are segments missing
	int nb;				///< number of tags detected in the frame
	vector <marker> tags; /// table of markers containing the information of each detected tag, and 0 for the others
};

/// Structure describing the previous detection of a tag (BEWARE: no box information! We assume that files are corrected for each box separatly!)
struct previous {
	int frame;  ///< frame of the previous detection
	position p;	///< position of previous detection
	short a;	///< angle of previous detection
};

///Structure describing the every frame read from the input: keeping the frame number and the timestamp
struct timestamp{
	int frame;
	double time;
  timestamp(): frame(0), time(0.0){}
  timestamp(int a, double b): frame(a), time(b){}
};

//==========================================================
/**\fn bool is_valid_box(int box)
 * \brief Tests whether the id of the box is valid
 * \param box ID of the box
 * \return True if the  box has a valid id, false otherwise;
 */
bool is_valid_box(int box);
//==========================================================
/**\fn bool operator<(const frameline& f1,const frameline& f2)
 * \brief Compares the frames of two framelines, determines which one has a smaller framenumber
 * \param f1 Line 1 from file
 * \param f2 Line 2 from file
 * \return True if the frame of f1 is smaller than the frame of f2, false otherwise 
 */
bool operator<(const frameline& f1,const frameline& f2);
//==========================================================
/**\fn bool operator<(const timestamp& t1,const timestamp& t2)
 * \brief Compares the frames of two timestamps, determines which one has a smaller framenumber
 * \param f1 Frame from first element in list
 * \param f2 Frame from second element in list
 * \return True if the frame of t1 is smaller than the frame of t2, false otherwise 
 */
bool operator<(const timestamp& t1,const timestamp& t2);

//==========================================================
/**\fn void merge_duplicates(list <frameline>& framelist,list <frameline>::iterator iter)
 * \brief Merges part of incomplete frames
 * \param framelist list of frames with problems, contains all information of frame 
 * \param iter Iterator to list of frames with ptoblems
 */
void merge_duplicates(list <frameline>& framelist,list <frameline>::iterator iter);

//==========================================================
/**\fn void merge_duplicates(list <timestamp>& frame_read,list <timestamp>::iterator frame_iter)
 * \brief Merges part of incomplete frames of all frames read from the input and verifies ciherence of timestamp
 * \param frames_read List of frames read from input, contains framenumber and timestamp 
 * \param iter Iterator to  list of frames read
 */
void merge_duplicates(list <timestamp>& frames_read, list <timestamp>::iterator frame_iter);

//==========================================================
/**\fn inline double round2(double x)
 * \brief Rounds a real number (function is needed because Microsoft Visual Studio lacks the round() funtion from <cmath>)
 * \param x The number to be rounded
 * \return Rounded number
 */
inline double round2(double x);

//==========================================================
/**\fn bool find_index(int tag, int& idx)
 * \brief Finds the index of a tag in the list defined in trackcvt.h
 * \param tag Id of the tag to be searched
 * \param idx Index of the tag in the list tag_list defined in trackcvt2.h
 * \return True if the tag was found in the list, false otherwise
 */
bool find_index(int tag, int& idx);

//==========================================================
/**\fn bool parse_line(string& s, line& temp)
 * \brief Parses a line from the csv input file, stores information in line structure 
 * \param s String containing a line (information from 1 frame) from the csv input file
 * \param temo Line structure to store the information from a frame temporally
 * \return True if the reading of the line was a success, false otherwise
 */
bool parse_line(string& s, line& temp);

//==========================================================
/**\fn double dist(const position& p1, const position& p2)
 * \brief Calculate the distance between to points
 * \param p1 Point of position 1
 * \param p2 Point of position 2
 * \return double Return the distance between the 2 points
 */
double dist(const position& p1, const position& p2);

//==========================================================
/**\fn void convert(double& a)
 * \brief Converts an angle between -180 and +180 to an angle between 0-360¬∞
 * \param a Angle to be converted
 * \retrun void
 */
void convert(double& a);

//==========================================================
/**\fn void convert(double& a)
 * \brief Reads a line from the csv input, extracts frame number, timestamp, nb segments and nb tags detected
 * \param f Input stream
 * \param current Timestamp containing frame number and UNIX timestamp
 * \param segments Number of segments analyzed in image
 * \return True if reading was successful 
 */
bool read_line(ifstream& f, frameline& my_frame, map<int,bool>& frames,list <timestamp>& frames_read, timestamp& my_current);

//==========================================================
/**\fn void update_filelist_limits(vector <info>& filelist, const timestamp& current, int& lastframe, int& firstframe, bool& first)
 * \brief Updates the 
 * \param filelist Info on each file with start, end frame, 
 * \param current Timestamp containing frame number and UNIX timestamp
 * \param lastframe Last frame in exeriment
 * \param firstframe
 * \param first True if this 
 */
void update_filelist_limits(vector <info>& filelist, const timestamp& current, int& lastframe, int& firstframe, bool& first, int files, int lastfr, string filename);

//==========================================================
/**\fn void check_frame_order(vector <info>& filelist, list <frameline>& framelist, 	map<int, bool>& delayed, map <int, bool>& ordered_frames, frameline& my_frame, int lastfr, int& incomplete, int& unordered)
 * \brief Checks order if frames and signals frames that are unordered or incomplete 
 * \param filelist
 * \param framelist
 * \param delayed Table with frames that are delayed
 * \param ordered_frames
 * \param my_frame Data from one frame
 * \param lastfr Last frame read
 * \param incomplete Number of incomplete frames
 * \param unordered Number of unordered frames
 */
void check_frame_order(vector <info>& filelist, list <frameline>& framelist, 	map<int, bool>& delayed, map <int, bool>& ordered_frames, frameline& my_frame, const timestamp& current, int lastfr, int& incomplete, int& unordered);

//==========================================================
/**\fn int check_missing_frames(vector <info> & filelist, map<int,bool>& frames, const int& firstframe, const int& lastframe)
 * \brief Check for missing frames. Each missing frame is added to a framelist and the corresponding file is marked for correction.
 * \param filelist
 * \param frames Table listing all frames that were in file
 * \param firstframe First frame of experiment
 * \param lastframe Last frame in experiment
 * \return Number of missing frames
 */
int check_missing_frames(vector <info> & filelist, map<int,bool>& frames, list <frameline>& framelist, const int& firstframe, const int& lastframe);

//==========================================================
/**\fn void check_overlap(vector <info>& filelist)
 * \brief  check the limits of all files and corrects them if a pair of files are overlapping
 * \param filelist 
 */
void check_overlap(vector <info>& filelist);

//==========================================================
/**\fn void copy_frames_from_input(ofstream& h, list <timestamp>& frames_read, map <int, string>& early_frames, map<int, bool>& frames_written, timestamp& last_written, const timestamp& my_file, timestamp& current, bool next, const string& s2)
 * \brief  
 * \param 
 */
void copy_frames_from_input(ofstream& h, list <timestamp>& frames_read, map <int, string>& early_frames, map<int, bool>& frames_written, timestamp& last_written, const timestamp& my_file, timestamp& current, bool next, const string& s2);

//==========================================================
/**\fn void read_file_to_correct(ifstream& f, ofstream& h, info& my_filelist, list <frameline*>& temp, map<int, bool>& frame_written, map <int, string>& early_frames, map <int, bool>& ordered_frames, map <int,bool>&  frame_problem, list <timestamp>& frames_read, timestamp& last_written, timestamp& current)
 * \brief Writes correct outputfiles in which all frames are ordered in increasing number, are complete and there are no missing frames
 * \param f Input raw file that was already read, now second reading
 * \param h Output :corrected file that will be generated (frames ordered, merged and no missing frames)
 * \param my_filelist Info of files to read (start and end frame, etc)
 * \param temp Temporary list to hold pointers to frames that need correction for a given file
 * \param frame_written All frames that have been written
 * \param early_frames Frames that are ordered locally but should be in a different file
 * \param ordered_frames All frames that are locally ordered and complete
 * \param frame_problem All frames that are unordered and belong to this input file (bigger or equal to the startframe and smaller ot equal to the endframe of the input raw file limits)
 * \param frames_read All frames that have been read from all raw input files
 * \param last_written Last timestamp (frame, unix time) scanned
 * \param current Next frame expected from input, i.e. should be first frame in file
 */
void read_file_to_correct(ifstream& f, ofstream& h, info& my_filelist, list <frameline*>& temp, map<int, bool>& frames_written, map <int, string>& early_frames, map <int, bool>& ordered_frames, map <int,bool>&  frame_problem, list <timestamp>& frames_read, timestamp& last_written, timestamp& current);

//==========================================================
/**\fn void copy_frames_from_list(ofstream& h, list <frameline*> temp, timestamp& last_written, timestamp& current, map<int, bool>& frame_written, map <int, string>& early_frames, list <timestamp>& frames_read)
 * \brief  
 * \param 
 */
void copy_frames_from_list(ofstream& h, list <frameline*> temp, timestamp& last_written, timestamp& current, map<int, bool>& frame_written, map <int, string>& early_frames, list <timestamp>& frames_read);

//==========================================================
/**\fn void correct_files(vector <info>& filelist, map <int, bool>& ordered_frames, list <timestamp>& frames_read, list <frameline>& framelist, vector <string>& filelist2)
 * \brief  
 * \param 
 */
void correct_files(vector <info>& filelist, map <int, bool>& ordered_frames, list <timestamp>& frames_read, list <frameline>& framelist, vector <string>& filelist2);

