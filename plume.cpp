/*
 *  plume.cpp
 *  
 *
 *  Created by Danielle Mersch on 24/6/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <fstream>
#include <sstream>
#include "plume.h"
#include "exception.h"

// plume file

const int REDUCED_W = 608;
const int REDUCED_H = 912;
const int REDUCED_SIZE = REDUCED_W * REDUCED_H;

Plume::Plume()
{
  bitmap = new uint8_t[REDUCED_SIZE]; // alloue bitmap et initialise tout à 255
  memset(bitmap,255,REDUCED_SIZE);
  
  for (unsigned int i(0); i < NUMBER_LINES_COLOR; i++) {
    LUT_state[i] = false;
  }
  firstframe = 0;
  lastframe = 0;
}

Plume::~Plume()
{
  delete[] bitmap;
}

//====================================================================================
bool Plume::read_plume(const string& plumefile){
	ifstream f;
	f.open(plumefile.c_str(), ios::binary);
	if (!f.is_open()){
		cout<<"Cannot open file."<<endl;
		return false;
	}
	string s; //< string in which we read the data
	
	// Frames of validity of plume file, format: firstframe=xxxxxxxx lastframe=xxxxxxxx (on two lines)
	getline(f, s);
	rtrim(s);
	string::size_type n = s.find_first_of('=');
	if (n == string::npos){
		cout<<"The file "<<plumefile<<" is in an unsupported format."<<endl;
		return false;
	}
	int start = atoi(s.substr(n+1).c_str());
	getline(f, s); 
	rtrim(s);
	n = s.find_first_of('=');
	if (n == string::npos){
		cout<<"The file "<<plumefile<<" is in an unsupported format."<<endl;
		return false;
	}
	int stop = atoi(s.substr(n+1).c_str());
	
	if (stop<start){
		cout<<"In the file "<<plumefile<<" the validity period is errounous. The stop frame precedes the start frame."<<endl;
		return false;
	}else{
		firstframe =start;
		lastframe = stop;
	}
	
	// NUMBER_LINES_COLOR
	getline(f, s); 
	rtrim(s);
	s = s.substr(1);
	int nb (atoi(s.c_str()));
	
	if (nb != NUMBER_LINES_COLOR){
		cout<<"Number of color lines wrong."<<endl;
		return false;
	}
	
	// yellow
	getline(f, s); 
	rtrim(s);
	n = s.find_first_of('=');
	if (n == string::npos){
		cout<<"The file "<<plumefile<<" is in an unsupported format."<<endl;
		return false;
	}
	
	zone[YELLOW-1] = s.substr(n+1);
	
	//ORANGE
	getline(f, s); 
	rtrim(s);
	n = s.find_first_of('=');
	if (n == string::npos){
		cout<<"The file "<<plumefile<<" is in an unsupported format."<<endl;		
		return false;
	}
	zone[ORANGE-1] = s.substr(n+1);
	
	//RED
	getline(f, s); 
	rtrim(s);
	n = s.find_first_of('=');
	if (n == string::npos){
		cout<<"The file "<<plumefile<<" is in an unsupported format."<<endl;		
		return false;
	}
	zone[RED-1] = s.substr(n+1);
	
	//PURPLE
	getline(f, s); 
	rtrim(s);
	n = s.find_first_of('=');
	if (n == string::npos){
		cout<<"The file "<<plumefile<<" is in an unsupported format."<<endl;		
		return false;
	}
	zone[PURPLE-1] = s.substr(n+1);
	
	//BLUE
	getline(f, s); 
	rtrim(s);
	n = s.find_first_of('=');
	if (n == string::npos){
		cout<<"The file "<<plumefile<<" is in an unsupported format."<<endl;		
		return false;
	}
	zone[BLUE -1] = s.substr(n+1);
	
	//GREEN
	getline(f, s); 
	rtrim(s);
	n = s.find_first_of('=');
	if (n == string::npos){
		cout<<"The file "<<plumefile<<" is in an unsupported format."<<endl;		
		return false;
	}
	zone[GREEN -1] = s.substr(n+1);
	
	f.read((char*) bitmap, REDUCED_SIZE);
	f.close();
	
	clear_LUT();
	
	return true;
}

//========================================================================
bool Plume::write_plume(const string& plumefile){
  ofstream f;
  f.open(plumefile.c_str(), ios::out | ios::binary);
  if (!f.is_open()) {
    cerr << "Could not open " << plumefile << "." << endl;
    return false;
  }	
	f<<"# firstframe="<<firstframe<<endl;
	f<<"# lastframe="<<lastframe<<endl;
	f<<"# "<<NUMBER_LINES_COLOR<<endl;
	f<<"# "<<YELLOW<<"="<<zone[YELLOW - 1]<<endl;
	f<<"# "<<ORANGE<<"="<<zone[ORANGE - 1]<<endl;
	f<<"# "<<RED<<"="<<zone[RED - 1]<<endl;
	f<<"# "<<PURPLE<<"="<<zone[PURPLE - 1]<<endl;
	f<<"# "<<BLUE<<"="<<zone[BLUE - 1]<<endl;
	f<<"# "<<GREEN<<"="<<zone[GREEN - 1]<<endl;
  f.write((const char*) bitmap, REDUCED_SIZE);
  f.close();
  return true;
}

//========================================================================
int Plume::get_firstframe(){
	return firstframe;
}
//========================================================================
int Plume::get_lastframe(){
	return lastframe;
}
//========================================================================
bool Plume::init_LUT(const int code){
  if (code <= 0 || code > NUMBER_LINES_COLOR) {
    return false;
  }
	for (int x(0); x < IMAGE_WIDTH; x++){
		cout<<"-> x: "<<x<<" of "<<IMAGE_WIDTH<<endl;
		for (int y(0); y < IMAGE_HEIGHT; y++){
			//cout<<"--> y: "<<y<<" of "<<IMAGE_HEIGHT<<endl;
			position p(x,y);
			double d(DOUBLE_MAX);
			for (int i(0); i < REDUCED_SIZE; i++){
				//cout<<"initializing.."<<endl;
				
				if(bitmap[i] == code){
					//cout<<"calculating for "<<i<<"/"<<REDUCED_SIZE<<"..."<<endl;
					double temp = calculate_dist(p, i);
					if (temp <d){
						d = temp;
					}
				}
			}
			LUT[code - 1][p] = sqrt(d);
		}
	}
}

//========================================================================
bool Plume::init_LUT(const string zonename){
  int code;
  if (zone_to_code(zonename, code)) {
    return init_LUT(code);
  }
  return false;
}

//========================================================================
/**\fn double calculate_dist(const position& p, int bitmap_index)
 * \brief Calculates distances between a zone position and a position in the image
 * \param p Position in the image
 * \param bitmap_index Position of the zone point
 * \return Square of distance between the 2 points
 */
