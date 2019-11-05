/*
 *  datFile.cpp
 *
 *
 *  Created by Danielle Mersch on 8/16/08.
 *  Copyright 2008 __UNIL__. All rights reserved.
 *
 */
#include <iostream>
#include <iomanip>
#include "datfile.h"

//constructor
DatFile::DatFile() {
	firstframe = 0;
	lastframe = 0;
	lasttime = 0.0;
	firsttime = 0.0;
	current = 0;
	currenttime = 0.0;
	count = 0;
	pos = 0;
}

// destructor
DatFile::~DatFile(){
}

//=================== methods =================================
bool DatFile::exists(string nomfichier){
	ifstream g;
	g.open(nomfichier.c_str(), ios::in | ios::binary);
	if (g.fail()){
		return false;
	}
	g.close();
	return true;
}

bool DatFile::copy(string source_filename, string dest_filename) {
	ifstream src(source_filename.c_str(), ios::binary);
	ofstream dst(dest_filename.c_str(), ios::binary);
	dst << src.rdbuf();
	return true;
}

//============================================================================================
void DatFile::open(string nomfichier, const bool write){
	// (write ? ios::out : 0) ->  has value ios::out if write == true, or 0 if false
	//f.open(nomfichier.c_str(), ios::in | (write ? ios::out : (std::_Ios_Openmode) 0) | ios::binary);
	// for visual studio compatibilities (again and again ...) we change to
	if (write){
		f.open(nomfichier.c_str(), ios::in | ios::out | ios::binary);
	}else{
		f.open(nomfichier.c_str(), ios::in | ios::binary);
	}
	if (f.fail()){
		throw Exception (CANNOT_OPEN_FILE, nomfichier);
	}


	// test whether the file is a datfile, i.e. check whether the total number of frames is an integrer
	streampos old = f.tellg();  // keeps current position
	f.seekg(0, ios::end);       // goes to end of file
	f.clear();
	streampos re = f.tellg();   // size of file in bytes
	f.seekg(old, ios::beg);     // resets the pointer to the current position
	if (re % sizeof(framerec) != 0){
		string info = "The fileformat is not a dat format.";
		close();
		throw Exception(DATA_ERROR, info);
	}


	// identify first frame
	framerec temp;
	read_frame(temp);
	firstframe = temp.frame;
	firsttime = temp.time;

  /*
  cout << "sizeof(double) = " << sizeof(double) << endl;
  cout << "sizeof(uint32_t) = " << sizeof(uint32_t) << endl;
  cout << "sizeof(tag_pos) = " << sizeof(tag_pos) << endl;
  cout << "sizeof(framerec.tags) = " << sizeof(temp.tags) << endl;
  cout << "pos(framerec.frame) = " << (int) ((char*)(&temp.frame) - (char*)(&temp.time)) << endl;
  cout << "pos(framerec.tags) = " << (int) ((char*)(&temp.tags) - (char*)(&temp.time)) << endl;
*/

	// identify last frame
	f.seekg(0, ios::end);
 // cout << "size of a framerec is " << sizeof(framerec) << " bytes" << endl;
  //cout << "size of the dat file is " << f.tellg() << " bytes" << endl;
	f.seekg(sizeof(framerec)*((streampos)- 1), ios_base::end);
	read_frame(temp);
	lastframe = temp.frame;
	lasttime = temp.time;
	count = 0;
	//cout<<"lastframe: "<<lastframe<<endl;
	//cout<<"firsframe: "<<firstframe<<endl;

	// clears the flags and goes to beginning of the file
	f.clear();
	f.seekg(0,ios::beg);
	pos = f.tellg();
}

//============================================================================================
bool DatFile::read_frame(framerec& temp){
	f.read((char*) &temp, sizeof(temp));
	current = temp.frame;
	currenttime = temp.time;
	count = 1;
	pos = f.tellg();
	if (f.fail()){
		count = 0;
		return false;
	}
	return true;
}

//============================================================================================
bool DatFile::read_frame(framerec* buffer, const int bufcount){
	f.read((char*)buffer, sizeof(framerec)*bufcount);
	count = f.gcount()/ sizeof(framerec);
	pos = f.tellg();
	if (count == 0){
		return false;
	}
	current = buffer[count - 1].frame;
	currenttime = buffer[count - 1].time;
	return true;
}

//============================================================================================
void DatFile::create_dat(string nomfichier){
	f.open(nomfichier.c_str(), ios::in | ios::binary);
	if (f.is_open()){
		throw Exception(OUTPUT_EXISTS, nomfichier);
	}
	f.close();
	f.clear();
	f.open(nomfichier.c_str(), ios::out | ios::in | ios::binary | ios::trunc);
	if (!f.is_open()){
		throw Exception(CANNOT_OPEN_FILE, nomfichier);
	}
}

