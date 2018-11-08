/*
 *  heatmap.cpp
 *  make heatmap for individual ant
 *	also write text output file for input to matlab
 *  Created by Danielle Mersch on 11/21/10.
 *  Copyright 2010 __UNIL__. All rights reserved.
 *
 */



#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <getopt.h>

#include <libants/datfile.h>
#include <libants/exception.h>
#include <libants/tags3.h>
#include <libants/colormap.h>
#include <libants/histogram.h>
#include <libants/statistics.h>
#include <libants/utils.h>
#include <libants/trackcvt.h>

using namespace std;

const int START = 1167609600; // UNIX time of 1st january 2007 00:00:00, no data exists from before
const int HEATMAP_REDUCTION = 5;
const int HEATMAP_X = IMAGE_WIDTH / HEATMAP_REDUCTION;
const int HEATMAP_Y = IMAGE_HEIGHT / HEATMAP_REDUCTION;
const int HEATMAP_SIZE = HEATMAP_X * HEATMAP_Y;


struct statistics{
  double mean;
  double stdev;
  int max;
  statistics(): mean(0), stdev(0.0), max(0){};
  statistics(double m, double s, int x): mean(m), stdev(s), max(x) {};
};


//================================================================================
/**\fn int find_max(int* heatmap)
* \brief Finds the maximum in the one dimensional table
* \param heatmap One dimentional table of integrers
* \return The maximum of the table
*/
int find_max(int* heatmap){
  int max(0);
  for (int i(0); i< HEATMAP_SIZE; i++){
    if (heatmap[i]> max){
      max = heatmap[i];
    }
  }
  return max;
}


//================================================================================
bool write_textfile(int* heatmap, string textfile){
  ofstream f;
  f.open(textfile.c_str());
  if (!f.is_open()){
    return false;
  }
  cout<<"writing to file..."<<endl;
  for (int i(0); i< HEATMAP_SIZE; i++){
    if (heatmap[i]<0) { // no control on upper values possible
      cerr << "No, this shouldn't happen! heatmap contains " << heatmap[i] << endl;
    }
    int x = i % HEATMAP_X;
    int y = i / HEATMAP_X;
    f<<x<<","<<y<<","<<heatmap[i]<<endl;
    
  }
  cout<<"...finished"<<endl;
  f.close();
  return true;
}


