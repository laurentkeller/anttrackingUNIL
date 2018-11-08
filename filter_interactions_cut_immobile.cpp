/*
 *  filter_interactions.cpp
 *  --> takes as input a list of interactions and checks whether subsequent interactions 
 *		between the same pair of ants belong to the same event or 2 distinct events 
 *	 ! expects time in input file in addition to frame number   
 * 
 *
 *  Created by Danielle Mersch on 23/5/11.
 *  Modified by Nathalie Stroeymeyt (NS) in June 2016.
 *  Copyright 2011 __UNIL__. All rights reserved.
 *
 */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <getopt.h>

#include "trackcvt.h"
#include "exception.h"
#include "tags3.h"
#include "utils.h"

using namespace std;

// number of frames between two subsequent interactions,
//if the delay is shorter, then we test whether the interactions constitute a single event
//const int F_TH = 20;  // threshold for temporary close interaction, they are tested whether they are part of the same event
//const int D_TH = 120; //minimal duration of events that are for the quality of interactions (-->calculate ratio of detection vs missed detections)
//const double I_TH = 0.65; // quality ratio  (number of detection vs number of missing detections ) if below -> a cout message is displayed for the event

//const int X_TH = 25; // maximal variation in x coordinate that is accepted for interactions belonging to the same event
//const int F_MAX = 30000;	/// maximal duration in frames of interaction, if longer than is considered to be
const int F_MIN = 0; /// minmal duration in frames of interaction, if shorter than this it is not included in the filtered file

/// structure of an interaction
struct data{
  double time_start;
  double time_stop;
  uint32_t frame_start;		// frame of interaction
  //==== NS code start =====
  uint32_t frame_ref_awakeness;		// frame used for calculating asleep duration
  //==== NS code end =====
  uint32_t frame_stop;		// frame of end of interaction (filled only during filtering)
  uint16_t box;
  //coor ant 1
  uint16_t x1;
  uint16_t y1;
  int16_t a1;
  //==== NS code start =====
  bool awake1;
  //==== NS code end =====
  // coor ant 2
  uint16_t x2;
  uint16_t y2;
  int16_t a2;
  //==== NS code start =====
  bool awake2;
  //==== NS code end =====
  int det;//number of frames in which the interaction was detected
  int direction; // direction of interaction (1: ant1 interacts, 2: ant2 interacts, 3:both ants interact);
  //==== NS code start =====
  char from;//point of interacting ant: A antenna, M mandibles, G gaster tip
  char to;//region of touched ant: F front, B back
  //==== NS code end =====
};

struct event{
  uint16_t tag1;
  uint16_t tag2;
  data d;
  char s;		// state of interaction: long, blinking
};

typedef vector <data> interactions;
typedef vector <vector <int> > matrice;

// convert a tag in the corresponding index of the tag_list table
bool find_idx(const int tag, int& idx){
  int i(0);
  do{
    if (tag == tag_list[i]){
      idx = i;
      return true;
    }
    i++;
  }while(idx == -1 && i<tag_count);
  return false;
}

// function to compare elements of vector 
bool cmp(const event& a, const event& b){
  return a.d.frame_start < b.d.frame_start;
}

// ==============================================================================
/** void read_interaction_file(char* filename, vector <vector <interactions> >& i_table)
 * * * * \brief Opens input file with list of interactions (expected format is frame,box,IDant1,IDant2,x1,y1,a1,x2,y2,a2)
 * * * *		reads them into a 3 dimensional vector
 * * * * \param filename Name of the input file to read
 * * * * \param i_table Table of interactions to fill
 * * * */