//============================================================================================
void DatFile::show_frame(const unsigned int i){
	if (is_valid(i)){
		go_to_frame(i);
		framerec temp;
		read_frame(temp);
		cout<<"Frame = "<<temp.frame<<endl;
		time_t ct = temp.time;
		tm *t = localtime( &ct );  // convert to local
		cout<<"Unix Time = "<<temp.time<<endl;
		cout<<"Date & Time = "<<asctime(t)<<endl;
		for (int i(0); i<tag_count; i++){
			cout<<"Tag: "<<tag_list[i]<<" Box: "<<(int)temp.tags[i].id<<" Position: "<<temp.tags[i].x<<", "<<temp.tags[i].y<<" Angle: "<<temp.tags[i].a<<endl;
		}
	}else{

		cerr<<"Cannot display frame "<<i<<". Frame is out of file range: "<<firstframe<<" to "<<lastframe<<"."<<endl;
	}
}

//============================================================================================
void DatFile::show_frame(const framerec& temp){
	cout<<"Frame = "<<temp.frame<<endl;
	time_t ct = temp.time;
	tm *t = localtime( &ct );  // convert to local
	cout<<"Unix Time = "<<temp.time<<endl;
	cout<<"Date & Time = "<<asctime(t)<<endl;

	for (int i(0); i<tag_count; i++){
		cout<<"Tag: "<<tag_list[i]<<" Box: "<<(int)temp.tags[i].id<<" Position: "<<temp.tags[i].x<<", "<<temp.tags[i].y<<" Angle: "<<temp.tags[i].a<<endl;

	}
}

//============================================================================================
void DatFile::show_tag(const int& tag, const unsigned int& frame){
	int idx =get_tag_index(tag);
	if (is_valid(frame)){
		go_to_frame(frame);
		framerec temp;
		read_frame(temp);
		cout<<"At frame "<<temp.frame<<" tag "<<tag_list[idx]<<" is in box"<<(int)temp.tags[idx].id<<" at position: "<<temp.tags[idx].x<<","<<temp.tags[idx].y<<" with angle: "<<temp.tags[idx].a<<" ."<<endl;
	}else{
		cerr<<"Cannot display tag for frame "<<frame<<". Frame is out of file range: "<<firstframe<<" to "<<lastframe<<"."<<endl;
	}
}

//============================================================================================
void DatFile::show_tag(const int& tag, const framerec& temp){
	int idx =get_tag_index(tag);
	cout<<"At frame "<<temp.frame<<" tag "<<tag_list[idx]<<" is in box"<<(int)temp.tags[idx].id<<" at position: "<<temp.tags[idx].x<<","<<temp.tags[idx].y<<" with angle: "<<temp.tags[idx].a<<" ."<<endl;
}

//============================================================================================
bool DatFile::go_to_frame(const unsigned int fr){
	if(is_valid(fr)){
		f.seekg(sizeof(framerec)*((streampos)(fr - firstframe)), ios_base::beg);
		pos = f.tellg();
		return true;
	}else{
		cerr<<"Cannot go to frame "<<fr<<". Frame is out of file range: "<<firstframe<<" to "<<lastframe<<"."<<endl;
		return false;
	}
}

//============================================================================================
void DatFile::go_to_streampos(streampos p){
	f.seekg(p);
	pos = f.tellg();
}

//============================================================================================
void DatFile::go_to_time(double time){
	if (is_valid_time(time)){
		// calculate approximate position in file with frame coorespondance
		double time_from_start = time - firsttime;
		int fr = time_from_start*2;
		f.seekg(sizeof(framerec)*((streampos)(fr)), ios_base::beg);
		pos = f.tellg();
		// read frame and move backwards of forwards depending on time in frame
		framerec temp;
		read_frame(temp);
		if ((int)temp.time > time){	// move backwards
			do {
				move(-2);
				read_frame(temp);
			}while((int)temp.time > time);
			move(-1);
		}else if (temp.time < (int)time){ // move forwards
			do{
				read_frame(temp);
			}while (temp.time < (int)time);
			move(-1);
		}else{
			move(-1);
		}
	}else{
		cerr<<"Cannot go to time "<<time<<". The time is out of the file range: "<<firsttime<<" to "<<lasttime<<"."<<endl;
	}
}

//============================================================================================
void DatFile::move(const int x){
	f.seekg(sizeof(framerec)*((streampos) x), ios_base::cur);
	pos = f.tellg();
}

//============================================================================================
unsigned long DatFile::get_frame_count(){
	if (!f.is_open()){
		return 0; // tests whether the file is open
	}
	return (lastframe - firstframe +1);
}

//============================================================================================
unsigned int DatFile::get_last_frame(){
	return	lastframe;
}

//============================================================================================
unsigned int DatFile::get_first_frame(){
	return	firstframe;
}

//============================================================================================
unsigned int DatFile::get_current_frame(){
	return current;
}

//============================================================================================
unsigned int DatFile::get_count(){
	return count;
}

//============================================================================================
streampos DatFile::get_streampos(){
	return pos;
}

