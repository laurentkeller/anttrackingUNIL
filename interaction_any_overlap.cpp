  /*
 *  interaction.cpp
 *  --> reads through dat file and check for every frame and every pair of tags whether they are interacting. If so they are written to and outfile
 *
 *  Created by Danielle Mersch on 11/10/10.
 *  Modified by Nathalie Stroeymeyt (NS) in June 2016.
 *  Copyright 2010 __UNIL__. All rights reserved.
 *
 */


#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <vector>

#include "trackcvt.h"
#include "exception.h"
#include "tags3.h"
#include "utils.h"

using namespace std;

struct coordinates{
  double x;
  double y;
};

struct segment{
  coordinates point1;
  coordinates point2;
};

//==== NS code start =====
// Returns 1 if the lines intersect, otherwise 0. In addition, if the lines 
// intersect the intersection point may be stored in the doubles i_x and i_y.
bool get_line_intersection(segment segment1, segment segment2)
{
  
  double p0_x, p0_y, p1_x, p1_y, p2_x, p2_y, p3_x, p3_y;
  p0_x = segment1.point1.x;p0_y = segment1.point1.y;
  p1_x = segment1.point2.x;p1_y = segment1.point2.y;
  p2_x = segment2.point1.x;p2_y = segment2.point1.y;
  p3_x = segment2.point2.x;p3_y = segment2.point2.y;
  
  
  double s1_x, s1_y, s2_x, s2_y;
  s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
  s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;
  
  double s, t;
  s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
  t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);
  
  if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
  {
    // Collision detected
     return 1;
  }
  
  return 0; // No collision
}
//==== NS code end =====
// =====================================================================================
/**\fn bool test_interaction(tag_pos pt1, tag_pos pt2, int size1, int size2, int d_th, int a_th_par, int a_th_beh, double width_factor, double width_ratio)
 * \brief Test whether 2 tags with positions pt1 ant pt2 and radius' size1 and size2 are interacting
 * \param pt1 Position and angle of tag1
 * \param pt2 Position and angle of tag2
 * \param size_md1 Md reach of ant1
 * \param size_md2 Md of ant2
 * \param size_ant1 Antenna reach of ant1
 * \param size_ant2 Antenna of ant2
 * \param tl_md1 trapezoid kength of ant 1, from mandibles to gaster tip
 * \param tl_md2 trapezoid kength of ant 2, from mandibles to gaster tip
 * \param tl_ant1 trapezoid kength of ant 1, from antenna to gaster tip
 * \param tl_ant2 trapezoid kength of ant 2, from antenna to gaster tip
 * \param d_th Distance threshold
 * \param a_th_par Angle threshold for paralell ants
 * \param width_factor Factor by which the trapezoid increases in width at the head of the ant compared to the abdomen of the ant
 * \param width_ratio Ratio of the width of the ant compared to the height of the ant 
 * \return 0 if there is no interaction, 1 if interaction
 */