void read_interaction_file(string filename, vector <vector <interactions> >& i_table){
  
  ifstream f;
  f.open(filename.c_str());
  if (!f.is_open()){
    throw Exception(CANNOT_OPEN_FILE, (string) filename);
  }
  
  while (!f.eof()){
    string s;
    getline(f,s);
    stringstream ss;
    ss.str(s);
    data temp;
    memset(&temp, 0, sizeof(temp));
    temp.frame_stop = 0;
    //==== NS code start =====
    temp.awake1=0;
    temp.awake2=0;
    //==== NS code end =====
    ss>>temp.time_start;
    if (!ss.fail()){
      ss.ignore(1,',');
      ss>>temp.frame_start;
      ss.ignore(1,',');
      ss>>temp.box;
      ss.ignore(1,',');
      int tag1;
      ss>>tag1;
      ss.ignore(1,',');
      int tag2;
      ss>>tag2;
      ss.ignore(1,',');
      ss>>temp.x1;
      ss.ignore(1,',');
      ss>>temp.y1;
      ss.ignore(1,',');
      ss>>temp.a1;
      ss.ignore(1,',');
      ss>>temp.x2;
      ss.ignore(1,',');
      ss>>temp.y2;
      ss.ignore(1,',');
      ss>>temp.a2;
      ss.ignore(1,',');
      ss>>temp.direction;
      //==== NS code start =====
      ss.ignore(1,',');
      ss>>temp.from;
      ss.ignore(1,',');
      ss>>temp.to;
      //==== NS code end =====
      temp.det = 1;
      
      if (temp.x2 == 0){
        cout<<"--->error:"<<endl;
        cout<<"temp: "<<temp.x1<<","<<temp.y1<<" & "<<temp.x2<<","<<temp.y2<<endl;
        cout<<"read: "<<s<<endl;
        return;
      }
      
      // find index of each tag
      int idx1 (-1); 
      int idx2 (-1);
      if (!find_idx(tag1, idx1)){
        stringstream ss;
        ss<<tag1;
        string info = ss.str();
        throw Exception(TAG_NOT_FOUND, info);
      }
      if (!find_idx(tag2, idx2)){
        stringstream ss;
        ss<<tag2;
        string info = ss.str();
        throw Exception(TAG_NOT_FOUND, info);
      }
      //==== NS code start =====
      temp.frame_ref_awakeness = temp.frame_start;
      //==== NS code end =====
      
      // add interaction to list (matrix is symmetric)
      i_table[idx1][idx2].push_back(temp);
      i_table[idx2][idx1].push_back(temp);
    }
  }	
  f.close();
}

// ==============================================================================
// takes a vector of data (position and orientation of 2 ants) and calculates the average position and orientation and direction
bool average_position(data& tmp, const vector <data>& inter, bool length, int F_MAX){
  int ctr (0);
  double x1 (0), y1 (0), x2(0), y2(0), d(0);
  vector <double> a1;
  vector <double> a2;
  if (inter.size() < F_MAX){
    length = false;
  }
  if (!length){ // if interaction length shorter than F_MAX, use all positions
    
    for (int j(0); j < inter.size(); j++){
      x1 += inter.at(j).x1;
      x2 += inter.at(j).x2;
      y1 += inter.at(j).y1;
      y2 += inter.at(j).y2;
      a1.push_back((double)inter.at(j).a1/100);
      a2.push_back((double)inter.at(j).a2/100);
      d += inter.at(j).direction;
      ctr++;
      
    }
  }else{ // if interaction length longer than F_MAX, only use position in the first F_MAX frames
    // determine how many positions are known and how many have to be read
    int known = inter.size();
    int j = 0;
    int limit = inter.at(j).frame_start + F_MAX;
    do{
      x1 += inter.at(j).x1;
      x2 += inter.at(j).x2;
      y1 += inter.at(j).y1;
      y2 += inter.at(j).y2;
      a1.push_back((double)inter.at(j).a1/100);
      a2.push_back((double)inter.at(j).a2/100);
      d += inter.at(j).direction;
      ctr++;
      j++;
    }while(j < known && inter.at(j).frame_stop < limit);
  }
  tmp.x1 = x1/ctr;
  tmp.y1 = y1/ctr;
  tmp.x2 = x2/ctr;
  tmp.y2 = y2/ctr;
  tmp.a1 = average_direction(a1) * 180/M_PI *100;
  tmp.a2 = average_direction(a2) * 180/M_PI *100;
  tmp.direction = d;
  tmp.det = ctr;
  
  if (tmp.x1 > IMAGE_WIDTH || tmp.x2 > IMAGE_WIDTH || tmp.y1 > IMAGE_HEIGHT || tmp.y2 > IMAGE_HEIGHT){
    cout<<"Problem with average position. "<<endl;
    cout<<"ctr: "<<ctr<<endl;
    cout<<"inter.size: "<<inter.size()<<endl;
    cout<<"length: "<<length<<endl;
    return false;
  }
  return true;
}
//==== NS code start =====
void write_interaction(int i, vector <event>& event_table, int t1, int t2, vector <vector <interactions> >& i_table){
  event e;
  e.tag1 = tag_list[t1];
  e.tag2 = tag_list[t2];
  e.d = i_table[t1][t2][i];
  if (i==0){
    e.d.frame_stop = e.d.frame_start;
    e.d.time_stop = e.d.time_start;
  }
  event_table.push_back(e);
  
}

