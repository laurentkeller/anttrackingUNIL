/*
 *  extrapolate_coordinates_step1.cpp
 *  Modified by Nathalie Stroeymeyt in 2016 based on a program created by Danielle Mersch.
 *  Copyright UNIL. All rights reserved.
 *
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

struct last_position{
	int tag;
	int frame_number;
	int box;
	int x;
	int y;
	double a;
};

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
	
	double nb_steps;	double x_step; double y_step ; double a_step; int new_x; int new_y; int new_a; int angle_diff;
	// test whether all parameters are presen
	if (argc != 7){
		string info = (string) argv[0] + " input.dat input.tags output.txt maximum_time_gap(frames) maximum_distance_moved(pixels) maximum_angle_changed(abs_degree*100)";
		throw Exception(USE, info);
	}
	
	// read arguments
	int time_thresh(atof(argv[4]));	
	int dist_thresh(atof(argv[5]));
	int angle_thresh(atof(argv[6]));

	cout << "Maximum time gap between successive detections = " << time_thresh << " frames." << endl;
	cout << "Maximum distance moved between successive detections = " << dist_thresh << " pixels." << endl;
	cout << "Maximum change in angle between successive detections = " << angle_thresh/100 << " degrees." << endl;

	// opens input files	
	DatFile dat;
	dat.open((string) argv[1], 0);
		
	// opens tags file
	TagsFile tags;
	tags.read_file(argv[2]);
	
	// opens outputfile
	ofstream g;
	g.open(argv[3]);
	if (!g.is_open()){
		throw Exception(CANNOT_OPEN_FILE, (string) argv[3]);
	}
	g<< "#frame,tag,box,x_coor,y_coor,angle"<<endl;
	
	// initialise last detections
	vector<last_position> last_positions;
	for (int i(0); i< tag_count; i++){
		last_position initial_pos;
		initial_pos.tag = tag_list[i];
		initial_pos.frame_number = -1;
		initial_pos.box = -1;
		initial_pos.x = -1;
		initial_pos.y = -1;
		initial_pos.a = -1;
		last_positions.push_back(initial_pos);
	}
	cout<<"Reading datfile and extrapolating... "<<endl;

	while(!dat.eof()){
		
		framerec temp;
		dat.read_frame(temp);
		
		for (int i(0); i< tag_count; i++){
			if (tags.get_state(i) && (tags.get_death(i)== 0 || tags.get_death(i) > temp.frame)){//if ant in tag file and still alive at that time_of_day
				if (temp.tags[i].x != -1){// if ant detected in that frame
				// check conditions for extrapolation
				if(
					(last_positions[i].frame_number != -1)//ant was detected before
					&&
					(last_positions[i].frame_number < temp.frame-1)//ant was not detected in last frame
					&&
					(last_positions[i].box==(int)temp.tags[i].id)//ant was detected in same box
					&&
					(last_positions[i].frame_number>=temp.frame-time_thresh)// ant was detected a number of frame ago inferior to time_thresh#
					 &&
					(((temp.tags[i].x-last_positions[i].x)*(temp.tags[i].x-last_positions[i].x) + (temp.tags[i].y-last_positions[i].y)*(temp.tags[i].y-last_positions[i].y) )<= (dist_thresh*dist_thresh))//ant has moved less than dist_thresh; ie, (x1-x2)^2+(y1-y2)^2 <=d^2
				){
				// then calculate angle diff and test that condition too				
					angle_diff = temp.tags[i].a - last_positions[i].a; // angles have to be between (-180*100 and +180*100; so correct angle_diff accordingly) 
					while (angle_diff > 18000)	{ angle_diff = angle_diff - 36000;}
					while (angle_diff  <= (-18000)){ angle_diff = angle_diff + 36000;}
					if ((angle_diff*angle_diff)<=(angle_thresh*angle_thresh)){//if angle difference is lower than angle threshold. Use squares because angle diff can have positive or negative values
					//then if those conditions are met, interpolate
						nb_steps = (temp.frame-last_positions[i].frame_number);//calculate how many steps. Equals to nb of frames to fill +1
						x_step = (temp.tags[i].x-last_positions[i].x)/nb_steps;
						y_step = (temp.tags[i].y-last_positions[i].y)/nb_steps;
						a_step = (angle_diff)/nb_steps;
						int frames_to_fill(nb_steps-1);// calculate nb of frames to fill. 

						for (int j(1); j<=frames_to_fill; j++){// for each frame to fill
							new_x = (last_positions[i].x+j*x_step);//calculate intermediate x
							new_y = (last_positions[i].y+j*y_step) ;//calculate intermediate y
							new_a = (last_positions[i].a+j*a_step);//calculate intermediate a
							while (new_a > 18000)	{ new_a = new_a - 36000;}//new_a must be between +180*100 and -180*100, so correcte accordingly
							while (new_a  <= (-18000)){ new_a = new_a + 36000;}

							//then output those values; which will be used in a second step to modify dat file
							g << last_positions[i].frame_number + j <<",";
							g << tag_list[i] <<",";
							g << (int)temp.tags[i].id<<",";
							g << new_x<<",";
							g << new_y<<",";
							g << new_a << endl;
						}


					}
					
				}

				// finally update last positions
				last_positions[i].frame_number = temp.frame;
				last_positions[i].box = (int)temp.tags[i].id;
				last_positions[i].x = temp.tags[i].x;
				last_positions[i].y = temp.tags[i].y;
				last_positions[i].a = temp.tags[i].a;
				}
			}
		}
	}
	g.close();

}catch (Exception e) {
}
}

