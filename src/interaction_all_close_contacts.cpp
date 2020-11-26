  /*
 *  interaction_all_close_conta.cpp
 *  --> reads through dat file and check for every frame and every pair of tags whether they are interacting. If so they are written to and outfile
 *
 *  Created by Danielle Mersch and Alessandro Crespi in 2010.
 *  Modified by Nathalie Stroeymeyt (NS) in 2016.
 *  Copyright UNIL and Alessandro Crespi. All rights reserved. 
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

//==== NS code start =====
struct interaction_type{
	int direction;
	string to;
	string from;
};
struct is_interaction{
	bool in;
	string to;
	string from;
};
//==== NS code end =====
// =====================================================================================
// =====================================================================================
/**\fn bool is_in_trapezoid(int xc, int yc, int a, int w1, int w2, int h, int x, int y)
 * \brief Test whether the point x,y is in the trapezoid of height 2h and width w1(head) and w2(abdomen);
 the 'height' of the trapezoid is respective to the ant turned with the head at 90deg (top of image)
 the trapezoid is centered at (xc,yc) with the w1 side (modeled head of ant) oriented  at 90deg
 * \param xc X-coordiante of tag of ant modeled as trapezoid
 * \param yc Y-coordiante of tag of ant modeled as trapezoid
 * \param a Angle of ant modeled as trapezoid
 * \param w1 Width of trapezoid at the head end of the ant modeled (= radius * width_ratio *width_factor)
 * \param w2 Width of trapezoid at the abdomen end of the ant modeled (= radius * width_ratio)
 * \param ha antennal reach of ant modeled as trapezoid
 * \param trapezoid length of ant modeled as trapezoid
 * \param x X-coordinate of interaction point
 * \param y Y-coordinate of interaction point
 */
is_interaction is_in_trapezoid(int xc, int yc, int a, double w1, double w2, double ha, double tl,int x, int y, string from){
  //==== NS code start =====	
	is_interaction output;
	output.from = from;
	//==== NS code end =====
	// Centers the tested point
	double xt = x - xc;
	double yt = y - yc;
	
	//Calculate rotation angle to have trapezoid facing "up" (i.e. with head at 90deg (top of image)
	double ar = ((double) (90-a) * M_PI / 180.0);
	
	// Calculate rotation parameters
	double ca = cos(ar);
	double sa = -sin(ar);
	
	// Apply rotation to centered, tested point
	double xtr = xt * ca - yt * sa;
	double ytr = xt * sa + yt * ca;
	
	//==== NS code start =====
	//Remember that the yaxis goes down!
	//condition on ytr: 	if ytr <= 0 (i.e. tested point "between" tag and antennal reach), then we want abs(ytr) smaller than antennal reach
	//			if ytr > 0 (i.e. tested point "between" tag and gaster), then we want ytr smaller than (trapezoid length - antennal reach)
	//so we create this ycondition variable, that has to be smaller than 0
	double ycondition;
	if (ytr <=0){ycondition= abs(ytr)-ha;}
	
	if (ytr >0){ycondition=ytr-(tl-ha);}
  
	// condition on xtr: we need to calculate the half-width of the trapezoid for this ytr.
	// equation of the line joining bottom right and top right corners of the trapezoid
	double wtr((w1 + (w2 - w1) * ((ytr+ha) / tl))/2);
  
	output.in = (abs(xtr) < wtr && ycondition < 0);
	if (!output.in){//if no interaction, to = NA
		output.to = "NA";
	}
	if (output.in){//if no interaction, output whether interaction in front half or back half
		if (ytr <=0){//if tested point "between" tag and antennal reach
			output.to = "H";
		}
		if (ytr >0){//if tested point "between" tag and gaster
			output.to = "B";
		}
	}
	return output;
	//==== NS code end =====
}


// =====================================================================================
/**\fn int test_interaction(tag_pos pt1, tag_pos pt2, int size1, int size2, int d_th, int a_th_par, int a_th_beh, double width_factor, double width_ratio)
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
 * \return 2 if there is no interaction, 1 if ant 1 is trapezoid of other, -1 if ant 2 is in trpaezoid of other and 0 if both are in trapezoids
 */