bool test_interaction(tag_pos pt1, tag_pos pt2, int size_md1, int size_md2, double tl_md1, double tl_md2, int d_th, int a_th_par, double width_factor, double width_ratio){

	// if no position data for one ant only or if ants in 2 different boxes, no interaction is possible
	if (pt1.x == -1 || pt2.x==-1 || pt1.id != pt2.id){
		return 0;
	}
	// angles (orientation vector) in degres and distance calculation
	double a1 = ((double) pt1.a / 100.0);
	double a2 = ((double) pt2.a / 100.0);
	double dx = pt2.x - pt1.x;
	double dy = pt2.y - pt1.y;
	double dist = sqrtf((double)dx*dx + (double)dy*dy);   // distance

	// si (distance < (Antenna reach 1+ Antenna reach 2 +threshold)) alors interaction possible, autrement non
	if (dist >= size_md1 + size_md2 + d_th){
		return 0;
	}

	// Calculates angle difference between both orientation vectors of ants
	double da = abs(limit_angle(a2 - a1));

	// if the angle difference is bigger the the parallel threshold, then an interaction is possible, 
	// otherwise ants are more or less paralell and unlikly to interact 
	if (da < a_th_par){
		return 0;
	}
  
  //==== NS code start =====
  // get the 4 corners of ant 1
  //param w1 Width of trapezoid at the head end of the ant modeled (= radius * width_ratio *width_factor)
	//param w2 Width of trapezoid at the abdomen end of the ant modeled (= radius * width_ratio)
	  
  double ant1_w1 = tl_md1/2 * width_ratio * width_factor;
	double ant1_w2 = tl_md1/2 * width_ratio;
	double ant1_angle_rad = a1 * M_PI / 180.0; // Convert angle to radians for trigonometry
	
	coordinates ant1_front_midpoint; 
	ant1_front_midpoint.x = pt1.x + size_md1 * cos (ant1_angle_rad); 
	ant1_front_midpoint.y = pt1.y - size_md1 * sin (ant1_angle_rad);
	
	coordinates ant1_back_midpoint; 
	ant1_back_midpoint.x = pt1.x + (tl_md1-size_md1) * cos (ant1_angle_rad+M_PI); 
	ant1_back_midpoint.y = pt1.y - (tl_md1-size_md1) * sin (ant1_angle_rad+M_PI);
	
	coordinates ant1_corner1; coordinates ant1_corner2; coordinates ant1_corner3; coordinates ant1_corner4; 
	
  ant1_corner1.x = ant1_front_midpoint.x + (ant1_w1/2) * cos(ant1_angle_rad + (M_PI/2));
  ant1_corner1.y = ant1_front_midpoint.y - (ant1_w1/2) * sin(ant1_angle_rad + (M_PI/2));
  ant1_corner2.x = ant1_front_midpoint.x + (ant1_w1/2) * cos(ant1_angle_rad - (M_PI/2));
  ant1_corner2.y = ant1_front_midpoint.y - (ant1_w1/2) * sin(ant1_angle_rad - (M_PI/2));
  ant1_corner3.x = ant1_back_midpoint.x +  (ant1_w2/2) * cos(ant1_angle_rad - (M_PI/2));
  ant1_corner3.y = ant1_back_midpoint.y -  (ant1_w2/2) * sin(ant1_angle_rad - (M_PI/2));
  ant1_corner4.x = ant1_back_midpoint.x +  (ant1_w2/2) * cos(ant1_angle_rad + (M_PI/2));
  ant1_corner4.y = ant1_back_midpoint.y -  (ant1_w2/2) * sin(ant1_angle_rad + (M_PI/2));
  
   segment ant1_segment1;segment ant1_segment2;segment ant1_segment3;segment ant1_segment4;
   ant1_segment1.point1 = ant1_corner1;ant1_segment1.point2 = ant1_corner2;
   ant1_segment2.point1 = ant1_corner2;ant1_segment2.point2 = ant1_corner3;
   ant1_segment3.point1 = ant1_corner3;ant1_segment3.point2 = ant1_corner4;
   ant1_segment4.point1 = ant1_corner4;ant1_segment4.point2 = ant1_corner1;
  
  // get the 4 corners of ant 2
  //param w1 Width of trapezoid at the head end of the ant modeled (= radius * width_ratio *width_factor)
  //param w2 Width of trapezoid at the abdomen end of the ant modeled (= radius * width_ratio)
  
  double ant2_w1 = tl_md2/2 * width_ratio * width_factor;
  double ant2_w2 = tl_md2/2 * width_ratio;
  double ant2_angle_rad = a2 * M_PI / 180.0; // Convert angle to radians for trigonometry
  
  coordinates ant2_front_midpoint; 
  ant2_front_midpoint.x = pt2.x + size_md2 * cos (ant2_angle_rad); 
  ant2_front_midpoint.y = pt2.y - size_md2 * sin (ant2_angle_rad);
  
  coordinates ant2_back_midpoint; 
  ant2_back_midpoint.x = pt2.x + (tl_md2-size_md2) * cos (ant2_angle_rad+M_PI); 
  ant2_back_midpoint.y = pt2.y - (tl_md2-size_md2) * sin (ant2_angle_rad+M_PI);
  
  coordinates ant2_corner1; coordinates ant2_corner2; coordinates ant2_corner3; coordinates ant2_corner4; 
  
  ant2_corner1.x = ant2_front_midpoint.x + (ant2_w1/2) * cos(ant2_angle_rad + (M_PI/2));
  ant2_corner1.y = ant2_front_midpoint.y - (ant2_w1/2) * sin(ant2_angle_rad + (M_PI/2));
  ant2_corner2.x = ant2_front_midpoint.x + (ant2_w1/2) * cos(ant2_angle_rad - (M_PI/2));
  ant2_corner2.y = ant2_front_midpoint.y - (ant2_w1/2) * sin(ant2_angle_rad - (M_PI/2));
  ant2_corner3.x = ant2_back_midpoint.x +  (ant2_w2/2) * cos(ant2_angle_rad - (M_PI/2));
  ant2_corner3.y = ant2_back_midpoint.y -  (ant2_w2/2) * sin(ant2_angle_rad - (M_PI/2));
  ant2_corner4.x = ant2_back_midpoint.x +  (ant2_w2/2) * cos(ant2_angle_rad + (M_PI/2));
  ant2_corner4.y = ant2_back_midpoint.y -  (ant2_w2/2) * sin(ant2_angle_rad + (M_PI/2));
  
  segment ant2_segment1;segment ant2_segment2;segment ant2_segment3;segment ant2_segment4;
  ant2_segment1.point1 = ant2_corner1;ant2_segment1.point2 = ant2_corner2;
  ant2_segment2.point1 = ant2_corner2;ant2_segment2.point2 = ant2_corner3;
  ant2_segment3.point1 = ant2_corner3;ant2_segment3.point2 = ant2_corner4;
  ant2_segment4.point1 = ant2_corner4;ant2_segment4.point2 = ant2_corner1;
  
  
  bool intersection(0);
  if (!intersection){intersection = get_line_intersection( ant1_segment1,ant2_segment1);}
  if (!intersection){intersection = get_line_intersection( ant1_segment1,ant2_segment2);}
  if (!intersection){intersection = get_line_intersection( ant1_segment1,ant2_segment3);}
  if (!intersection){intersection = get_line_intersection( ant1_segment1,ant2_segment4);}
  if (!intersection){intersection = get_line_intersection( ant1_segment2,ant2_segment1);}
  if (!intersection){intersection = get_line_intersection( ant1_segment2,ant2_segment2);}
  if (!intersection){intersection = get_line_intersection( ant1_segment2,ant2_segment3);}
  if (!intersection){intersection = get_line_intersection( ant1_segment2,ant2_segment4);}
  if (!intersection){intersection = get_line_intersection( ant1_segment3,ant2_segment1);}
  if (!intersection){intersection = get_line_intersection( ant1_segment3,ant2_segment2);}
  if (!intersection){intersection = get_line_intersection( ant1_segment3,ant2_segment3);}
  if (!intersection){intersection = get_line_intersection( ant1_segment3,ant2_segment4);}
  if (!intersection){intersection = get_line_intersection( ant1_segment4,ant2_segment1);}
  if (!intersection){intersection = get_line_intersection( ant1_segment4,ant2_segment2);}
  if (!intersection){intersection = get_line_intersection( ant1_segment4,ant2_segment3);}
  if (!intersection){intersection = get_line_intersection( ant1_segment4,ant2_segment4);}

  return (intersection);
  //==== NS code end =====
}

