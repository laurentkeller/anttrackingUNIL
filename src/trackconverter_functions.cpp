

#include "trackconverter_functions.h"

//==========================================================
bool info::operator ==(const info& file_to_find){
	return (name == file_to_find.name);
}
//==========================================================
//brief Tests whether the id of the box is valid
bool is_valid_box(int box){
	for (int i(0); i < box_count; i++){
		if (box == box_list[i]){
			return true;
		}
	}
	return false;
}

//==========================================================
//brief Compares the frames of two framelines, determines which one has a smaller framenumber
bool operator<(const frameline& f1,const frameline& f2){
	if( f1.frame < f2.frame ) return true;
	return false;
}

//==========================================================
// Compares the frames of two timestamps, determines which one has a smaller framenumber
bool operator<(const timestamp& t1,const timestamp& t2){
	if( t1.frame < t2.frame ) return true;
	return false;
}

//==========================================================
//brief Merges part of incomplete frames
void merge_duplicates(list <frameline>& framelist,list <frameline>::iterator iter){
	
	for (iter = framelist.begin(); iter!= framelist.end(); iter++){
  //  cout<<"frame: "<<iter->frame<<endl;
  }
  //cout<<"framelist size start: "<<framelist.size()<<endl;

  iter = framelist.begin();
  do {
		const frameline& frame1 = *iter;
		
		if (iter == framelist.end() || framelist.empty()){
			return;
		}
		list <frameline>::iterator iter2 = iter;
		iter2++;
		const frameline& frame2 = *(iter2);
		
		if (frame1.frame == frame2.frame){ // if the frames are identical they are parts of an incomplete frame -> merge parts
			cout<<"-> 2 frame parts for "<<frame1.frame<<" and "<<frame2.frame<<endl;
			frameline f;
			f.frame = frame1.frame;  
			if (frame1.time < frame2.time){
				f.time = frame1.time;
			}else{
				f.time = frame2.time;
			}
			f.segments = frame1.segments + frame2.segments;
			f.tags = frame1.tags + frame2.tags;
			if (f.tags > 0){
				if (frame2.tags > 0){
					if (frame1.tags > 0){
						f.content = frame1.content + "," + frame2.content;
					}else{
						f.content = frame2.content;
					}
				}else{
					f.content = frame1.content;
				}
				
			}else{
				f.content ="";
			}
			f.type = 'I';
			// delete iterators of merged elements
			list <frameline>::iterator to_delete = iter;
      //cout<<"before incrementing iter: "<<iter->frame<<endl;
			iter++;
      //cout<<"after incrementing iter: "<<iter->frame<<endl;
			framelist.erase(to_delete); // erase incomplete part 
			to_delete = iter;
			iter++;
      //cout<<"after incrementing again: "<<iter->frame<<endl;
			framelist.erase(to_delete);
			framelist.insert(iter, f); // inserts merged frame before location loc
			iter--;
      //cout<<"framelist size: "<<framelist.size()<<endl;
      
			/*if (iter == framelist.begin()) {
				string info = "This should not happen... beginning of list (1a)";
				throw Exception(INTERNAL_ERROR, info);
			}*/
			//iter--;
		}
		// Do not inrecrement the iterator if we are already at the end of the list
		if (iter != (framelist.end())) {
			iter++;
		}
	} while (iter != (--framelist.end()) && iter != (framelist.end()));
}

//==========================================================
// Merges part of incomplete frames of all frames read from the input and verifies coherence of timestamp
void merge_duplicates(list <timestamp>& frames_read, list <timestamp>::iterator frame_iter){
	 frame_iter = frames_read.begin();
	 timestamp last;
	 timestamp current;
	int i (0);
	 do {
		 const timestamp& frame1 = *frame_iter;
		 
		 if (i ==0){
			 last.time = frame1.time - 0.5;
			 last.frame = frame1.frame - 1;
			 i++;
		 }
		 
		 current.time = frame1.time;
		 current.frame = frame1.frame;
		 
		 if (frame_iter == frames_read.end()){
			 return;
		 }
		 list <timestamp>::iterator frame_iter2 = frame_iter;
		 frame_iter2++;
		 const timestamp& frame2 = *(frame_iter2);
		 
		 if (frame1.frame == frame2.frame){ // if the frames are identical they are parts of an incomplete frame -> merge parts
			 //cout<<"-> 2 frame parts for "<<frame1.frame<<endl;
			 timestamp f;
			 f.frame = frame1.frame;  
			 if (frame1.time < frame2.time){
				 f.time = frame1.time;
			 }else{
				 f.time = frame2.time;
			 }
			 current.time = f.time;
			 current.frame = f.frame;
			
			 // delete iterators for merged elements
			 list <timestamp>::iterator to_delete = frame_iter;
			 frame_iter++;
			 frames_read.erase(to_delete); // erase incomplete part 
			 to_delete = frame_iter;
			 frame_iter++;
			 frames_read.erase(to_delete);
			 frames_read.insert(frame_iter, f); // inserts merged frame before location loc
			 frame_iter--;
			 /*if (frame_iter == frames_read.begin()) {
				 string info = "This should not happen... beginning of list (2)";
			 throw Exception(INTERNAL_ERROR, info);
			 }
			 frame_iter--;
			 */
		 }
		 
		 // check timestamp coherence, time should always be increasing (current > last)
		 if (current.time <= last.time && current.frame != last.frame){
			 cout<<"Warning conflicting timestamps. last time: ";
			 cout.precision(12);
			 cout<<last.time<<" ("<<last.frame<<"), current time: ";
			 cout.precision(12);
			 cout<<current.time<<" ("<<current.frame<<")"<<endl;
		 }
		 
		 // update last timestamp information
		 last.time = current.time;
		 last.frame = current.frame;
		 
		 // Do not inrecrement the iterator if we are already at the end of the list
		 if (frame_iter != (frames_read.end())) {
			 frame_iter++;
		 }
	} while (frame_iter != (--frames_read.end()) && frame_iter != (frames_read.end()));
	 
}