interaction_type test_interaction(tag_pos pt1, tag_pos pt2, int size_md1, int size_md2, double tl_md1, double tl_md2, int d_th, int a_th_par, double width_factor, double width_ratio, double a_th, double interval_a){
  //==== NS code start =====
	interaction_type no_interaction = {2,"NA","NA"}; 
  //==== NS code end =====
	// if no position data for one ant only or if ants in 2 different boxes, no interaction is possible
	if (pt1.x == -1 || pt2.x==-1 || pt1.id != pt2.id){
		return no_interaction;
	}
	// angles (orientation vector) in degres and distance calculation
	double a1 = ((double) pt1.a / 100.0);
	double a2 = ((double) pt2.a / 100.0);
	double dx = pt2.x - pt1.x;
	double dy = pt2.y - pt1.y;
	double dist = sqrtf((double)dx*dx + (double)dy*dy);   // distance

	// si (distance < (Antenna reach 1+ Antenna reach 2 +threshold)) alors interaction possible, autrement non
	if (dist >= size_md1 + size_md2 + d_th){
		return no_interaction;
	}

	// Calculates angle difference between both orientation vectors of ants
	double da = abs(limit_angle(a2 - a1));

	// if the angle difference is bigger the the parallel threshold, then an interaction is possible, 
	// otherwise ants are more or less paralell and unlikly to interact 
	if (da < a_th_par){
		return no_interaction;
	}
  
  	// test whether the interaction points of ant2 are within trapezoid of ant1
  	double delta_a(- a_th);
  	is_interaction in2={false,"NA"};//initialise test_interaction

  	/////first test if the mandible tip of ant2 is within trapezoid of ant1; do it if no interaction has been found using the gaster tip
  	do {
  		double a2r = (a2 + delta_a) * M_PI / 180.0; 	// Convert angle to radians for trigonometry
    		double pt2e_x = pt2.x + size_md2 * cos(a2r);  	// calculates an "interaction point" for the ant
    		double pt2e_y = pt2.y - size_md2 * sin(a2r);
    		in2 = (is_in_trapezoid(pt1.x, pt1.y, a1, tl_md1/2 * width_ratio * width_factor, tl_md1/2 * width_ratio, size_md1, tl_md1, pt2e_x, pt2e_y,"M"));
       		delta_a += interval_a;
  	}while (!in2.in && delta_a < a_th);

  	if (!in2.in){
		delta_a = - a_th;
  	  //==== NS code start =====
	 	/////second test if the gaster tip of ant2 is within trapezoid of ant1
	  	do {
			double a2r = (a2 + delta_a) * M_PI / 180.0; 	// Convert angle to radians for trigonometry
			a2r= M_PI + a2r;//modify a2r to point to the direction of the gaster instead of the position of the front; do this by adding PI constant (does mater for the modulo)
	    		double pt2e_x = pt2.x + (tl_md2 - size_md2) * cos(a2r);  	// calculates an "interaction point" for the ant
	    		double pt2e_y = pt2.y - (tl_md2 - size_md2) * sin(a2r);
	    		in2 = (is_in_trapezoid(pt1.x, pt1.y, a1, tl_md1/2 * width_ratio * width_factor, tl_md1/2 * width_ratio, size_md1, tl_md1, pt2e_x, pt2e_y,"G"));
	       		delta_a += interval_a;
	  	}while (!in2.in && delta_a < a_th);
	}
  	//==== NS code end =====
  	/////third test if the mandible tip of ant1 is within trapezoid of ant2

  	delta_a = - a_th;	
  	is_interaction in1={false,"NA"};//initialise test_interaction
  	
	do {
		double a1r = (a1 + delta_a) * M_PI / 180.0;
    		double pt1e_x = pt1.x + size_md1 * cos(a1r);
    		double pt1e_y = pt1.y - size_md1 * sin(a1r);  // -sin as y axis is turned downwards
    		in1 = (is_in_trapezoid(pt2.x, pt2.y, a2, tl_md2/2 * width_ratio * width_factor, tl_md2/2 * width_ratio, size_md2, tl_md2, pt1e_x, pt1e_y,"M"));
    		delta_a += interval_a;
  	}while (!in1.in && delta_a < a_th);

  	if (!in1.in){
		delta_a = - a_th;
  	  //==== NS code start =====
	  	/////fourth test if the gaster tip of ant1 is within trapezoid of ant2
	 	do {
			double a1r = (a1 + delta_a) * M_PI / 180.0; 	// Convert angle to radians for trigonometry
			a1r= M_PI + a1r;//modify a1r to point to the direction of the gaster instead of the position of the front; do this by adding PI constant (does mater for the modulo)
	    		double pt1e_x = pt1.x + (tl_md1 - size_md1) * cos(a1r);  	// calculates an "interaction point" for the ant
	    		double pt1e_y = pt1.y - (tl_md1 - size_md1) * sin(a1r);
	    		in1 = (is_in_trapezoid(pt2.x, pt2.y, a2, tl_md2/2 * width_ratio * width_factor, tl_md2/2 * width_ratio, size_md2, tl_md2, pt1e_x, pt1e_y,"G"));
	       		delta_a += interval_a;
	  	}while (!in1.in && delta_a < a_th);
	}


	/////now check the results 
	if (!in1.in && !in2.in){
		return no_interaction;
	}

 	if(!in1.in && in2.in){
		interaction_type interaction = {-1,in2.to,in2.from};
		return interaction;

  	}else if (in1.in && !in2.in){
		interaction_type interaction = {1,in1.to,in1.from}; 
		return interaction;

  	}else if (in1.in && in2.in){
		interaction_type interaction;
		interaction.direction=0;
		if (in1.from=="M"||(in1.from=="G"&&in2.from!="M")){//make sure you indicate if this is a close contact for at least one of the ants
			interaction.from = in1.from;
			interaction.to = in1.to;
		}else{
			interaction.from = in2.from;
			interaction.to = in2.to;
		} 
		return interaction;
  	}
  	//==== NS code end =====
}

