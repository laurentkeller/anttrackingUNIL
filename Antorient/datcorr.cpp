/*
 *  datcorr.cpp
 *  
 *
 *  Created by Danielle Mersch.
 *  Copyright UNIl. All rights reserved.
 *
 */

#include "datcorr.h"


bool distance_to_last(const tag_pos& t, const int frame, const previous& p, double& a, double& dist){
	int dx, dy;
	dx = p.x - t.x;
	dy = p.y - t.y;
	dist = sqrt((double)dx*dx + dy*dy);
	
	a = abs(p.a - t.a)/100;
	if (a > 180){
		a = 360 - a;
	}
	int d = frame - p.f;
	if (d > 1){
		a/=d;
		dist/=d;
	}
	
	if (dist > DIST_MAX){ // distance bigger than maximal distance run by an ant in 1 frame
		return true;
	}else if((a > A_MAX && dist > DIST_MAX2)){
		return true;		
	}
	
	return false;
}


//function correct_tag_position_angle © Nathalie Stroeymeyt
// takes as input "tag" (coordinates and orientation, in degrees * 100, of the tag considered), "param" (calibration parameters for the ant identified by that specific tag) and "new_tag" (coordinates and orientation, in degrees * 100, of the tag considered, will be modified in the function to take into account the calibration parameters)
// returns nothing, but modifies by reference the content of "new_tag" according to the calibration parameters
void correct_tag_position_angle(const tag_pos& tag, const tags* tag_param, tag_pos& new_tag){
	//new orientation angle (in degrees x 100 for output)
	new_tag.a=100*limit_angle((tag.a/100)+ tag_param->rot);
	//displacement angle (in radians for following calculations)
	double displacement_angle(M_PI*limit_angle((tag.a/100)+tag_param->displacement_angle)/180);
	//new coordinates, general case: movement in a direction not parallel to X or y axis
	if ((cos(displacement_angle)!=0)&&(sin(displacement_angle)!=0))
		{
		double slope = 	sin(displacement_angle)/cos(displacement_angle);	
		double change_x(((abs(cos(displacement_angle)))/(cos(displacement_angle)))*((tag_param->displacement_distance)/(sqrt(1+(slope*slope)))) );
		double change_y(change_x*slope);//this is calculated considering a y axis going up; so need to take the opposite when applying the change
		new_tag.x = round(tag.x + change_x);
		new_tag.y = round(tag.y - change_y);
		}
	//new coordinates, if displacement angle parallel to y axis
	if (cos(displacement_angle)==0)
		{
		new_tag.x=tag.x;
		new_tag.y=round(tag.y-sin(displacement_angle)*tag_param->displacement_distance);
		}
	//new coordinates, if displacement angle parallel to x axis
	if (sin(displacement_angle)==0)
		{
		new_tag.y=tag.y;
		new_tag.x=round(tag.x+cos(displacement_angle)*tag_param->displacement_distance);
		}
	//check that coordinates are within limits
	if (new_tag.x < 1){new_tag.x=1;}
	if (new_tag.y < 1){new_tag.y=1;}
	if (new_tag.x >= IMAGE_WIDTH){new_tag.x=IMAGE_WIDTH-1;}
	if (new_tag.y >= IMAGE_HEIGHT){new_tag.y=IMAGE_HEIGHT-1;}
	}