// =====================================================================================
/**\fn inline void cherche_interaction(framerec& temp, TagsFile& tgs, table* i_table, int j, int d_th, int a_th_par,double width_factor, double width_ratio)
 * \brief Finds all interactions of a given tag (index j in tag_list) with all other tags
 * \param temp Frame recording of current frame
 * \param tgs File .tags with details on each tag
 * \param g Stream for output file
 * \param j Index of tag in tag_list (trackcvt.h file)
 * \param d_th Distance threshold
 * \param a_th_par Angle threshold for paralell ants
 * \param width_factor Factor by which the trapezoid increases in width at the head of the ant compared to the abdomen of the ant
 * \param width_ratio Ratio of the width of the ant compared to the height of the ant 
 */
inline void cherche_interaction(framerec& temp, TagsFile& tgs_md, ofstream& g, int j, int d_th, int a_th_par, double width_factor, double width_ratio){
	for (int k(j+1); k < tag_count; k++){
		if (temp.tags[k].x != -1){
			// check that partner ant is still alive, present in the tag file and, interactions only considered for live tagged ants
			if (tgs_md.get_state(k) && (tgs_md.get_death(k)== 0 || tgs_md.get_death(k) > temp.frame)){
				// if the interaction test return something different from 0 there is an interaction 
				// the test distinguishes 2 types of interaction: unidirectional (return value 1) and bidirectional (return value 2) but we don't distinguish between these interactions for the moment
//      testing <<tag_list[j]<<","<<tag_list[k]<<endl;
        bool interac = test_interaction(temp.tags[j], temp.tags[k], tgs_md.get_rayon(j), tgs_md.get_rayon(k), tgs_md.get_trapezoid_length(j), tgs_md.get_trapezoid_length(k), d_th, a_th_par, width_factor, width_ratio);
				if (interac){ // 
          //cout<<"tag "<<tag_list[j]<<" interacts with tag "<<tag_list[k]<<" in frame "<<temp.frame<<endl;
          g.precision(12);
          g<<temp.time<<","<<temp.frame<<","<<(int)temp.tags[j].id<<","<<tag_list[j]<<","<<tag_list[k]<<",";
					g<<temp.tags[j].x<<","<<temp.tags[j].y<<","<<temp.tags[j].a<<",";
					g<<temp.tags[k].x<<","<<temp.tags[k].y<<","<<temp.tags[k].a<<endl;
				}
			}
		}
	}
}

