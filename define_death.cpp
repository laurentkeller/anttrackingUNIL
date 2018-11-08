/*
 *  define_death.cpp
 *	the programme identifies dead ants or lost tags and indicates the time of the death or loss
 *  to do this the programm uses the total distance in a given interval (INTERVAL) if the distance is below a threshold 
 *  for more than DEATH_DELAY intervals, then the ant is considered dead since DEATH frames
 *
 *  Created by Danielle Mersch on 10/1/10.
 *  Copyright 2010 __UNIL__. All rights reserved.
 *
 */


#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <map>
#include <time.h>
#include <cstdlib>
#include <getopt.h> 

#include "trackcvt.h"
#include "exception.h"
#include "datfile.h"
#include "tags3.h"
#include "utils.h"

using namespace std;

const string VERSION = "DeadDetector 1.0.0";


//const unsigned int TUNNEL = 10944; /// length of tunnel converted to pixels (tunnel about 65cm)
const unsigned int P_TH = 3;  /// precision threshold of position detection
//double dist_lookup[SIZE_X][SIZE_Y];  // look-up table with all distances within a box --> for optimization purposes

//-------- structures --------------------------

// counters : holds the counters for intervals classification
struct compteur{
	unsigned int a; //ant moved more than threshold
	unsigned int b; // ant moved less than threshold but was not immobile
	unsigned int c; // ant was immobile
	unsigned int d;  // ant was not detected
	unsigned int e;  // ant was detected
	unsigned int f; // ant moved more than threshold from fixed point
	unsigned int g; // ant moved less than threshold from fixed point
};


// intermediate descriptors, used to store information for a given interval
struct lastinfo{
	position pos;   // last position
	int frame;  // frame of last detection
	position fixedpos;  // first position of interval
	double sum_dist;  // holds the sum of the distances run during the INTERVAL
	int ctr_fixed;  // counter for distances to fixed point
	double sum_fixed; // holds the sum of the distances to the first position in the INTERVAL
	bool dead;  // is true if ant is considered dead, false by default
	int det;  // counts the number of detections in an interval
	int immobile;  // counts the number of time steps the ant was immobile
	int hole;  // counts the number of holes in the interval
};



// ==================================================================================
/* \brief Calculates distance between 2 points and corrects for additional distance due to different boxes if necessary
 * \param pos Position of last detection
 * \param box Box of last detection
 * \param t Position and box of current detection
 * \param boxes Map with ID and position of door of all boxes in data
 * \param tunnels Map with pair of boxes and shortes tunnel length linking them for all combinaitions in data
 * \return double Distance between the 2 points: pos and t
 */
double calculate_distance(const position& pos, const tag_pos& t){
	int dx, dy;
	double dist;
	dx = pos.x - t.x;
	dy = pos.y - t.y;
	dist = sqrt((double)dx*dx + dy*dy);
	//dist = dist_lookup[abs(dx)][abs(dy)];
	return dist;
}

// ==================================================================================
/* \brief Tests if the ant had been detected in the frame of death, if not searches the last detection prior to the frame
 * \param dat DatFile containing the data 
 * \param frame Number of the frame in which the ant has died
 * \param idx Index of the ant in the tag_list table
 * \return int number of frame
 */
int find_death(DatFile& dat, const unsigned int fr, int idx){
	unsigned int curr = dat.get_current_frame();
	dat.go_to_frame(fr);
	framerec t; 
	dat.read_frame(t);
	int death (-1);
	// if tag had not been detected, search previous detection
	if (t.tags[idx].x == -1){
		if (!dat.find_previous_frame_with_index(idx, death)){
			death = fr;
		}
	}else{
		death = fr;
	}
	dat.go_to_frame(curr);
	dat.read_frame(t);
	return death;
}