//==========================================================
// Rounds a real number (function is needed because Microsoft Visual Studio lacks the round() funtion from <cmath>)
inline double round2(double x){ 
	return floor(x + 0.5); 
}

//==========================================================
// Finds the index of a tag in the list defined in trackcvt.h
bool find_index(int tag, int& idx){
	int k(0);
	do {
		if (tag_list[k] == tag){
			idx =k;
			return true;
		}
		k++;
	}while(k<tag_count && idx == -1);
	return false;
}

//==========================================================
//Parses a line from the csv input file, stores information in line structure
bool parse_line(string& s, line& temp){
	istringstream ss;
	ss.str(s);
	ss>>temp.time;
	ss.ignore(1,',');
	ss>>temp.frame;
	ss.ignore(1,',');
	ss>>temp.seg;
	ss.ignore(1,',');
	ss>>temp.nb;
	if (temp.nb>0){
		int i(0);
		do {
			marker m;
			ss.ignore(1,',');
			ss>>m.id;
			ss.ignore(1,',');
			ss>>m.p.x;
			ss.ignore(1,',');
			ss>>m.p.y;
			ss.ignore(1,',');
			double a;
			ss>>a;
			m.a = (short) round2(a * 100);
			temp.tags.push_back(m);
			i++;
		}while(i<temp.nb);
	}
	return true;
}

//==========================================================
// Calculate the distance between to points
double dist(const position& p1, const position& p2){
	double dx = abs(p1.x - p2.x);
	double dy = abs(p1.y - p2.y);
	return sqrt(dx * dx + dy * dy);
}

//==========================================================
//Converts an angle between -180 and +180 to an angle between 0-360¬∞
void convert(double& a){
	if (a > 180){ 
		a = 360 - a;
	}
}

//==========================================================
// Reads a line from the csv input, extracts frame number, timestamp, nb segments and nb tags detected
bool read_line(ifstream& f, frameline& my_frame, map<int,bool>& frames,list <timestamp>& frames_read, timestamp& my_current){
  string s;
  getline(f, s);
  istringstream ss;
  ss.str(s);
  if (!s.empty()){
    ss>>my_frame.time;
    if (!ss.fail()){
      ss.ignore(1,',');
      ss>>my_frame.frame;
      ss.ignore(1,',');
      ss>>my_frame.segments;
      ss.ignore(1,',');
      ss>>my_frame.tags;
      ss.ignore(1,',');
      ss>>my_frame.content;
      ss.clear();
    }else{
      throw Exception(CANNOT_READ_FILE, "Error while reading input.");
    }
    frames[my_frame.frame]=true;  /// add frame to map of read frames
    my_current.frame= my_frame.frame;
    my_current.time = my_frame.time;
    if (my_current.frame == 0){
      throw Exception(INTERNAL_ERROR, "Frame number zero. Should not occur.");
    }
    frames_read.push_back(my_current);
    return true;
  }else{
    return false;
  }
}

