/*
 *  statistics.cpp
 *  
 *
 *  Created by Danielle Mersch on 10/28/10.
 *  Copyright 2010 __UNIL__. All rights reserved.
 *
 */

#include "statistics.h"


// constructor
Stats::Stats(){
	sum = 0;
	sum_square = 0;
	ctr = 0;
	produit = 0;
	produit_flag = true;
	rapport = 0;
	rapport_flag = true;
	actr = 0;
	sum_sin = 0;
	sum_cos = 0;
}

Stats::~Stats(){ }

//=======================================================================
void Stats::reset(){
	sum = 0;
	sum_square = 0;
	ctr = 0;
	value_list.clear();
	produit = 0;
	produit_flag = true;
	rapport = 0;
	rapport_flag = true;
	actr = 0;
	sum_sin = 0;
	sum_cos = 0;
	angle_list.clear();
}
 
//=======================================================================
void Stats::add_element(double value){
	ctr++;
	sum += value;
	sum_square += value * value;
	if (produit_flag){
		produit *= value;
		if (isinf(produit) || produit < MIN_DOUBLE){
			produit_flag = false;
			//cerr<<"Warning: geometric mean cannot be calculated."<<endl;
		}
	}
	if (rapport_flag){
		double ratio= 1/value;
		if (isinf(ratio) || ratio < MIN_DOUBLE){
			rapport_flag = false;
			//cerr<<"Warning: hamonic mean cannot be calculated."<<endl;
		}else{
			rapport += ratio;
		}
	}
	value_list.push_back(value);
}

//=======================================================================
void Stats::add_angle(double angle){  // angle in degree
	angle_list.push_back(angle);
	// convert to radians
	angle = angle * M_PI / 180;
	double sina = sin(angle);
	double cosa = cos(angle);
	sum_sin += sina;
	sum_cos += cosa;
	actr++;
}

//=======================================================================
double Stats::calculate_mean(){
		double mean (0);
		if (ctr > 0){
			mean = sum /(double) ctr;
		}
		return mean;
}

//=======================================================================
double Stats::calculate_geometric_mean(){
		double mean (0);
		if (produit_flag && ctr > 0){
			mean = exp(log(produit) * 1/(double)ctr);
		}
		if (isinf(mean) || mean < MIN_DOUBLE){
			produit_flag = false;
			cerr<<"Warning: geometric mean cannot be calculated."<<endl;
			mean = 0;
		}
		return mean;
}

//=======================================================================
double Stats::calculate_harmonic_mean(){
	double mean (0);
	if (rapport_flag && ctr > 0){
		mean = ctr/ rapport;
		if (isinf(mean) || mean < MIN_DOUBLE){
			rapport_flag = false;
			cerr<<"Warning: harmonic mean cannot be calculated."<<endl;
			mean = 0;
		}
	}
	return mean;
}

//=======================================================================
double Stats::calculate_stdev(){
	double stdev;
	double mean = calculate_mean();
	if (ctr > 0 && mean != 0){
		stdev = sqrt(1/(double)ctr * (sum_square - ctr * mean * mean));
	}else{
		stdev = 0;
	}
	return stdev;
}

//=======================================================================
double Stats::find_median(){
	if (value_list.empty()){
		return 0;
	}else{
		angle_list.sort();
		double median;
		list<double>::iterator iter = value_list.begin();
		if (ctr % 2 == 0){
		  for (int i(0); i < ctr/2 - 1; i++){
				iter++;
			}
			median = *iter;
			iter++;
			median += *iter;
			median /= (double)2;
		}else{
			for (int i(0); i < ctr/2; i++){
				iter++;
			}
			median = *iter;
		}
	 return median;
	}
}

//=======================================================================
double Stats:: calculate_mean_angle(){
	if (actr > 0){
		// convert back to degrees
		return 180/M_PI * atan2(sum_sin, sum_cos); 
	}else{
		return 0;
	}
}

//=======================================================================
double Stats::find_median_angle(){
	if (angle_list.empty()){
		return 0;
	}else{
		angle_list.sort();
		double median;
		list<double>::iterator iter = value_list.begin();
		if (actr % 2 == 0){
			for (int i(0); i < actr/2 - 1; i++){
				iter++;
			}
			double a1 = *iter;
			iter++;
			double a2 = *iter;
			double sina = sin (a1 * M_PI / 180) + sin(a2* M_PI / 180);
			double cosa = cos(a1 * M_PI / 180) + cos(a2 * M_PI / 180);
			median = 180/M_PI * atan2(sina, cosa);
		}else{
			for (int i(0); i < actr/2; i++){
				iter++;
			}
			median = *iter;
		}
	 return median;  // in degrees
	}
}