//================================================================================
int main (int argc, char* argv[]){
  
  try {
    opterr = 0;
    
    string datfile = "";
    string tagsfile = "";
    
    int box = 0; // ID of box
    double start = 0.0;	// start frame (unix time)
    int duration = 0; // duration of interval in seconds
    int nb_intervals = 1; // number of intervals for which heatmaps are generated, default = 1
    string colname = "group";	// name of column to consider
    
    vector <int> ants; // IDs of tags, if option is present the heatmap is calculated for specific ants
    int ctr_ants= 0;	// counts number of ants
    bool colony = false; // true if the heatmap is done for all ants
    vector <string> groups; //ID of group, should be identical in tagsfile in column group
    int ctr_groups = 0; ///< number of groups
    string matlabfile = ""; // filename for textfiles (as input for matlab)
    
    // process arguments
    char option;
    while ((option = getopt(argc, argv, ":t:i:b:s:d:a:n:cg:h:m:")) != -1) {
      switch (option)
      {
      case '?':
        cerr<<"Unknown option"<<endl;
        return 1;
      case 'i':
        datfile = (string)optarg;
        break;
      case 't':
        tagsfile = (string)optarg;
        break;
      case 'b':
        box = atoi(optarg);
        break;
      case 'd':
        duration = atoi(optarg);
        break;
      case 's':
        start = atof(optarg);
        break;
      case 'n':
        nb_intervals = atoi(optarg);
        break;
      case 'c':
        colony = true;
        break;
      case 'h':
        colname = (string) optarg;
        break;
      case 'm':
        matlabfile = (string) optarg;
        break;
      case 'a': 
        {
          int ant = atoi(optarg);
          ants.push_back(ant);
          ctr_ants++;
          break;
        }
      case 'g':
        {	
          string group = (string) optarg;
          groups.push_back(group);
          ctr_groups++;
          break;
        }
      case ':':
        {
          string info = "-" + string(1, optopt);
          throw Exception(ARGUMENT_MISSING, info); 
        }
      }
    }
    
    
    // test if enough arguments
    if (argc < 7){
      string info = (string) argv[0] + " -i input.dat -t intput.tags -b boxID -s unixstart(sec) -d interval(sec) -n nb_intervals(default=1) [-h columnname=group] [-a antID | -g groupID | -c] -m outputname";
      throw Exception (USE, info);
    }
    
    // check parameters
    if (datfile == ""){
      throw Exception(PARAMETER_ERROR, "The filename of option -i is missing.");
    }
    if (tagsfile == ""){
      throw Exception(PARAMETER_ERROR, "The filename of option -t is missing.");
    }
    if (!is_valid_ID(box, box_list,box_count)){
      string b = (string) argv[3];
      throw Exception (BOX_NOT_FOUND,b);
    }
    
    if(start < START){
      throw Exception(PARAMETER_ERROR, "The parameter of option -s need to be a unixtime after " + to_string(START) + ".");
    }
    if (duration < 1){
      throw Exception(PARAMETER_ERROR, "The parameter of the option -n need to be a positiv integrer.");
    }
    if(colname == ""){
      throw Exception(PARAMETER_ERROR, "The filename of option -h is empty.");
    }
    
    if (!colony && ctr_groups == 0 && ctr_ants == 0){
      throw Exception (OPTION_MISSING, "An option specifiy for whom the neatmap should be generated is missing. Possible options are -a antID , -g groupID and -c.");
    }
    
    DatFile dat;
    dat.open(datfile, 0);
    TagsFile tgs;
    tgs.read_file(tagsfile.c_str());
    
    // check if start time exists in file
    if (!dat.is_valid_time(start)){
      string info = "The start time is not in the file.";
      throw Exception (PARAMETER_ERROR, info);
    }
    // check if the interval is feasible in the file
    if (!dat.is_valid_time(start + duration)){
      string info = "When starting at " + to_string(start) + " the duration of the first interval exceeds the duration of the file.";
      throw Exception (PARAMETER_ERROR, info);
    }
    
    // check if the number of intervals are feasible in the file
    int nb_i=1;
    int temp = 0;
    bool state_i = true;
    while (nb_i <= nb_intervals && state_i){
      if (!dat.is_valid_time(start+duration*nb_i)){
        state_i = false;
        temp = nb_i;
      }else{
        temp = nb_i;
        nb_i++;
      }
    }
    // if not feasible, we will calulate for the number of intervals feasible
    if (temp < nb_intervals){
      cerr<<"Warning: the heatmap can only be calculated for "<<temp<<" intervals instead of "<<nb_intervals<<" requested."<<endl;
      nb_intervals = temp;
    }
    
    // identify for whom to generate the heatmaps, and create corresponding filenames, and for groups create a table of bool with which tags to include
    vector <int> idx_ants;
    vector <string> antfile;
    bool ant_state[tag_count];
    memset(ant_state, 0, sizeof(ant_state));
    if (ctr_ants > 0){
      for (int i(0); i< ctr_ants; i++){
        int idx = get_idx(ants[i], tag_list, tag_count);
        if (idx != -1){
          idx_ants.push_back(idx);
          ant_state[idx] = true;
        }else{
          throw Exception (TAG_NOT_FOUND, to_string(ants[i]));
        }
      }
    }
    if (ctr_groups > 0){
      for (int i(0); i < ctr_groups; i++){
        if(!tgs.content_exists(colname, groups[i])){
          throw Exception (PARAMETER_ERROR, "There are no ants in the group " + groups[i] + ".");
        }
      }
    }
    vector <string> groupfile;
    vector <vector<bool> > group_states;
    for(int i(0); i< ctr_groups; i++){
      vector <bool> state;
      state.resize(tag_count);
      for(int t(0); t<tag_count; t++){
        string tmp = "";
        if (tgs.get_state(t)){
          if (tgs.get_content(t, colname, tmp)){
            if (tmp == groups[i]){
              state[t]=true;
            }
          }
        }
      }
      group_states.push_back(state);
    }
    
    // go to start time
    dat.go_to_time(start);
    cout << "Current time in file is " ;
    cout.precision(12);
    cout<< dat.get_current_time() << ", start is " << start << endl;
    
    bool end = false;
    
    // create heatmap tables
    // generate heatmap raster for colony
    int* heatmap_col;
    bool col_data;
    if(colony){
      heatmap_col = new int [HEATMAP_SIZE];
      memset(heatmap_col, 0, sizeof(int) * HEATMAP_SIZE);
    }
    // generate heatmap for each tag
    vector <int*> heatmap_groups;
    vector <bool> groups_data;
    groups_data.resize(ctr_groups);
    for (int i(0); i< ctr_groups; i++){
      int* heatmap;
      heatmap = new int[HEATMAP_SIZE];
      memset(heatmap, 0, sizeof(int) * HEATMAP_SIZE);
      heatmap_groups.push_back(heatmap);
    }
    
    // generate heatmap for each tag
    vector <int*> heatmap_ants;
    vector <bool> ants_data;
    ants_data.resize(ctr_ants);
    for (int i(0); i< ctr_ants; i++){
      int* heatmap;
      heatmap = new int [HEATMAP_SIZE];
      memset(heatmap, 0, sizeof(int) * HEATMAP_SIZE);
      heatmap_ants.push_back(heatmap);
    }
    
    cout<<"reading dat file..."<<endl;
    
    while(!dat.eof() && !end){
      
      for (int fi(1); fi <=nb_intervals; fi++){
        
        cout<<"interval: "<<fi<<endl;
        // reset heatmaps for next interval
        for (int i(0); i< ctr_ants; i++){
          memset(heatmap_ants[i], 0, sizeof(int) * HEATMAP_SIZE);
          ants_data.clear();
          ants_data.resize(ctr_ants);
        }
        for (int i(0); i< ctr_groups; i++){
          memset(heatmap_groups[i], 0, sizeof(int) * HEATMAP_SIZE);
          groups_data.clear();
          groups_data.resize(ctr_groups);
        }
        if(colony){
          memset(heatmap_col, 0, sizeof(int) * HEATMAP_SIZE);
          col_data = false;
        }
        
        // read value of interval and fill heatmaps
        double limit = start + duration * fi;
        double t = dat.get_current_time();
        while (t < limit && !dat.eof()){
          framerec temp;
          dat.read_frame(temp);
          for (int i (0); i < tag_count; i++){
            if (tgs.get_state(i) && (tgs.get_death(i)== 0 || temp.frame < tgs.get_death(i))){
              // test if ant in a group
              bool in_group = false;
              int j(0);
              if (ctr_groups > 0){
                do{
                  if (group_states[j][i]){
                    in_group = true;
                  }
                  j++;
                }while(!in_group && j < ctr_groups);
              }
              
              if (colony || in_group  || (ctr_ants > 0 && ant_state[i])){
                if (temp.tags[i].id == box && temp.tags[i].x != -1){
                  int x = temp.tags[i].x/ HEATMAP_REDUCTION;
                  int y = temp.tags[i].y/ HEATMAP_REDUCTION;
                  // colony
                  if (colony){
                    heatmap_col[y * HEATMAP_X + x]++;
                    col_data = true;
                  }
                  // ant
                  if (ctr_ants > 0 && ant_state[i]){
                    int a = 0;
                    bool found(false);
                    do {
                      if (idx_ants[a] == i){
                        found = true;
                      }
                      a++;
                    }while ( !found && a < ctr_ants);
                    if (!found){
                      throw Exception(INTERNAL_ERROR, "Could not find correspondance between index of tag in tag_list and idx_ants.");
                    }
                    heatmap_ants[a-1][y * HEATMAP_X + x]++;
                    ants_data[a-1] = true;
                  }
                  // group
                  if (in_group){
                    heatmap_groups[j-1][y * HEATMAP_X + x]++;
                    groups_data[j-1] = true;
                  }
                }
              }
            }
          }
          t = temp.time;
        }
        
        
        if (col_data){
          cout<<"There is data for the colony."<<endl;
        }
        for (int i(0); i< ctr_groups; i++){
          if (groups_data[i]){
            cout<<"There is group data for group "<< groups[i]<<"."<<endl;
          }
        }
        for (int i(0); i< ctr_ants; i++){
          if (ants_data[i]){
            cout<<"There is ant data for group "<< ants[i]<<"."<<endl;
          }
        }
        
        
        
        // creating and writing output files for the interval 
        // colony
        if(colony){
          if (col_data){
            if (matlabfile !=""){
              stringstream ss;
              ss<< matlabfile << "_col_" << fi << ".txt";
              string textfile;
              ss>>textfile;
              if (!write_textfile(heatmap_col, textfile)){
                throw Exception(CANNOT_OPEN_FILE, textfile);
              }
            }
            
          }else{
            cerr<<"No data for colony."<<endl;
          }
        }
        //ants
        for (int i(0); i< ctr_ants; i++){
          
          if (ants_data[i]) {
            if (matlabfile !=""){
              stringstream ss;
              ss<< matlabfile << "_ant" << to_string(ants[i]) << fi << ".txt";
              string textfile;
              ss>>textfile;
              if (!write_textfile(heatmap_ants[i], textfile)){
                throw Exception(CANNOT_OPEN_FILE, textfile);
              }
            }
            
            
          }else{
            cerr<<"No data for ant "<<ants[i]<<"."<<endl; 
          }
        }
        //groups
        for (int i(0); i< ctr_groups; i++){
          
          if (groups_data[i]) {
            if (matlabfile !=""){
              stringstream ss;
              ss<< matlabfile << "_group" << groups[i] << fi << ".txt";
              string textfile;
              ss>>textfile;
              cout<<"writing "<<textfile<<endl;
              if (!write_textfile(heatmap_groups[i], textfile)){
                throw Exception(CANNOT_OPEN_FILE, textfile);
              }
            }
          }else{
            cerr<<"No data for group "<<groups[i]<<"."<<endl; 
          }
        }
      }	
      end = true;
    }
    
    dat.close();
    
    // delete heatmaps allocated with new
    for (int i(0); i< ctr_groups; i++){
      delete[] heatmap_groups[i];
    }
    for (int i(0); i< ctr_ants; i++){
      delete[] heatmap_ants[i];
    }
    if(colony){
      delete[] heatmap_col;
    }
    
    return 0;
  }
  catch (Exception e) {
  }
}