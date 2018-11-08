/*
 *  histogram.cpp
 *  
 *
 *  Created by Danielle Mersch on 7/28/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "histogram.h"


Histogram::Histogram(){
	unclassified = 0;
}

Histogram::~Histogram(){}

bool Histogram::add_group(const double min, const double max){
	// check if group limits are correct
	for (int i(0); i < hist.size(); i++){
		if ((min >= hist.at(i).min && min < hist.at(i).max) || (max >= hist.at(i).min && max < hist.at(i).max)){
			return false;
		}
		if((hist.at(i).min >= min && hist.at(i).min < max) || (hist.at(i).max > min && hist.at(i).max < max)){
			return false;
		}
	}
	
	group a ;
	a.min = min;
	a.max = max;
	a.ctr = 0;
	hist.push_back(a);
	return true;
}

bool Histogram::add_data(const double d){
	for (int i (0); i < hist.size(); i++){
		if (d >= hist.at(i).min && d < hist.at(i).max){
			hist.at(i).ctr++;
			return true;
		}
	}
	unclassified++;
	return false;
}

unsigned int Histogram::get_group_count(){
	return hist.size();
}

group Histogram::get_group(const unsigned int i){
	return hist.at(i);
}

unsigned long long Histogram::get_unclassified_count(){
	return unclassified;
}

	



