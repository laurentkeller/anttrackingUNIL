/*
 *  utils.cpp
 *  
 *
 *  Created by Danielle Mersch on 11/20/10.
 *  Copyright 2010 __UNIL__. All rights reserved.
 *
 */

#include "utils.h"

using namespace std;


//==========================================================
bool position_compare::operator() (const position& a, const position& b){
	return (a.y < b.y) || (a.y == b.y && a.x < b.x);
}


//==========================================================
// converts unixtime into a string of format DD/MM/YYYY HH:MM:SS
string time_to_str(const time_t h){
	struct tm* tmp;
	tmp = localtime(&h);
	
	char buf[32];
	strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", tmp);
	return string(buf);
}


//==========================================================
// converts a string in format YYYYMMDD hh:mm:ss to unixtime
time_t string_to_time(const string s){
	struct tm timeinfo;
	timeinfo.tm_year = atoi(s.substr(0,4).c_str())-1900;
	timeinfo.tm_mon = atoi(s.substr(4,2).c_str())-1;
	timeinfo.tm_mday = atoi(s.substr(6,2).c_str());
	timeinfo.tm_hour = atoi(s.substr(9,2).c_str());
	timeinfo.tm_min = atoi(s.substr(12,2).c_str());
	timeinfo.tm_sec = atoi(s.substr(15,2).c_str());
	return mktime(&timeinfo);
}

//====================================================================================
//Cuts a line after the first carriage return
void rtrim(string& s){
	string::size_type a = s.find_first_of('\n');
	if (a != string::npos){
		s.erase(a);
	}
}

//========================================================================
// parses the option with multiple parameters separated by a comma
void parse_options(string option, int& box, int& x, int& y){
	stringstream os; 
	os<<option;
	string tmp1, tmp2, tmp3;
	getline(os,tmp1,',');
	box = atoi(tmp1.c_str());
	getline(os,tmp2,',');
	x = atoi(tmp2.c_str());
	getline(os,tmp3);
	y = atoi(tmp3.c_str());
}

//========================================================================
//brief Calculate the difference in degree between two angles
int angledifference(int a1, int a2){
	return (((a1 + 180 - a2) % 360 - 180) * -1);
}


// =====================================================================================
// Keeps an angle between -180deg and +180deg
float limit_angle(float a){
	while (a>180) a -= 360;
	while (a<-180) a += 360;
	return a;
}

// =====================================================================================
double average_direction(vector <double> angles){
	double x (0);
	double y (0);
	for(int i(0); i< angles.size(); i++) {
		x += cos(angles[i] * M_PI / 180);
		y += sin(angles[i] * M_PI / 180);
	}
	return atan2(y, x);
}


//========================================================================
// Tests whether the id is valid (ie. in the list)
bool is_valid_ID(int ID, const int* list, const int n){
	for (int i(0); i < n ; i++){
		if (ID == list[i]){
			return true;
		}
	}
	return false;
}


//========================================================================
// Finds the index of the given tag in the tag list
int get_idx(int name, const int* list, const int n){
	int idx (-1);
	int i(0);
	do {
		if (name == list[i]){
			idx = i;
		}
		i++;
	}while(i < n && idx == -1);
	return idx;
}

//========================================================================
//Test whether point p is in the image of dimensions IMG_W * IMG_H
bool is_in_image(const position& p, const int IMG_W, const int IMG_H){
	if (p.x > IMG_W || p.x < 0){
		return false;
	}
	if (p.y > IMG_H || p.y < 0){
		return false;
	}
	return true;
}

//========================================================================
// Identifies the minimum of two values
int min(int a1, int a2){
	if (a1 < a2){
		return a1;
	} else{
		return a2;
	}
}

//========================================================================
//Identifies the Maximum between values
int max(int a1, int a2){
	if (a1 < a2){
		return a2;
	}else{
		return a1;
	}
}

//========================================================================
// Finds the coordinates of the position half way between 2 positions
position find_middle(const position& d1, const position& d2){
	position p;
	p.x = (d2.x - d1.x)/2 + d1.x;
	p.y = (d2.y - d1.y)/2 + d1.y;
	return p;
}