//==========================================================
// Updates the boundaries of the file
void update_filelist_limits(vector <info>& filelist, const timestamp& current, int& lastframe, int& firstframe, bool& first, int files, int lastfr, string filename){
  
  if (lastframe == 0 || current.frame == 0){
    cout<<"lastframe "<<lastframe<<", current: "<<current.frame<<endl;
    throw Exception(INTERNAL_ERROR, "Frame = 0, this should not happen.");
  }
  /// update last and first frame from experiment if necessary:
  /// checks whether the current frame is bigger than the last frame, if so updates lastframe and sets last frame of file to current frame
  if (current.frame >= lastframe){
    lastframe = current.frame;
    if (!first){ // if it at least the second frame in the file
      filelist[filelist.size()-1].end=current.frame; /// put end of file to my_file.frame
    }
  }
  
  /// if the current frame is smaller than first frame (or if firstframe is zero), then firstframe and start frame of file is set to current frame
  if (current.frame < firstframe){
    firstframe = current.frame;
    filelist[filelist.size()-1].start = current.frame;
  }
  
  /// for the first frame of each file, add file information to the filelist
  if(first){
    if(files > 1){
      filelist[filelist.size()-1].end=lastfr;  ///< and update last frame from previous file
    }else{
      firstframe = current.frame;
    }
    info file;
    file.name = filename;
    file.start = current.frame;
    //file.end = 0;
    file.state = true;
    first = false;
    file.start_time = current.time;
    file.end_time = current.time;
    filelist.push_back(file); ///< add the file in the filelist
    cout<<"===> added "<<file.name<<" start: "<<file.start<<", end: "<<file.end<<endl;

  }
}

//==========================================================
// Checks order if frames and signals frames that are unordered or incomplete
void check_frame_order(vector <info>& filelist, list <frameline>& framelist, 	map<int, bool>& delayed, map <int, bool>& ordered_frames, frameline& my_frame, const timestamp& current, int lastfr, int& incomplete, int& unordered){
  
  /// tests for unordered frames in the file
  /// if the frame is bigger than the lastframe +1 or smaller or equal to the lastframe it is unordered
  if(current.frame > lastfr+1 || current.frame <= lastfr){
    
    /// for each unordered frame add it to the framelist
    my_frame.type = 'U';
    framelist.push_back(my_frame);
    
    filelist[filelist.size()-1].state = false;
    unordered++;
    if (my_frame.segments < 15){
      incomplete++;
    }
    if (my_frame.segments == 15 && current.frame == lastfr){
      throw Exception (INTERNAL_ERROR, "This should not happen. Twice the same frame: Frame " + to_string(lastfr));
    }
    
    // if the frame is bigger than lastframe+1, then one or several frames have been skipped, add the 'skipped' frames to delayed frames
    if(current.frame > lastfr+1 && lastfr != 0){
      for (int d(lastfr+1); d<current.frame; d++){
        delayed[d]= true;
      }
    }

  /// check whether the frame is complete (= 15 segments)
  }else if (my_frame.segments < 15){

    //cout<<"-> frame "<<my_file.frame<<" incomplete: "<<segments<<" segments."<<endl;
    /// for each incomplete frame add it to the framelist
    my_frame.type = 'I';
    framelist.push_back(my_frame);
    filelist[filelist.size()-1].state = false;
    incomplete++;
    
  /// check whether it is a delayed frame (ordered when compared to previous one, but delayed)
  }else if (delayed.find(current.frame)!=delayed.end()){

    //cout<<"delayed frame "<<my_file.frame<<endl;
    my_frame.type = 'U';
    framelist.push_back(my_frame);
    filelist[filelist.size()-1].state = false;
    unordered++;
  
  // it is an ordered complete frame
  }else{

    ordered_frames[current.frame]= true;
  }
}

//==========================================================
// Check for missing frames. Each missing frame is added to a framelist and the corresponding file is marked for correction.
int check_missing_frames(vector <info> & filelist, map<int,bool>& frames, list <frameline>& framelist, const int& firstframe, const int& lastframe){
  int nb_missing(0);
  for (int i(firstframe); i<=lastframe; i++) {
		
		if (frames.find(i)==frames.end()) {
			cout<<"-> frame "<<i<<" is missing"<<endl;
			frameline missing(i,-1,0,0,'M',"");
			framelist.push_back(missing);
			//
			for(int j(0); j< filelist.size(); j++){
				if (i > filelist[j].start-1 && i <= filelist[j].end){
					filelist[j].state = false;
				}
			}
			nb_missing++;
		}
	}
  return nb_missing;
}