// ==================================================================================
int main(int argc, char* argv[]){
	
	try{
		
		// test if the number of arguments is correct
		if (argc != 4){
			string info = (string) argv[0] + " input.dat input.tags postprocessing.log";
			throw Exception(USE, info);
		}
		
		// open datfile in read only mode
		DatFile dat;
		dat.open((string) argv[1], 0);
		
		//open tags file
		TagsFile tgs;
		string tags_file = (string) argv[2];
		tgs.read_file(tags_file.c_str());
		
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
		
		// constants for parameters
		const unsigned int INTERVAL = 600; /// 600 interval for characteristics in frames
		const unsigned int DEATH  = 172800;  /// 172800 number of frames since ant died
		const unsigned int DEATH_DELAY = DEATH/INTERVAL;
		const double F_COEF = 0.9;  // 0.9
		const unsigned FIXED = DEATH * F_COEF;
		const unsigned int FIXED_DELAY = FIXED/ INTERVAL;
		const double I_COEF = 0.3; // 0.45 - 0.55 
		const unsigned int IMMOBILE = DEATH * I_COEF;  
		const unsigned int I_TH = IMMOBILE /INTERVAL; // number of intervals that the ant can be immobile before it is considered dead
		const double H_COEF = 1.8; //1.6
		const unsigned int HOLE = DEATH * H_COEF;  //
		const unsigned int H_TH = HOLE /INTERVAL;// number of intervals that the ant can be missing before it is considered dead
		const double I_LIMIT_TH = 0.75;  // 0.75
		const unsigned int I_LIMIT = INTERVAL * I_LIMIT_TH; //0.75 number of times the ant has to be immobile during an interval and still be considered entirely immobile
		
		const unsigned int D_TH = 100; /// 100 distance threshold
		const unsigned int GREAT_DIST_TH = 20;  ///20 number of times the distance can be above D_TH pixels and the ant will still be considered dead
		const unsigned int DET_TH = 35000/INTERVAL; // number of intervals in which the tag should be detected to be considered seen: 5hours
		const unsigned int FIX_TH = 3000;  /// 3000 fixed distance threshold
		const unsigned int GREAT_FIXED_TH = 8;  // number of times the fixed distance can be above FIX_TH and the ant will still be considered dead
			
		// initialize everything
		
		//init_dist_lut();
		compteur count[tag_count];
		memset(&count, 0, sizeof(count));
		lastinfo last[tag_count];
		memset(&last, 0, sizeof(last));
		bool present[tag_count];
		memset(&present, 0, sizeof(present));
		int false_positives(0);
		int max(0);
		int maxtag(-1);
		double mean (0);
		int ctr (0);
		long long ss(0);
		// read through file
		//cout<<"reading file ..."<<endl;
		
		cout<<endl;
		cout<<"=========================================="<<endl;
		cout<<"--------- parameter settings -------------"<<endl;
		cout<<"Interval: "<<INTERVAL<<endl;
		cout<<"Death: "<<DEATH<<" Immobile coef: "<<I_COEF<<" Hole coef: "<<H_COEF<<endl; 
		cout<<"Distance threshold: "<<D_TH<<", Nb of big distances accepted:"<<GREAT_DIST_TH<<endl;
		cout<<"Detection threshold: "<<DET_TH<<", Immobile threshold: "<<I_LIMIT_TH<<endl;
		cout<<"Fixed distance threshold: "<<FIX_TH<<", Nb of big fixed distances accepted: "<<GREAT_FIXED_TH<<endl;
		cout<<endl;
		
		while(!dat.eof()){
			
			int t(0);
			framerec temp;
			do{  // start of interval
				if (dat.read_frame(temp)){
				
					// for first frame of interval
					if (t==0){
						
						for(int i(0); i< tag_count; i++){
							if (tgs.get_state(i) && !last[i].dead){
								// if tag detected: initialize with coordinates otherwise set to -1
								if (temp.tags[i].x != -1){ 
									last[i].pos.x = temp.tags[i].x;
									last[i].pos.y = temp.tags[i].y;
									last[i].frame = temp.frame;
									last[i].sum_dist = 0;
									last[i].sum_fixed = 0;
									last[i].ctr_fixed = 0;
									last[i].det = 1;
									last[i].immobile = 0;
									last[i].hole =0;
									present[i] = true;
									last[i].fixedpos.x = temp.tags[i].x;
									last[i].fixedpos.y = temp.tags[i].y;
								}else{
									last[i].pos.x = -1;
									last[i].pos.y = -1;
									last[i].frame = temp.frame;
									last[i].sum_dist = -1;
									last[i].sum_fixed = -1;
									last[i].ctr_fixed = -1;
									last[i].det = 0;
									last[i].immobile = -1;
									last[i].hole = 1;
									last[i].fixedpos.x = -1;
									last[i].fixedpos.y = -1;
								}
							}
						}
						// for all other frames of interval	
					}else{
						for(int i(0); i< tag_count; i++){
							
							if (tgs.get_state(i) && !last[i].dead){
								if (temp.tags[i].x != -1){// tag detected
									present[i] = true;
									if (last[i].pos.x != -1){  // at least second detection in interval
										// calculate distance to previous detection and add it
										double d = calculate_distance(last[i].pos, temp.tags[i]);
										if (d < P_TH){ // distance smaller than precision threshold
											last[i].immobile++;
										}else{
											last[i].sum_dist += d;
										}
										
										d = calculate_distance(last[i].fixedpos, temp.tags[i]);
										last[i].sum_fixed += d;
										last[i].ctr_fixed ++;
										
									}else{ // first detection of interval: initialize variables
										last[i].sum_dist = 0;
										last[i].sum_fixed = 0;
										last[i].ctr_fixed = 0;
										last[i].det = 1;
										last[i].immobile = 0;
										last[i].fixedpos.x = temp.tags[i].x;
										last[i].fixedpos.y = temp.tags[i].y;
									}
									//update last detection
									last[i].pos.x = temp.tags[i].x;
									last[i].pos.y = temp.tags[i].y;
									last[i].frame = temp.frame;
									last[i].det++;
								}else{
									last[i].hole++;
								}
							}
						}
					}
				}
				t++;
			}while(t<INTERVAL && !dat.eof());  // end of interval
			
						
			// test for dead ants
			for (int i(0); i< tag_count; i++){
				// if the ant is not dead
				if (tgs.get_state(i) && present[i] && !last[i].dead){
					if (last[i].det > 0){
						last[i].sum_fixed /= last[i].ctr_fixed;  /// calculate mean fixed distance
						// the distance is compared to a threshold and the corresponding counter incremented
						if (last[i].sum_dist > D_TH){
							count[i].a++;
						}else if (last[i].immobile > I_LIMIT) {
							count[i].c++;
						}else{
							count[i].b++;
						}
						if (last[i].sum_fixed > FIX_TH){
							count[i].f++;
						}else{
							count[i].g++;
						}	
						count[i].e++; // detection probability
					}else{
						count[i].d++;  // holes
					}
					// if the counter for big distances exceeds a threshold, the counters are reset
					if (count[i].a > GREAT_DIST_TH){
						count[i].a = 0;	// big distances
						count[i].b = 0; //small distances
						count[i].c = 0; // immobility
						count[i].d = 0; // holes
						count[i].e = 0;  // number of detections
						count[i].f = 0; //big distances to fixed point 
						count[i].g = 0; // small distances to fixed point
					
					}
					if (count[i].f > GREAT_FIXED_TH){
						count[i].a = 0;	// big distances
						count[i].b = 0; //small distances
						count[i].c = 0; // immobility
						count[i].d = 0; // holes
						count[i].e = 0;  // number of detections
						count[i].f = 0; //big distances to fixed point 
						count[i].g = 0; // small distances to fixed point
					}
					
					// if the counter for detections exceeds a detection probability the hole and detection counters are reset
					if (count[i].e > DET_TH){
						count[i].d = 0; // holes
						count[i].e = 0; // number of detections
					}
					
					// if the sum of the counter for small distances and immobility exceeds a threshold, the ant is considered dead
					if (count[i].b + count[i].c > DEATH_DELAY){
						cout<<"small dist & imm ---> tag "<<tag_list[i]<<" small dist ="<<count[i].b<<" immobile ="<<count[i].c<<" holes ="<<count[i].d<<endl;
						//int frame = temp.frame - DEATH - INTERVAL;
						int frame = temp.frame - INTERVAL * count[i].c - count[i].b * INTERVAL -count[i].d * INTERVAL ;
						int death = find_death(dat, frame, i);
						double precision (0.1);
						tgs.set_death(i, death);
						last[i].dead = true;
					
					// if count of the fixed distances exceed FIXED_DELAY the ant is declared dead
					}else if(count[i].g > FIXED_DELAY){
						cout<<"fixed dist ---> tag "<<tag_list[i]<<" fixed dist ="<<count[i].g<<endl;
						//int frame = temp.frame - FIXED - INTERVAL;
						int frame = temp.frame - INTERVAL * count[i].g - count[i].d * INTERVAL ;
						int death = find_death(dat, frame, i);
						double precision (0.1);
						tgs.set_death(i, death);
						last[i].dead = true;
						
					// if the count of immobile intervals exceeds I_TH/2 and the count of hole intervals exceeds H_TH/2 the and is considred dead
					}else if (count[i].c > I_TH/2 && count[i].d > H_TH/2){
						cout<<"immobile --> tag "<<tag_list[i]<<" at frame "<<temp.frame<<" b ="<<count[i].b<<" c ="<<count[i].c<<" d ="<<count[i].d<<endl;
						int frame = temp.frame - INTERVAL * count[i].c - count[i].b * INTERVAL -count[i].d * INTERVAL ;
						//int frame = temp.frame - count[i].c * INTERVAL - count[i].d * INTERVAL - INTERVAL;
						int death = find_death(dat, frame, i);
						double precision (0.1);
						tgs.set_death(i, death);					
						last[i].dead = true;
						
					// if the count of hole intervals exceeds H_TH, the ant is declared dead
					}else if (count[i].d > H_TH){
						cout<<"-->hole: tag "<<tag_list[i]<<" at frame "<<temp.frame<<" b ="<<count[i].b<<" imm ="<<count[i].c<<" holes ="<<count[i].d<<endl;
						//int frame = temp.frame - HOLE - INTERVAL;
						int frame = temp.frame - INTERVAL * count[i].g;
						int death = find_death(dat, frame, i);
						double precision (0.1);
						tgs.set_death(i, death);
						last[i].dead = true;
						
					}else if (count[i].c > I_TH*3){
						//int frame = temp.frame - IMMOBILE - INTERVAL;
						cout<<"immobility ---> tag "<<tag_list[i]<<" small dist ="<<count[i].b<<" immobile ="<<count[i].c<<" holes ="<<count[i].d<<endl;
						int frame = temp.frame - INTERVAL * count[i].c - count[i].b * INTERVAL -count[i].d * INTERVAL ;
						int death = find_death(dat, frame, i);
						double precision (0.1);
						tgs.set_death(i, death);					
						last[i].dead = true;
					
					}

				}
			}
		}
		dat.close();
		
		
		tgs.write_file(tags_file.c_str());

		return 0;
		
	}catch(Exception e){}
	
}
					