//============================================================================================
double DatFile::get_first_time(){
	return firsttime;
}
//============================================================================================
double DatFile::get_last_time(){
	return lasttime;
}

//============================================================================================
double DatFile::get_current_time(){
	return currenttime;
}

//============================================================================================
bool DatFile::is_valid(const unsigned int frame){
	if (frame >= firstframe && frame <= lastframe){
		return true;
	}
	return false;
}

//============================================================================================
bool DatFile::is_valid_time(const double time){
	cout << std::setprecision(12) << "Tested time = " << time << "; First time = " << firsttime << "; Last time = " << lasttime << endl;
	if (time >= firsttime && time <= lasttime){
		return true;
	}
	return false;
}

//============================================================================================
bool DatFile::find_next_frame_with_tag(const int& tag, int& frame){
	int idx =get_tag_index(tag);
	frame = -1;
	if (idx == -1){
		return false;
	}
	if(find_next_frame_with_index(idx, frame)){
		return true;
	}
	return false;
}

//============================================================================================
bool DatFile::find_previous_frame_with_tag(const int& tag, int& frame){
	int idx =get_tag_index(tag);
	frame = -1;
	if (idx == -1){
		return false;
	}
	if(find_previous_frame_with_index(idx, frame)){
		return true;
	}
	return false;
}

//============================================================================================
bool DatFile::find_last_frame_with_tag(const int& tag, int& frame){
	int idx =get_tag_index(tag);
	frame = -1;
	if (idx == -1){
		return false;
	}
	if (find_last_frame_with_index(idx, frame)){
		return true;
	}
	return false;
}

//============================================================================================
bool DatFile::find_first_frame_with_tag(const int& tag, int& frame){
	f.seekg(0,ios::beg);
	pos = f.tellg();
	frame = -1;
	int idx =get_tag_index(tag);

	if (find_first_frame_with_index(idx, frame)){
		return true;
	}
	return false;
}

//============================================================================================
bool DatFile::find_next_frame_with_index(const int& idx, int& frame){
	frame = -1;
	if (idx == -1){
		return false;
	}
	do{
		framerec temp;
		read_frame(temp);
		if (!f.fail() && temp.tags[idx].x != -1){
			frame = temp.frame;
			return true;
		}
	}while(frame == -1 && !f.eof());
	f.clear();
	return false;
}

//============================================================================================
bool DatFile::find_previous_frame_with_index(const int& idx, int& frame){
	frame = -1;
	if (idx == -1){
		return false;
	}
	do{
		move(-2);
		framerec temp;
		read_frame(temp);
		if (!f.fail() && temp.tags[idx].x !=-1){
			frame = temp.frame;
			//cout<<temp.frame<<endl;
			return true;
		}
	}while(frame == -1 && !f.fail());
	f.clear();
	return false;

}

//============================================================================================
bool DatFile::find_last_frame_with_index(const int& idx, int& frame){
	frame = -1;
	if (idx == -1){
		return false;
	}
	do{
		framerec temp;
		read_frame(temp);
		if (!f.fail() && temp.tags[idx].x != -1){
			frame = temp.frame;
			return true;
		}
	}while(frame == -1 && !eof());
	f.clear();
	return false;
}

//============================================================================================
bool DatFile::find_first_frame_with_index(const int& idx, int& frame){
	f.seekg(0,ios::beg);
	pos = f.tellg();
	frame = -1;
	if (idx == -1){
		return false;
	}
	do{
		framerec temp;
		read_frame(temp);
		if (!f.fail() && temp.tags[idx].x != -1){
			frame = temp.frame;
			return true;
		}
	}while(frame == -1 && !f.fail());
	f.clear();
	return false;
}

//============================================================================================
void DatFile::close(){
	f.close();
}

//============================================================================================
bool DatFile::eof(){
  return f.eof();
}

//============================================================================================
void DatFile::clear(){
	f.clear();
}

//============================================================================================
bool DatFile::bad(){
	return f.bad();
}

//============================================================================================
bool DatFile::write_frame(const framerec* temp, const int a){

	f.write((char*) temp, sizeof(framerec)*a);
	if (f.fail()){
		f.clear();
		return false;
	}
	return true;
}


bool DatFile::write_tag(unsigned int frame, tag_pos &tag) {
	if (go_to_frame(frame) == false)
		return false;
	framerec temp;
	if (read_frame(temp) == false)
		return false;
	for (int i(0); i < tag_count; i++){
		if (temp.tags[i].id == tag.id) {
			temp.tags[i].x = tag.x;
			temp.tags[i].y = tag.y;
			temp.tags[i].a = tag.a;
			break;
		}
	}
	if (write_frame(&temp, 1) == false)
		return false;
	return true;
}

//============================================================================================
int DatFile::get_tag_index(const int& tag){
	int idx (-1);
	for (int i(0); i < tag_count; i++){
		if (tag_list[i] == tag){
			idx = i;
			return idx;
		}
	}
	if (idx == -1){
		cerr<<"Non valid tag id."<<endl;
		return idx;
	}
}