//==========================================================
//  check the limits of all files and corrects them if a pair of files are overlapping, if there are frames missing between two files, the frames are added to the end of the earlier file,
// if there are more than MISS frames missing then, a new files with empty frames is created
void check_overlap(vector <info>& filelist){
  cout<<"files: "<<filelist.size()<<endl;
  
  
  for (int fi(0); fi< filelist.size()-1; fi++){
    // check if files are overlapping, to remove the overlap modify start of file2 and end of file 1. frames are added to file with fewer frames
		if(filelist[fi].end >= filelist[fi+1].start){
			cout<<"--> files "<<filelist[fi].name<<" and "<<filelist[fi+1].name<<" are overlapping"<<endl;
			int df1 = filelist[fi].end - filelist[fi].start;
			int df2 = filelist[fi+1].end - filelist[fi+1].start;
			// decide on how to change file limits
			if (df1 < df2 ){
				filelist[fi+1].start = filelist[fi].end + 1;
			}else{
				filelist[fi].end = filelist[fi+1].start - 1;
			}
			filelist[fi].state = false;
			filelist[fi+1].state = false;
      
    // check that no frames are missing between the 2 subsequent files. all frames need to be part of a file
		}else if (filelist[fi].end + 1 < filelist[fi+1].start){
			filelist[fi].state = false;
			filelist[fi+1].state = false;
			// calculate number of missing frames between the 2 files
			int frames_missing = filelist[fi+1].start - filelist[fi].end -1;
			// nb of missing frames small, they are added to the preceding existing file
			if (frames_missing < MISS){
				cout<<"--> "<<frames_missing <<" frames missing added to end of file "<<filelist[fi].name<<endl;
				filelist[fi].end = filelist[fi].end + frames_missing;
				filelist[fi+1].start = filelist[fi].end + 1;
				
				int df1 = filelist[fi].end - filelist[fi].start;
				int df2 = filelist[fi+1].end - filelist[fi+1].start;
				// decide on how to change file limits
				if (df1 < df2 ){
					filelist[fi+1].start = filelist[fi].end + 1;
				}else{
					filelist[fi].end = filelist[fi+1].start - 1;
				}
				
      // nb of missing frames big, a new file is created.
			}else{
				cout<<"Warning there were "<<frames_missing<<" frames missing between files "<<filelist[fi].name<<" and "<<filelist[fi+1].name<<"."<<endl;
				cout<<"A new file with only missing frames will be created. In case the missing frames are in a file you forgot to add, then quit and restart the program."<<endl;
				
				// create new filename: name of previous file with _b added
				info file;
				string name = filelist[fi].name;
				name.insert(filelist[fi].name.find_last_of('.'), "_b");
				
				cout<<"new filename: "<<name<<endl;
				file.name = name;
				file.start = filelist[fi].end + 1;
				file.end = filelist[fi+1].start -1;
				file.start_time = filelist[fi].end_time + 0.5;
				file.end_time = filelist[fi+1].start_time - 0.5;
				file.state = true;
				//cout<<"added file "<<file.name<<" has state "<<file.state<<endl;
				
				// find the position where the file should be inserted in the filelist
				info file_to_find = filelist[fi+1];
				vector<info>::iterator insert_pos;
				insert_pos = find(filelist.begin(), filelist.end(), file_to_find);
				if (insert_pos == filelist.end()) {
					cout << "Warning: no new file added because could not find file " << file_to_find.name<<endl;
				} else {
					filelist.insert(insert_pos, file);
					// calculate sampling time: because there are many frames missing it is important to calculate the exact samplig time
					// an average sampling time of 0.5 might accumulate sufficant error to create time conflict in the output data
					double sampling_time = (file.end_time - file.start_time)/(double) (frames_missing - 1);
					// create file and write content
					ofstream g;
					g.open(file.name.c_str());
					if (!g.is_open()){
						throw Exception(CANNOT_WRITE_FILE, file.name);
					}
					double unixtime = file.start_time;
					for (int k(file.start); k<=file.end; k++){
						g.precision(12);
						g<<unixtime<<","<<k<<",0,0"<<endl;
						unixtime += sampling_time;
					}
					g.close();
				}
			}
		}
		//cout<<"File: "<<filelist[fi].name<<": start: "<<filelist[fi].start<<", end: "<<filelist[fi].end<<endl;
	}
  /*for (int fi(0); fi< (filelist.size()); fi++){
    cout<<"File: "<<filelist[fi].name<<": start: "<<filelist[fi].start<<", end: "<<filelist[fi].end<<endl;
  }*/
}

//==========================================================
void copy_frames_from_input(ofstream& h, list <timestamp>& frames_read, map <int, string>& early_frames, map<int, bool>& frame_written, timestamp& last_written, const timestamp& my_file, timestamp& current, bool next, const string& s2){
  list <timestamp>::iterator frame_iter;
  map <int, string>::iterator iter_early;
  // write frame from input
  if (next){
    h<<s2<<endl;
    last_written.time = my_file.time;
    //cout<<"frame written from file (2) :"<<my_file.frame<<endl;
    frame_written[my_file.frame]=true;
    last_written.frame = my_file.frame;
    // delete frame from frames_read input list
    if (current.frame == my_file.frame){
      frames_read.pop_front();
      frame_iter = frames_read.begin();
      if (frame_iter != frames_read.end()){
        current = *frame_iter;
      }
    }
    // write frame from early_frames until the next frame should be the one read from the input
  }else{
    
    do{
      // search the next frame in the early_frames map and write it to the file
      iter_early = early_frames.find(last_written.frame+1);
      if (iter_early != early_frames.end()){
        string s4 = iter_early->second;
        h << s4 << endl;
        last_written.frame = last_written.frame+1;
        frame_written[last_written.frame]=true;
        // extract time from frame in early_frames
        double t = atoi((s4.substr(0,s4.find(',')-1)).c_str());
        last_written.time = t;
        // delete frame from frames_read input list
        if (current.frame == last_written.frame){
          frames_read.pop_front();
          frame_iter = frames_read.begin();
          if (frame_iter != frames_read.end()){
            current = *frame_iter;
          }
        }
      }else{
        string info = "Warning2: this should not happen. ";
        throw Exception (INTERNAL_ERROR, info);
      }
    }while(last_written.frame + 1 < my_file.frame);
    
    h<<s2<<endl;
    last_written.time = my_file.time;
    frame_written[my_file.frame]=true;
    last_written.frame = my_file.frame;
    // delete frame from frames_read input list
    if (current.frame == my_file.frame){
      frames_read.pop_front();
      frame_iter = frames_read.begin();
      if (frame_iter != frames_read.end()){
        current = *frame_iter;
      }
    }
  }
}