// =====================================================================================
/**\fn inline void cherche_interaction(framerec& temp, TagsFile& tgs, table* i_table, int j, int d_th, int a_th_par, int a_th_beh, double width_factor, double width_ratio)
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
inline void cherche_interaction(framerec& temp, TagsFile& tgs_md, ofstream& g, int j, int d_th, int a_th_par, double width_factor, double width_ratio, double a_th, double interval_a){
	for (int k(j+1); k < tag_count; k++){
		if (temp.tags[k].x != -1){
			// check that partner ant is still alive, present in the tag file and, interactions only considered for live tagged ants
			if (tgs_md.get_state(k) && (tgs_md.get_death(k)== 0 || tgs_md.get_death(k) > temp.frame)){
				// if the interaction test return something different from 0 there is an interaction 
				// the test distinguishes 2 types of interaction: unidirectional (return value 1) and bidirectional (return value 2) but we don't distinguish between these interactions for the moment
//      testing <<tag_list[j]<<","<<tag_list[k]<<endl;
        interaction_type interac = test_interaction(temp.tags[j], temp.tags[k], tgs_md.get_rayon(j), tgs_md.get_rayon(k), tgs_md.get_trapezoid_length(j), tgs_md.get_trapezoid_length(k), d_th, a_th_par, width_factor, width_ratio, a_th, interval_a);
				if (interac.direction != 2){ // 2 = no interaction
          //cout<<"tag "<<tag_list[j]<<" interacts with tag "<<tag_list[k]<<" in frame "<<temp.frame<<endl;
          g.precision(12);
          g<<temp.time<<","<<temp.frame<<","<<(int)temp.tags[j].id<<","<<tag_list[j]<<","<<tag_list[k]<<",";
					g<<temp.tags[j].x<<","<<temp.tags[j].y<<","<<temp.tags[j].a<<",";
					g<<temp.tags[k].x<<","<<temp.tags[k].y<<","<<temp.tags[k].a<<","<<interac.direction<<","<<interac.from<<","<<interac.to<<endl;
				}
			}
		}
	}
}

// =====================================================================================
int main(int argc, char* argv[]){
try{

	if (argc!=10){
		string info = string (argv[0]) + " input.dat mandibles.tags outfile.txt distance(px) angle_paralell(deg) width_factor width_ratio delta_angle(degree) angle_interval(degree)"; //trapezoid.txt interaction_tester.txt tag_call.txt";
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
  	double a_th = atof(argv[8]); ///<  angle delta for arc calculation in degrees, a_th needs to be a multiple of interval_a, if = 0, only interaction point, no arc
	cout << "angle delta = " << a_th << endl;
  	double interval_a = atof(argv[9]); ///< interval angle at which interaction points are calculated, can only be =0 if a_th is equal to zero!
	cout << "interval = " << interval_a << endl;

  
	if (d_th < 0){
		string info = "Enter a positiv distance.";
		throw Exception (PARAMETER_ERROR, info);
	}

  if (a_th != 0 && interval_a == 0){
    string info = "Invalid parameter combination: if delta angle differs from zero, the angle_interval cannot be zero.";
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
	
	TagsFile tgs_md;
	tgs_md.read_file(argv[2]);

	// opens outputfile
	ofstream g;
	g.open(argv[3]);
	if (!g.is_open()){
		f.close();
		throw Exception(CANNOT_OPEN_FILE, (string) argv[3]);
	}
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
					cherche_interaction(temp, tgs_md, g, j, d_th, a_th_par, width_factor, width_ratio, a_th, interval_a);
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