void process_single_frame_interaction(int i, int t1, int t2, vector <vector <interactions> >& i_table){
  i_table[t1][t2][i].frame_stop = i_table[t1][t2][i].frame_start;
  // update time
  i_table[t1][t2][i].time_stop = i_table[t1][t2][i].time_start;
  i_table[t1][t2][i].det = 1;
}

void process_first_two_interactions(int i,data temp1,data temp2, int t1, int t2, vector <vector <interactions> >& i_table,vector <data>& inter){
  // update the start and stop frame in the list i_table
  int tmp = temp2.frame_start;
  i_table[t1][t2][i+1].frame_start = temp1.frame_start;
  i_table[t1][t2][i+1].frame_stop = tmp;
  i_table[t1][t2][i].frame_start = 0;
  // update also time
  double tmp2 = temp2.time_start;
  i_table[t1][t2][i+1].time_start = temp1.time_start;
  i_table[t1][t2][i+1].time_stop = tmp2;
  
  inter.push_back(temp1);
}

void truncate_interaction(bool& length,int i, int t1, int t2, vector <vector <interactions> >& i_table, int F_MAX){
  if (i_table[t1][t2][i].frame_stop - i_table[t1][t2][i].frame_ref_awakeness + 1 > F_MAX){
    i_table[t1][t2][i].frame_stop = i_table[t1][t2][i].frame_ref_awakeness + F_MAX -1;
    // update time
    i_table[t1][t2][i].time_stop = i_table[t1][t2][i].time_start + ( (i_table[t1][t2][i].frame_stop - i_table[t1][t2][i].frame_start + 1) /2);
    length = true;
  }
}

void test_awakeness(int i,int t1,int t2, vector <vector <interactions> >& i_table, int X_min,data temp1, data& temp2){
  if ( ( (temp2.x1 - temp1.x1) > X_min) || ( (temp2.y1 - temp1.y1) > X_min) ) {// if ant1 awake update awake1
    i_table[t1][t2][i+1].awake1=1;
    temp2.awake1=1;	
  }
  
  if ( ( (temp2.x2 - temp1.x2) > X_min) || ( (temp2.y2 - temp1.y2) > X_min) ) {// if ant2 awake update awake2
    i_table[t1][t2][i+1].awake2=1;
    temp2.awake2=1;	
  }
  
  if (i_table[t1][t2][i+1].awake1||i_table[t1][t2][i+1].awake2){//if either ant awake update frame_ref_awakeness
    i_table[t1][t2][i+1].frame_ref_awakeness = i_table[t1][t2][i+1].frame_start;
    temp2.frame_ref_awakeness = i_table[t1][t2][i+1].frame_start;
  }else{//else copy frame_ref_awakeness of interaction i into frame_ref_awakeness of interaction i+1
    i_table[t1][t2][i+1].frame_ref_awakeness = i_table[t1][t2][i].frame_ref_awakeness;	
    temp2.frame_ref_awakeness = i_table[t1][t2][i].frame_ref_awakeness;
  }
}
//==== NS code end =====
// ==============================================================================


//								MAIN
// ==============================================================================
// usage: filter_interactions -i input.txt -o output.txt