double Plume::calculate_dist(const position& p, int bitmap_index){
	int y = (bitmap_index / REDUCED_W) * 5;
	int x = (bitmap_index % REDUCED_W) * 5;
	double dx = x - p.x;
	double dy = y - p.x;
	return (dx*dx + dy*dy);
}

//========================================================================
void Plume::set_firstframe(const int frame){
  firstframe = frame;
  if (frame > lastframe) {
    lastframe = frame;
  }
}

//========================================================================
void Plume::set_lastframe(const int frame){
  lastframe = frame;
  if (frame < firstframe) {
    firstframe = frame;
  }
}

//========================================================================
uint8_t* Plume::get_bitmap(){
  return bitmap;
}

//========================================================================
int Plume::size(){
	return REDUCED_SIZE;
}

//========================================================================
bool Plume::zone_to_code(const string zonename, int& code){
 for (unsigned int i(0); i < NUMBER_LINES_COLOR; i++) {
    if (zone[i].compare(zonename) == 0) {
      code = i + 1;
      return true;
    }
  }
  return false;
}

//========================================================================
const lut_type& Plume::get_LUT(const int code){
	
  if (code <= 0 || code > NUMBER_LINES_COLOR) {
    ostringstream os;
    os << "Invalid zone code: " << code;
    throw Exception(PARAMETER_ERROR, os.str());
  }
  if (!LUT_state[code - 1]) {
    throw Exception(INTERNAL_ERROR, "The requested LUT is not initialized");
  }
  return LUT[code - 1];
}

//========================================================================
const lut_type& Plume::get_LUT(const string zonename){
  int code;
  if (zone_to_code(zonename, code)) {
    return get_LUT(code);
  }
  throw Exception(PARAMETER_ERROR, string("Invalid zone name: ") + zonename);
}
  
//========================================================================
int Plume::exists(string codename){
	for (int i(0); i< NUMBER_LINES_COLOR; i++){
		if (zone[i] == codename){
			return (i+1);
		}
	}
	return -1;
}