//==========================================================

void read_file_to_correct(ifstream& f, ofstream& h, info& my_filelist, list <frameline*>& temp, map<int, bool>& frame_written, map <int, string>& early_frames, map <int, bool>& ordered_frames, map <int,bool>&  frame_problem, list <timestamp>& frames_read, timestamp& last_written, timestamp& current){
  
  bool DEBUG = false;
  int DEBUG_FRAME = 1;
  
  if (DEBUG){
    cout<<"current frame "<<current.frame<<", last_written: "<<last_written.frame<<endl;
  }
  map<int, bool>::iterator map_it;
  map <int, string>::iterator iter_early;
  list <frameline*>::iterator iter2;
  list <timestamp>::iterator frame_iter;
  while(!f.eof()){
    timestamp my_file;
    string s;
    string s2;
    getline(f,s);
    s2 = s;
    istringstream ss;
    ss.str(s);
    ss>>my_file.time;
    ss.ignore(1,',');
    ss>>my_file.frame;
    if (DEBUG && my_file.frame > DEBUG_FRAME){
      cout<<"last_written: "<<last_written.frame<<endl;
      cout<<"...frame from input: "<<my_file.frame<<endl;
    }
    // test whether frame from file had been written already or not
    map_it = frame_written.find(my_file.frame);
    if (map_it == frame_written.end()){ //frame not yet written
      
      // test whether frame is part of the file
      if (my_file.frame >= my_filelist.start && my_file.frame <= my_filelist.end){
        bool next (last_written.frame+1 == my_file.frame); // next is true if the frame read from input is the next one to be written
        
        // test whether temporary list is empty: if not, compare frames in lists to current frame from input
        if (temp.begin() != temp.end()){
          iter2 = temp.begin();
          frameline fr = **iter2; //  put content of pointer in fr
          if (DEBUG && my_file.frame > DEBUG_FRAME){
            cout<<"...current first frame in list: "<<fr.frame<<endl;
          }
            //cout<<"frame file: "<<my_file.frame<<" & frame in list: "<<fr.frame<<endl;
          
          //=========================================
          // if the frame from input is smaller than the first frame in the list frames to correct, then copy frame directly from input
          if (my_file.frame < fr.frame && next){
             if (DEBUG && my_file.frame > DEBUG_FRAME){
               cout<<"->copying from input"<<endl;
             }
            h<<s2<<endl;
            last_written.time = my_file.time;
            frame_written[my_file.frame] = true;
            last_written.frame = my_file.frame;
            if (current.frame == my_file.frame){
              frames_read.pop_front();
              frame_iter = frames_read.begin();
              if (frame_iter != frames_read.end()){
                current = *frame_iter;
              }
            }
          
          //=========================================  
          }else if (next && my_file.frame == fr.frame){ // if the current frame from the input is also on the list, take the one from the list and write it to the output
            //map_it = frame_written.find(fr.frame);
            //if ( map_it == frame_written.end()){
            if (DEBUG && my_file.frame > DEBUG_FRAME){
              cout<<"->copying from list frame: "<<endl;
            }
            
            string s3;
            ostringstream so;
            // if the frame in the list is classified as a missing frame
            if(fr.type == 'M'){
              if (fr.frame == current.frame || fr.frame > current.frame){
                string info = "1. The missing frame should not be in the list of frames read or be bigger than the first frame in that list.";
                cout<<"current frame is: "<<current.frame<<", frame in list: "<<fr.frame<<", my_file.frame: "<<my_file.frame<<endl;
                throw Exception(INTERNAL_ERROR, info);
              }else{
                int dframe = current.frame - last_written.frame;
                double dtime = current.time - last_written.time;
                double delay = dtime/(double)dframe;
                so.precision(2);
                so<<fixed<<last_written.time + delay  << "," << fr.frame << ",0,0";
                s3 = so.str();
                last_written.time += delay;
              }
            
              /// if the frame in the list is classified as an incomplete or unordered frame
            /// replace with content from list
            }else{
              if (fr.time == -1){
                cout<<"Frame type is :"<<fr.type<<endl;
                throw Exception(INTERNAL_ERROR, "2. Timestamp should not be -1. This must be a missing frame.");
              }
              so.precision(2);
              so <<fixed<< fr.time << "," << fr.frame << "," << fr.segments << "," << fr.tags << "," << fr.content;
              s3 = so.str();
              last_written.time = fr.time;
            }
            
            h<<s3<<endl;
            if (DEBUG){
              cout<<"frame written from list :"<<fr.frame<<endl;
            }
            frame_written[fr.frame]=true;
            last_written.frame = fr.frame;
            // delete frame from frames_read input list
            if (current.frame == my_file.frame){
              frames_read.pop_front();
              frame_iter = frames_read.begin();
              if (frame_iter != frames_read.end()){
                current = *frame_iter;
              }
            }
            
            temp.pop_front();
            if (temp.begin() != temp.end()){
              iter2 = temp.begin();
              fr = **iter2;
            }
            
          //=========================================  
          }else{ // current frame is bigger that the first one in the list
              
              // check if the current frame is in the list of frames to correct, if it is, we read the next frame from the input
              map <int, bool>::iterator it_frpb;
              it_frpb = frame_problem.find(my_file.frame);
              if (it_frpb == frame_problem.end()){
                
                /// if the current frame is bigger than the one in the temporary list --> there was one or more missing frame(s)
                /// take the frame(s) from temporary list and writes them to the file, then delete them from the list
                while (my_file.frame > fr.frame && (temp.begin()!= temp.end())){
                  
                  if(DEBUG && my_file.frame > DEBUG_FRAME){
                    cout<<"frame from input: "<<my_file.frame<<", frame with problem: "<<fr.frame<<", current frame: "<<current.frame<<" last written: "<<last_written.frame<<endl;
                  }
                  if (last_written.frame+1 == fr.frame){
                    
                    map_it = frame_written.find(fr.frame);
                    if ( map_it == frame_written.end()){ // if frame not yet written
                      if (DEBUG){
                        cout<<"frame not yet written"<<endl;
                      }
                      string s3;
                      ostringstream so;
                      // if it is a missing frame
                      if(fr.type == 'M'){
                        
                        if (fr.frame == current.frame /*|| fr.frame < current.frame*/){
                          string info = "2. The missing frame should not be in the list of frames read.";
                          cout<<"current frame is: "<<current.frame<<", frame in list: "<<fr.frame<<", my_file.frame: "<<my_file.frame<<endl;
                          throw Exception(INTERNAL_ERROR, info);
                        }else{
                          int dframe = current.frame - last_written.frame;
                          double dtime = current.time - last_written.time;
                          double delay = dtime/(double)dframe;
                          so.precision(2);
                          so<<fixed<<last_written.time + delay  << "," << fr.frame << ",0,0";
                          s3 = so.str();
                          last_written.time += delay;
                        }
                        /// if it is an incomplete or unordered frame
                        /// replace with content from list
                      }else{
                        so.precision(2);
                        if (fr.time == -1){
                          cout<<"Frame type is :"<<fr.type<<endl;
                          throw Exception(INTERNAL_ERROR, "1. Timestamp should not be -1. This must be a missing frame.");
                        }
                        so <<fixed<< fr.time << "," << fr.frame << "," << fr.segments << "," << fr.tags << "," << fr.content;
                        s3 = so.str();
                        last_written.time = fr.time;
                        // delete frame from frames_read input list
                        if (current.frame == fr.frame){
                          frames_read.pop_front();
                          frame_iter = frames_read.begin();
                          if (frame_iter != frames_read.end()){
                            current = *frame_iter;
                            if (DEBUG){
                              cout<<"==> current after deletion: "<<current.frame<<endl;
                            }
                          }
                        }
                      }
                      h<<s3<<endl;
                      if (DEBUG && fr.frame > DEBUG_FRAME){
                        cout<<"frame written from list :"<<fr.frame<<endl;
                      }
                      frame_written[fr.frame]=true;
                      last_written.frame = fr.frame;
                    }
                    if (DEBUG){
                      cout<<"taille liste before "<<temp.size()<<endl;
                    }
                    temp.pop_front();
                    if (DEBUG){
                      cout<<"taille liste after "<<temp.size()<<endl;
                    }
                    //cout<<"--> delete frame from list "<<endl;
                    //cout<<"taille liste apres effacement"<<temp.size()<<endl;
                    if (temp.begin()!= temp.end()){
                      iter2 = temp.begin();
                      fr = **iter2;
                      if (DEBUG){
                        cout<<"==> next problem frame in list: "<<fr.frame<<endl;
                      }
                    }
                  }else{
                    // search the next frame in the early_frames map and write it to the file
                    do{
                      iter_early = early_frames.find(last_written.frame+1);
                      if (iter_early != early_frames.end()){
                        string s4 = iter_early->second;
                        h << s4 << endl;
                        last_written.frame = last_written.frame+1;
                        frame_written[last_written.frame]=true;
                        // delete frame from frames_read input list
                        if (current.frame == last_written.frame){
                          frames_read.pop_front();
                          frame_iter = frames_read.begin();
                          if (frame_iter != frames_read.end()){
                            current = *frame_iter;
                          }
                        }
                        
                        // extract time from frame in early_frames
                        double t = atoi((s4.substr(0,s2.find(',')-1)).c_str());
                        last_written.time = t;
                        //}
                      }else{
                        string info = "Warning1: this should not happen. ";
                        throw Exception (INTERNAL_ERROR, info);
                      }
                    }while(last_written.frame + 1 < fr.frame);
                  }
                }
                
                // write the frame from the input
                h<<s2<<endl;
                last_written.time = my_file.time;
                //cout<<"frame written from file (after list) :"<<my_file.frame<<endl;
                frame_written[my_file.frame]=true;
                last_written.frame = my_file.frame;
                // delete frame from frames_read input list
                if (current.frame == my_file.frame){
                  frames_read.pop_front();
                  frame_iter = frames_read.begin();
                  if (frame_iter != frames_read.end()){
                    current = *frame_iter;
                  }
                }
              }
              
            }
            
        }else{ // temporary list is empty -> copy all frames from input
          copy_frames_from_input(h, frames_read, early_frames, frame_written, last_written, my_file, current, next, s2);

        }
      
      // frame is not part of the current file
      }else{
        // if the frame was an ordered frame
        if (ordered_frames[my_file.frame]){
          early_frames[my_file.frame]=s2;  //keep frame content of frames that were ordered but in the wrong file
        }
      }
    }// end map frame_written
  }// end of while on file
}