// =====================================================================================
int main(int argc, char* argv[]){
try{

	if (argc!=8){
		string info = string (argv[0]) + " input.dat mandibles.tags outfile.txt distance(px) angle_paralell(deg) width_factor width_ratio"; //trapezoid.txt interaction_tester.txt tag_call.txt";
		throw Exception (USE, info);
	}
	
 
	// read input parameters and tests whether they are valid
	int d_th = atoi(argv[4]);		//distance threshold : maximum distance between 2 ants to be considered interacting, choice based on visual inspection of videos
	cout << "distance_threshold = " << d_th << endl;
	int a_th_par = atoi(argv[5]);	//angle threshold: minimum angle between 2 ants to be considered interacting
	cout << "angle threshold = " << a_th_par << endl;
	double width_factor = atof(argv[6]);     // Head-width correction factor (determines how wide the trapezoid will be at the ant head)
	cout << "width_factor = " << width_factor << endl;
	double width_ratio = atof(argv[7]);      // ratio width/height of ant (determines the height to average width of trapezoid)
	cout << "width_ratio = " << width_ratio << endl;

	if (d_th < 0){
		string info = "Enter a positiv distance.";
		throw Exception (PARAMETER_ERROR, info);
	}

 	if (a_th_par > 180 || a_th_par < 0){
		string info = "Enter an angle(to define interactions)  between 0 and 180.";
		throw Exception (PARAMETER_ERROR, info);
	}
	
	if (width_factor <= 0){
		string info = "Enter a positive width factor.";
		throw Exception(PARAMETER_ERROR, info);
	}
	
	if (width_ratio <= 0){
		string info = "Enter a positive width ratio.";
		throw Exception(PARAMETER_ERROR, info);
	}
	
	// test if outfile exists already
	ifstream f;
	f.open(argv[3]);
	if (f.is_open()){
		f.close();
		throw Exception (OUTPUT_EXISTS, (string) argv[3]);
	}
	// Open input files
	f.open(argv[1]);
	if (f.fail()){
		string info = string (argv[1]);
		throw Exception (CANNOT_OPEN_FILE, info);
	}
	cout << "datfile opened" <<endl;
	
	TagsFile tgs_md;
	cout << "About to open tag file" << argv[2] <<endl;
	tgs_md.read_file(argv[2]);
	cout << "tagfile opened" <<endl;
	// opens outputfile
	ofstream g;
	g.open(argv[3]);
	if (!g.is_open()){
		f.close();
		throw Exception(CANNOT_OPEN_FILE, (string) argv[3]);
	}
	cout << "outfile opened" <<endl;
	
	g<<"Time,Frame,Box,Ant1,Ant2,Xcoor1,Ycoor1,Angle1,Xcoor2,Ycoor2,Angle2,Direction,From,To"<<endl;
	
	cout<<"start interaction search... "<<endl;
	// read through binary file
	while (!f.eof()){
		
		framerec temp;
		//cout<<"interval 1"<<endl;
		f.read((char*) &temp, sizeof(temp)); 
		for (int j(0); j < tag_count; j++){ 
			// for tags that are in the tags files and not dead/lost tags
			if (tgs_md.get_state(j) && (tgs_md.get_death(j)== 0 || tgs_md.get_death(j) > temp.frame)){
				// for tags that are detected
				if (temp.tags[j].x != -1){
					cherche_interaction(temp, tgs_md, g, j, d_th, a_th_par, width_factor, width_ratio);
				}
			}
		}
	}
	
	g.close();
	f.close();
//  trapezoid.close();
//  interaction_tester.close();
//  testing.close();
	
}catch(Exception e){
	return 1;
}
return 0;
}