//========================================================================
bool Plume::load_LUT(const string filename, const int code){

  if (LUT_state[code - 1]) {
    throw Exception(INTERNAL_ERROR, "LUT already existing");
  }

  ifstream f;
  f.open(filename.c_str(), ios::in | ios::binary);
  if (!f.is_open()) {
    throw Exception(CANNOT_OPEN_FILE, filename);
  } 
  string s;
  getline(f, s);
  if (s.compare("# ZoneLUT") != 0) {
    throw Exception(CANNOT_READ_FILE, "invalid format 1");
  }
  getline(f, s);
  if (s.empty() || s[0] != '#') {
    throw Exception(CANNOT_READ_FILE, "invalid format 2");
  }
  
  istringstream is;
  is.str(s);
  is.ignore(1, '#');
  int c;
  is >> c;
  if (c != code) {
    ostringstream os;
    os << "File is a LUT for code " << c << ", but the program wanted to load it on code " << code << ".";
    throw Exception(DATA_ERROR, os.str());
  }
  
  getline(f, s);
  if (s.empty() || s[0] != '#') {
    throw Exception(CANNOT_READ_FILE, "invalid format 3");
  }
  s = s.substr(2);
  if (s.compare(zone[code - 1]) != 0) {
    ostringstream os;
    os << "File is a LUT for zone " << s << ", but the loaded plume file names it " << zone[code - 1] << ".";
    throw Exception(DATA_ERROR, os.str());
  }

  getline(f, s);
  if (s.empty() || s[0] != '#') {
    throw Exception(CANNOT_READ_FILE, "invalid format 4");
  }
  is.clear();
  is.str(s);
  is.ignore(1, '#');
  is >> c;
  if (c > firstframe) {
    ostringstream os;
    os << "File is a LUT starting at frame " << c << ", but the loaded plume file starts at " << firstframe << ".";
    throw Exception(DATA_ERROR, os.str());
  }

  getline(f, s);
  if (s.empty() || s[0] != '#') {
    throw Exception(CANNOT_READ_FILE, "invalid format 5");
  }
  is.clear();
  is.str(s);
  is.ignore(1, '#');
  is >> c;
  if (c < lastframe) {
    ostringstream os;
    os << "File is a LUT ending at frame " << c << ", but the loaded plume file ends at " << lastframe << ".";
    throw Exception(DATA_ERROR, os.str());
  }
  
  double* buffer = new double[REDUCED_SIZE];
  f.read((char*) buffer, sizeof(double) * REDUCED_SIZE);
  
  double* bptr = buffer;
  for (unsigned int y(0); y < REDUCED_H; y++) {
    for (unsigned int x(0); x < REDUCED_W; x++) {
      position p(x, y);
      LUT[code - 1][p] = *bptr;
      bptr++;
    }
  }
  delete[] buffer;
  LUT_state[code - 1] = true;

  return true;
}

//========================================================================
bool Plume::load_LUT(const string filename, const string zonename){
  int code;
  if (zone_to_code(zonename, code)) {
    return load_LUT(filename, code);
  }
  return false;
}

//========================================================================  
bool Plume::save_LUT(const string filename, const int code){
  if (code <= 0 || code > NUMBER_LINES_COLOR) {
    return false;
  }
  if (!LUT_state[code - 1]) {
    throw Exception(INTERNAL_ERROR, "Could not save uninitialized LUT");
  }
  double* buffer = new double[REDUCED_SIZE];
  double* bptr = buffer;
  for (unsigned int y(0); y < REDUCED_H; y++) {
    for (unsigned int x(0); x < REDUCED_W; x++) {
      position p(x, y);
      *bptr = LUT[code - 1][p];
      bptr++;
    }
  }
  ofstream f;
  f.open(filename.c_str(), ios::out | ios::binary);
  if (!f.is_open()) {
    throw Exception(CANNOT_OPEN_FILE, filename);
  }
  f << "# ZoneLUT" << endl;
  f << "# " << code << endl;
  f << "# " << zone[code - 1] << endl;
  f << "# " << firstframe << " firstframe" << endl;
  f << "# " << lastframe << " lastframe" << endl;
  f.write((const char*) buffer, sizeof(double) * REDUCED_SIZE);
  delete[] buffer;
  return true;
}

//========================================================================
bool Plume::save_LUT(const string filename, const string zonename){
  int code;
  if (zone_to_code(zonename, code)) {
    return save_LUT(filename, code);
  }
  return false;
}

//========================================================================
void Plume::clear_LUT(){
  for (int i(1); i <= NUMBER_LINES_COLOR; i++) {
    clear_LUT(i);
  }
}

//========================================================================
void Plume::clear_LUT(const int code){
  if (code > 0 && code <= NUMBER_LINES_COLOR) {
    LUT_state[code - 1] = false;
  }
}

//========================================================================
int Plume::get_code(position p){
  if (p.x < 0 || p.x > IMAGE_WIDTH || p.y < 0 || p.y > IMAGE_HEIGHT) {
	cout << "p.x = " << p.x << ";p.y = " << p.y << "; IMAGE_WIDTH= " << IMAGE_WIDTH << "; IMAGE_HEIGHT = " << IMAGE_HEIGHT << endl;
    throw Exception(DATA_ERROR, "Invalid image coordinates");
  }
  p.x /= 5;
  p.y /= 5;
  // test for border cases (pixels that are not in the low resolution images)
  if (p.x > REDUCED_W) {
    p.x = REDUCED_W;
  }
  if (p.y > REDUCED_H) {
    p.y = REDUCED_H;
  }
  unsigned int idx = p.x + (p.y * REDUCED_W);
  return bitmap[idx];
}

//========================================================================
int Plume::get_code(const int idx){
	if (idx < 0 || idx > (REDUCED_SIZE -1)){
		throw Exception(DATA_ERROR, "Invalid index for plume file.");
	}
	return bitmap[idx];
}

//========================================================================
string Plume::get_zonename(const int code){
  if (code <= 0 || code > NUMBER_LINES_COLOR) {
    ostringstream os;
    os << "Invalid zone code: " << code;
    throw Exception(PARAMETER_ERROR, os.str());
  }
  return zone[code - 1];
}

//========================================================================
void Plume::set_zonename(const int code, const string name){
  if (code <= 0 || code > NUMBER_LINES_COLOR) {
    ostringstream os;
    os << "Invalid zone code: " << code;
    throw Exception(PARAMETER_ERROR, os.str());
  }
  zone[code - 1] = name;
}