int main (int argc, char* argv[]){
  
  try{
    
    opterr = 0;
    char option; 
    string input = "";
    string output = "";
    int F_TH = -1; // threshold for temporary close interaction, they are tested whether they are part of the same event 
    int F_MAX = 0; /// maximal duration in frames of interaction, if longer than is considered to be
    int X_TH = -1; // maximal variation in x and y coordinate that is accepted for interactions belonging to the same event
    //==== NS code start =====
    int X_min = -1; // minimum variation in x and y coordinate that is necessary for the ant to be declared awake
    //==== NS code end =====
    
    // extract options
    while((option = getopt(argc, argv, ":i:o:t:m:d:a:")) !=-1){
      switch(option){
      case 'i':
        input = (string)optarg;
        break;
      case 't':
        F_TH = atoi(optarg);
        break;
      case 'm':
        F_MAX = atoi(optarg);
        break;
      case 'o':
        output = (string)optarg;
        break;
      case 'd':
        X_TH = atoi(optarg);
        break;
      case 'a':
        X_min = atoi(optarg);
        break;
        
      case '?':
        {
          string info = "-" + string(1, optopt);
          throw Exception(UNKNOWN_OPTION, info);
        }
      case ':':
        {
          string info = "-" + string(1, optopt);
          throw Exception(ARGUMENT_MISSING, info); 
        }
      }
    }
    
    if (argc < 7){
      string info = (string)argv[0] + " -i input.txt -o output.txt -t timethreshold(frames)  -m max_duration(frames) -d distance_threshold(pixels) -a min_awake_distance(pixels) input.tags";
      throw Exception(USE, info);
    }
    
    // check whether options have a parameter
    if (input == ""){
      string info = "Parameter of option -i is missing.";
      throw Exception(PARAMETER_ERROR, info);
    }
    if (output == ""){
      string info = "Parameter of option -o is missing.";
      throw Exception(PARAMETER_ERROR, info);
    }
    if (F_TH == -1){
      throw Exception(PARAMETER_ERROR,"Parameter of option -t is missing.");
    }
    if (F_TH < 0){
      throw Exception(PARAMETER_ERROR,"Time threshold need to be positive.");
    }
    if (F_MAX == 0){
      throw Exception(PARAMETER_ERROR,"Parameter of option -m is missing.");
    }
    if (F_MAX < 1){
      throw Exception(PARAMETER_ERROR,"Maximal duration of interaction need to be positive.");
    }
    if (X_TH == -1){
      throw Exception(PARAMETER_ERROR,"Parameter of option -d is missing.");
    }
    if (X_TH < 0){
      throw Exception(PARAMETER_ERROR,"Distance threshold need to be non-negative.");
    }
    if (X_min == -1){
      throw Exception(PARAMETER_ERROR,"Parameter of option -a is missing.");
    }
    if (X_min < 0){
      throw Exception(PARAMETER_ERROR,"Min distance awake need to be non-negative.");
    }
    
    TagsFile tgs;
    tgs.read_file(argv[optind]);
    
    // test whether output exists already
    ifstream f; 
    f.open(output.c_str());
    if (f.is_open()){
      f.close();
      throw Exception (OUTPUT_EXISTS, output);
    }
    
    // create table with interactions and inilialize with empty vectors of type data
    vector <vector <interactions> > i_table;
    i_table.resize(tag_count);
    for (int i(0); i < tag_count; i++) {
      i_table[i].resize(tag_count);
    }
    
    // read data from file and store in i_table
    cout<<"reading data from input..."<<endl;
    read_interaction_file(input, i_table);
    
    vector <event> event_table;
    int singleinteraction (0); // number of ant pairs that had a single 1 frame interaction
    
    // verify interactions in i_table
    cout<<"filtering interactions..."<<endl;
    for (int t1 (0); t1<tag_count-1; t1++){
      for(int t2 (t1+1); t2 < tag_count; t2++){
        
        // check whether there are interactions for a given pair of ants
        if (!i_table[t1][t2].empty()){
          // if there is more than one interaction between the 2 ants, check whether several interactions are part of same interaction event
          if (i_table[t1][t2].size() > 1){
            
            
            vector <data> inter;
            for (int i(0); i<i_table[t1][t2].size()-1; i++){
              
              // read element and compare with next one
              data temp1 = i_table[t1][t2][i];
              data temp2 = i_table[t1][t2][i+1];
              
              // if 2 subsequent interactions are temporally close (nb of frames between them < F_TH), test if same event
              // if it is the first interaction of the event (frame_stop == 0) check distance between start frames, otherwise check distance between stop of first and start of second
              if ((temp1.frame_stop == 0 && temp2.frame_start - temp1.frame_start < F_TH ) || temp2.frame_start - temp1.frame_stop < F_TH){
                
                // check positions and angles of interacting ants,
                // if positions are close, interactions are part of same event
                if ((temp2.x1 - temp1.x1 < X_TH) && (temp2.y1 - temp1.y1 < X_TH) && (temp2.x2 - temp1.x2 < X_TH) && (temp2.y2 - temp1.y2 < X_TH)){
                  
                  //==== NS code start =====           
                  // check if awakeness. 
                  // same event if both ants asleep; or if ant awake and time since last awakeness below F_MAX
                  test_awakeness(i,t1,t2,i_table,X_min,temp1,temp2);
                  if ( (!temp2.awake1 && !temp2.awake2 ) || ( ( temp2.awake1||temp2.awake2 ) && (( temp2.frame_start-temp1.frame_ref_awakeness) < F_MAX ))){
                    //==== NS code end =====
                    // use first 2 interactions of an event: set start frame of first interaction (the one read into temp1) to zero to mark as read and, and set start frame of second interaction in i_table to start frame of first interaction 
                    if (temp1.frame_stop == 0){
                      // update the start and stop frame in the list i_table
                      process_first_two_interactions(i,temp1,temp2,t1,t2,i_table,inter);
                      
                      // third or further interaction of event
                    }else{
                      // test if there are frames missing between the subsequent interactions detected, if so count how many
                      i_table[t1][t2][i+1].frame_stop = temp2.frame_start;
                      i_table[t1][t2][i+1].frame_start = temp1.frame_start;
                      i_table[t1][t2][i].frame_start = 0;
                      // update also time
                      i_table[t1][t2][i+1].time_stop = temp2.time_start;
                      i_table[t1][t2][i+1].time_start = temp1.time_start;
                    }
                    
                    //add interaction to table for coordinate calculations
                    inter.push_back(temp2);
                    
                    
                    // if temp2 is last interaction of event and this is the last event for this ant pair
                    if (i+1 == i_table[t1][t2].size()-1){
                      
                      // check if previous event is of reasonable duration (less than F_MAX frames), if not cut it at F_MAX
                      bool length (false);
                      truncate_interaction(length,i+1,t1,t2,i_table, F_MAX);
                      if (! average_position( i_table[t1][t2][i+1], inter, length, i_table[t1][t2][i+1].frame_stop-i_table[t1][t2][i+1].frame_start+1)){
                        return 1;
                      }
                      // add event
                      write_interaction(i+1,event_table,t1,t2,i_table);
                    }
                    
                  }else{ // interactions are part of distinct events because distance ant was asleep for too long
                    // restore frame awakeness
                    //==== NS code start =====
                    temp2.frame_ref_awakeness = temp2.frame_start;
                    i_table[t1][t2][i+1].frame_ref_awakeness = i_table[t1][t2][i+1].frame_start; 
                    //==== NS code end =====
                    // if previous event was only 1 frame long set stop frame to start frame
                    if (i_table[t1][t2][i].frame_stop == 0){
                      process_single_frame_interaction(i, t1, t2, i_table);
                    }
                    
                    // check if previous event is of reasonable duration (less than F_MAX frames), if not cut it at F_MAX
                    bool length (false);
                    truncate_interaction(length,i,t1,t2,i_table, F_MAX);
                    // if the previous event had several interactions, then calculate average positions of ants
                    if (!inter.empty()){
                      if (! average_position(i_table[t1][t2][i], inter, length, i_table[t1][t2][i].frame_stop-i_table[t1][t2][i].frame_start+1)){
                        return 1;
                      }
                    }                   
                    // add event to table
                    write_interaction(i,event_table,t1,t2,i_table);
                    inter.clear();
                    // if temp2 is last interaction for this ant pair, add it as well to event_table
                    if (i+1 == i_table[t1][t2].size()-1){
                      process_single_frame_interaction(i+1, t1, t2, i_table);
                      write_interaction(i+1,event_table,t1,t2,i_table);
                    }
                  }
                  
                  // interactions are part of distinct events beacuse distance moved is too large
                }else{
                  // if previous event was only 1 frame long set stop frame to start frame
                  if (i_table[t1][t2][i].frame_stop == 0){
                    process_single_frame_interaction(i, t1, t2, i_table);
                  }
                  
                  // check if previous event is of reasonable duration (less than F_MAX frames), if not cut it at F_MAX
                  bool length (false);
                  truncate_interaction(length,i,t1,t2,i_table, F_MAX);
                  
                  // if the previous event had several interactions, then calculate average positions of ants
                  if (!inter.empty()){
                    if (! average_position(i_table[t1][t2][i], inter, length, i_table[t1][t2][i].frame_stop-i_table[t1][t2][i].frame_start+1)){
                      return 1;
                    }
                  }                   
                  // add event to table
                  write_interaction(i,event_table,t1,t2,i_table);
                  inter.clear();
                  // if temp2 is last interaction for this ant pair, add it as well to event_table
                  if (i+1 == i_table[t1][t2].size()-1){
                    process_single_frame_interaction(i+1, t1, t2, i_table);
                    write_interaction(i+1,event_table,t1,t2,i_table);
                  }
                }
                
                
                // 2 subsequent interaction are NOT temporally close --> distinct events
              }else{
                if (i_table[t1][t2][i].frame_stop == 0){
                  process_single_frame_interaction(i, t1, t2, i_table);
                }
                bool length (false);
                truncate_interaction(length,i,t1,t2,i_table, F_MAX);
                // if the previous event had several interactions, then calculate average positions of ants
                if (!inter.empty()){
                  if (!average_position(i_table[t1][t2][i], inter, length, i_table[t1][t2][i].frame_stop-i_table[t1][t2][i].frame_start+1)){
                    return 1;
                  }
                }
                write_interaction(i,event_table,t1,t2,i_table);
                
                // if temp2 is last interaction for this ant pair, add it as well to event_table
                if (i+1 == i_table[t1][t2].size()-1){
                  process_single_frame_interaction(i+1, t1, t2, i_table);
                  write_interaction(i+1,event_table,t1,t2,i_table);
                }							
                inter.clear();
              }
            } // end for i_table
            
          }else{  // only 1 interaction between these 2 ants
            write_interaction(0,event_table,t1,t2,i_table);
            singleinteraction++;
          }
        }
      }// end for t2
    }// end for t1
    
    cout<<"There were "<<singleinteraction<<" antpairs that interaction only once for 1 frame. "<<endl; 
    
    // sorting the events
    cout<<"sorting the events temporally ..."<<endl;
    sort(event_table.begin(), event_table.end(), cmp);
    
    
    // writing outfile
    ofstream g;
    g.open(output.c_str());
    if (!g.is_open()){
      throw Exception(CANNOT_OPEN_FILE, output);
    }
    g<<"#Tag1,Tag2,Startframe,Stopframe,Starttime,Stoptime,Box,Xcoor1,Ycoor1,Angle1,Xcoor2,Ycorr2,Angle2,Direction,Detections"<<endl;
    
    cout<<"writing sorted events to file..."<<endl;
    for (int i(0); i< event_table.size(); i++){
      if ((event_table[i].d.frame_stop - event_table[i].d.frame_start +1)  > F_MIN){
        g<<event_table[i].tag1<<","<<event_table[i].tag2<<","<<event_table[i].d.frame_start<<","<<event_table[i].d.frame_stop<<",";
        g.precision(12);
        g<<event_table[i].d.time_start<<",";
        g.precision(12);
        g<<event_table[i].d.time_stop<<","<<event_table[i].d.box<<",";
        g<<event_table[i].d.x1<<","<<event_table[i].d.y1<<","<<event_table[i].d.a1<<",";
        g<<event_table[i].d.x2<<","<<event_table[i].d.y2<<","<<event_table[i].d.a2<<",";
        g<<event_table[i].d.direction<<","<<event_table[i].d.det;
        //g<<i_table[idx1][idx2].push_back(temp);
        g<<endl;
      }
    }
    
    g.close();
    
    return 0; 
  }catch(Exception e){
    return 1; 
  }
  
}