//=============================================================================================
void copy_frames_from_list(ofstream& h, list <frameline*> temp, timestamp& last_written, timestamp& current, map<int, bool>& frame_written, map <int, string>& early_frames, list <timestamp>& frames_read){
  map<int, bool>::iterator map_it;
  map <int, string>::iterator iter_early;
  list <frameline*>::iterator iter2;
  list <timestamp>::iterator frame_iter;
  while(temp.begin() != temp.end()){
    iter2 = temp.begin();
    frameline fr = **iter2; //  put content of pointer in fr
    string s3;
    ostringstream so;
    
    // test if frame in list is the next frame to be written, if not check in early_frame map
    if (last_written.frame+1 == fr.frame){
      map_it = frame_written.find(fr.frame);
      if (map_it == frame_written.end()){
        
        // if it is a missing frame
        if(fr.type == 'M'){
          if (fr.frame == current.frame || fr.frame > current.frame){
            string info = "3. The missing frame should not be in the list of frames read or be bigger than the first frame in that list.";
            //cout<<"current frame is: "<<current.frame<<", frame in list: "<<fr.frame<<endl;
            throw Exception(INTERNAL_ERROR, info);
          }else{
            int dframe = current.frame - last_written.frame;
            double dtime = current.time - last_written.time;
            double delay = dtime/(double)dframe;
            so.precision(2);
            so<<fixed<<last_written.time + delay  << "," << fr.frame << ",0,0";
            s3 = so.str();
            last_written.time += delay;
          }
          /// if it is an incomplete or unordered frame
          /// replace with content from list
        }else{
          if (fr.time == -1){
            throw Exception(INTERNAL_ERROR, "3. Timestamp should not be -1. This must be a missing frame.");
          }
          so.precision(2);
          so <<fixed<< fr.time << "," << fr.frame << "," << fr.segments << "," << fr.tags << "," << fr.content;
          s3 = so.str();
          last_written.time = fr.time;
          // delete frame from frames_read input list
          if (current.frame == fr.frame){
            frames_read.pop_front();
            frame_iter = frames_read.begin();
            if (frame_iter != frames_read.end()){
              current = *frame_iter;
            }
          }
          
        }
        h<<s3<<endl;
        //cout<<"frame written from list(2) :"<<fr.frame<<endl;
        frame_written[fr.frame]=true;
        last_written.frame = fr.frame;
        
      }
      temp.pop_front();
    }else{
      do{
        // search the next frame in the early_frames map and write it to the file
        iter_early = early_frames.find(last_written.frame+1);
        if (iter_early != early_frames.end()){
          
          string s4 = iter_early->second;
          h << s4 << endl;
          last_written.frame = last_written.frame+1;
          frame_written[last_written.frame]=true;
          // extract time from frame in early_frames
          double t = atoi((s4.substr(0,s4.find(',')-1)).c_str());
          last_written.time = t;
          // delete frame from frames_read input list
          if (current.frame == last_written.frame){
            frames_read.pop_front();
            frame_iter = frames_read.begin();
            if (frame_iter != frames_read.end()){
              current = *frame_iter;
            }
          }
          
        }else{
          string info = "Warning3: this should not happen. ";
          throw Exception (INTERNAL_ERROR, info);
        }
      }while(last_written.frame + 1 < fr.frame);
    }
  }
}