bool execute_datcorr(string datfile, string tagsfile, string output, string logfile, update_status* callback, void* user_pointer){

try{

	// tests whether the output exists already
	DatFile dat_out;
	dat_out.create_dat(output);

	//Reads the .tags file
	TagsFile tgs;
	tgs.read_file(tagsfile.c_str());

	// dynamically allocates space for frames read into buffer
	framerec* buf = new framerec[BUFCOUNT];
	if (buf==NULL) {
		string info = "Unable to allocate memory for file read buffer.";
		delete[] buf;
		throw Exception (BUFFER, info);
	}

	// Opens the datafile
	DatFile dat_in;
	dat_in.open(datfile, 0);
	
	cout<<"last frame in file: "<<dat_in.get_last_frame()<<endl;

	int cnt (0);
	long long cntdel(0);
	long long cntrot(0);
	
	previous last[tag_count];
	memset(&last, 0, sizeof(last));
	bool state[tag_count];
	memset(&state, 0, sizeof(state));		
	dernier der[tag_count];
	memset(&der,0, sizeof(der));	
	int ctr(0);
	int cdead (0);
	
	// file with parameter infos: Program version, command and constants
	ofstream p;
	p.open(logfile.c_str(), ios_base::app);
	if(!p.is_open()){
		throw Exception (CANNOT_OPEN_FILE, logfile);
	}
	time_t t = time(NULL);
	string date = time_to_str(t);
	p<<VERSION<<" on "<<date<<endl;
	p<<"using function execute_datcorr (via antorient) with files: "<<datfile<<", "<<tagsfile<<" and "<<logfile<<" as input and "<<output<<" as output."<<endl;

	while (!dat_in.eof()) {
		
		// reads a maximal number of BUFCOUNT framerecs from input datfile.
		if (!dat_in.read_frame(buf, BUFCOUNT)){
				string info = "Could not read frames into buffer.";
				throw Exception(BUFFER, info);
		}
		
		// callback for progression bar in antorient
		cnt += dat_in.get_count();
		callback(cnt/(double)dat_in.get_last_frame() * 100, user_pointer);
		
		// processes all read frames
		for (unsigned int i(0); i<dat_in.get_count(); i++) {

			// processes all tags in .tags file
			for (int t(0); t<tag_count; t++) {

				if (tgs.get_state(t)){
					// Processing for detected tags 
					if (buf[i].tags[t].x!=-1) {

						// Tag to delete
						if (tgs.get_count(t)==0) {
							buf[i].tags[t].x = -1;
							buf[i].tags[t].y = -1;
							cntdel++;
						}else{
							//Tag to move and rotate
							if (tgs.get_rot(t)!=0) {
                
                tag_pos new_tag;
		new_tag.id = buf[i].tags[t].id;
                correct_tag_position_angle(buf[i].tags[t], tgs.get_taginfo(t), new_tag);
                buf[i].tags[t] = new_tag;
								cntrot++;
							}
						}
						
						// test for false positives
						if (state[t]){  // state is true if tag had been detected once before
							double a (0.0);
							double dist(0.0);
							//calculates distance to last detection and angle difference, if above a threshold return true
							if (distance_to_last(buf[i].tags[t], buf[i].frame, last[t], a, dist)){
								if (dist < DIST_MAX){
									// if the distance and angle differences is identical in to successive detections, then it is a false positiv
									if ((fabs(der[t].a - a) < 2) && (fabs(der[t].d - dist) < 2) && (buf[i].frame - last[t].f < 2)){
										// correct frame
										if (i > 0){
											buf[i-1].tags[t].x = -1;
											buf[i-1].tags[t].y = -3;
										}else{
											dat_out.move(-1);
											framerec temp;
											if (!dat_out.read_frame(temp)){
												//cout<<"frame in buffer: "<<buf[i].frame<<endl; 
												string info = "Could not read frame from file.";
												throw Exception(DATA_ERROR, info);
											}
											temp.tags[t].x = -1;
											temp.tags[t].y = -3;
											dat_out.move(-1);
											dat_out.write_frame(&temp);											
										}
										ctr++; // increases ctr by 2: (++ctr)++
									}	
									der[t].t = tag_list[t];
									der[t].f = buf[i].frame;
									der[t].a = a;
									der[t].d = dist;
								}else{  // falses positives because distance between 2 detections exceeds DIST_MAX pixels
										// an ant cannot run that fast ;)
									ctr++;
									//correct frame
									if (i > 0){
										buf[i-1].tags[t].x = -1;
										buf[i-1].tags[t].y = -3; // put y = -3 for false positives
									}else{
										dat_out.move(-1);
										framerec temp;
										if (!dat_out.read_frame(temp)){
											//cout<<"2. frame in buffer: "<<buf[i].frame<<endl; 
											string info = "Could not read frame from file.";
											throw Exception(DATA_ERROR, info);
										}
										
										temp.tags[t].x = -1;
										temp.tags[t].y = -3;
										dat_out.move(-1);
										dat_out.write_frame(&temp);
									}
								}
							}
						}else{
							state[t] = true;  // state become true if first detection
						}
						// copy infos of detection in last for comparisons
						last[t].f = buf[i].frame;
						last[t].x = buf[i].tags[t].x;
						last[t].y = buf[i].tags[t].y;
						last[t].a = buf[i].tags[t].a;	
						
					// Processing for undetected tags	
					}else if (buf[i].tags[t].x==-1 && tgs.get_last_det(t) < buf[i].frame && dat_in.get_last_frame() - tgs.get_last_det(t) > 7200) {
						buf[i].tags[t].y = -4;  // ant dead (or at least not detected until end of experiment anymore)
						cdead++;
					}
				} // end tgs.get_state(t)
			}  // end tags
		} // end buffer
		
		// writes the modified frames to the output file
		if (!dat_out.write_frame(buf, dat_in.get_count())){
			dat_in.close();
			dat_out.close();
			delete[] buf;
			throw Exception (CANNOT_WRITE_FILE,output);
		}

	}

	//closes the files and deletes the buffer
	dat_in.close();
	dat_out.close();
	delete[] buf;
	
	p <<cnt<<" frames processed, "<<cntrot<<" tags rotated, "<<cntdel<<" removed (0 detections).";
	p<<"false positives: "<<ctr;
	p<<cdead<<" tags declared dead (i.e. not detected anymore) "<<endl;
	p.close();
	tgs.write_file(tagsfile.c_str());
	
	cout<<"Angles rotated with "<<VERSION<<" on "<<date<<endl;
	cout<<cnt<<" frames processed, "<<cntrot<<" tags rotated, "<<cntdel<<" removed (0 detections)."<<endl;
	cout<<"false positives: "<<ctr<<endl;
	cout<<cdead<<" tags declared dead (i.e. not detected anymore) "<<endl;
	
	return true;
}catch(Exception e){
	return false;
}


}