//==========================================================
void correct_files(vector <info>& filelist, map <int, bool>& ordered_frames, list <timestamp>& frames_read, list <frameline>& framelist, vector <string>& filelist2){
  /// read input file that needs ordering
  map <int, bool> frame_written; ///< framenumber of the written frames are set to true
  map <int, bool>::iterator map_it;
  map <int, string> early_frames; ///< frames that are ordered locally but should be in a different file
  map <int, string>::iterator iter_early;
  list <frameline>::iterator iter;		///< iterator of framelist

  timestamp last_written (filelist[0].start-1, filelist[0].start_time - 0.5); ///< last timestamp (frame, Unix time)

  for (int i(0); i< filelist.size(); i++){
    
    cout<<"File "<<filelist[i].name<<" :"<<endl;
    
    // get first timestamp in the sorted list of frames_read, this is the current timestamp
    list <timestamp>::iterator frame_iter;
    frame_iter = frames_read.begin();
    timestamp current;
    if (frame_iter != frames_read.end()){ // set current to next frame expected from input, unless there are no more frames expected
      current = *frame_iter;
    }
    
    if(!filelist[i].state){ /// if the file has unordered, incomplete or missing frames, write corrected file
      
      //cout<<"---> has state "<<filelist[i].state<<endl;
      
      list <frameline*> temp;				///< temporary list to hold pointers to frames that need correction for a given file
      list <frameline*>::iterator iter2;  ///< iterator for temporary list
      
      map <int,bool> frame_problem;
      /// identify  all frame problems in that file
      cout<<"-> identifiying file problems ..."<<endl;
      for (iter = framelist.begin(); iter != framelist.end(); iter++){
        frameline& fr = *iter;
        if (fr.frame > filelist[i].start -1 &&  fr.frame <= filelist[i].end){
          frameline* a= &fr;
          temp.push_back(a);
          frame_problem[fr.frame]=true;
        }
      }
      
      cout<<"--> "<<temp.size()<<" file problems"<<endl;
      
      // display frames that need to be corrected in the file
      int k (0);
      for (iter2 = temp.begin(); iter2 !=temp.end();iter2++){
        frameline& testframe = **iter2;
        cout<<"["<<k<<"]  = "<<testframe.frame<<endl;
        k++;
      }
      
      /// open input file for reading
      ifstream f;
      f.open(filelist[i].name.c_str());
      cout << "--> correcting file " << filelist[i].name << endl;
      if (!f.is_open()) {
        string info = filelist[i].name;
        throw Exception(CANNOT_OPEN_FILE, info);
      }
      
      /// create copy of filename for ordered file (marqued with "_corr")
      int p= filelist[i].name.find_last_of('.');
      if(p == string::npos){
        string info = "Input file name format wrong (no .* extension).";
        throw Exception (INTERNAL_ERROR, info);
      }
      string copy = filelist[i].name.insert( p, "_corr" );
      
      /// open output file (_corr file)
      ofstream h;
      h.open(copy.c_str());
      if (!f.is_open()) {
        string info = copy;
        throw Exception(CANNOT_OPEN_FILE, info);
      }
      
      /// add name of _corr file to list for dat file generation
      filelist2.push_back(copy);
      read_file_to_correct(f, h, filelist[i], temp, frame_written, early_frames, ordered_frames, frame_problem, frames_read, last_written, current);
      
      // if the input has been read entirely but the list of frame to correct is not empty, we write the frames from the list directly to the file
      
      if (temp.begin() != temp.end()){
        cout<<"--> copying frames from list. last frame written from input: "<<last_written.frame<<endl;
        copy_frames_from_list(h, temp, last_written, current, frame_written, early_frames, frames_read);
      }
      /// close input and ordered output file
      f.close();
      h.close();
    }else{
      while(current.frame < filelist[i].end){
        frames_read.pop_front();
        frame_iter = frames_read.begin();
        current = *frame_iter;
      }
      
      cout<<"--> file ok"<<endl;
      // add name of file to list for dat file generation
      filelist2.push_back(filelist[i].name);
      last_written.frame = filelist[i].end;
      cout<<"last written frame: "<<last_written.frame<<endl;
    }
  }
}


