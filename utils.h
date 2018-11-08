/*
 *  @file utils.h
 *  \brief Various fonctions that are useful for many programs: 
			1. unix timestamp <-> string conversions 
			2. string manipulation 
			3. angle calculations
			4. index extraction from list and validity testing
			5. distance calculation between points
 *
 *  Created by Danielle Mersch on 11/20/10.
 *  Copyright 2010 __UNIL__. All rights reserved.
 *
 */

#ifndef __UTILS_H
#define __UTILS_H

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <time.h> 
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>

#include "trackcvt.h"

using namespace std;

const double DOUBLE_MAX = 1.79769e+308;  /// biggest number possible in double

struct position{
	int x;
	int y;
	position(): x(-1), y(-1){};
	position(int a, int b): x(a), y(b) {};
};

//==========================================================
/// Class implementing a method to compare the elements of a map that has a struct type key with 2 integrers that are used for comparisons
class position_compare {
public:
	/**\brief Compares the key values of a map with a position structure as key value
	 * \param a Position of brood with key value 1
	 * \param b Position of brood with key value 2
	 * \return True if the key value 1 is smaller than the key value 2
	 */
	bool operator() (const position& a, const position& b);
};

//========================================================================
/** \fn string time_to_str(const time_t h)
 * \brief Converts unix time to date and seconds
 * \param h Unix time
 * \return Date and time as text
 */
string time_to_str(const time_t h);


/** \fn time_t string_to_time(const string)
 * \brief Converts string with date and time to unix time
 * \param  s String containing date and time in format YYYYMMDD hh:mm:ss
 * \return Unix time
 */
time_t string_to_time(const string s);

/**\fn void parse_options(string option, int& box, int& x, int& y)
 * \brief Parses a string to extract 3 integrers seperated by commands
 * \param
 * \param
 * \param
 */
void parse_options(string option, int& box, int& x, int& y);


/**\fn void rtrim(string& s)
 * \brief Cuts a line after the first carriage return
 * \param s String with information of image
 * \return void
 */
void rtrim(string& s);


/**\fn double angledifference(double a1, double a2)
 * \brief Calculate the difference in degree between two angles
 * \param a1 Angle 1 in degrees (at time t1)
 * \param a2 Angle 2 in degrees (at time t2)
 * \return Angle difference in degrees (positi if trigonometric sens, false otherwise)
 */
int angledifference(int a1, int a2);


/**\fn float limit_angle(float a)
 * \brief Keeps an angle between -180deg and +180deg
 * \param a Angle to correct
 * \return Corrected angle
 */
float limit_angle(float a);


/**\fn double average_direction(vector <double> angles)
 * \brief calculate the average angle (direction) in degrees betweem -180deg and 180deg
 * \param angles Vector containing angles with directions
 * \return Average direction
 */
double average_direction(vector <double> angles);



/**\fn bool is_valid_box(int box)
 * \brief Tests whether the id is valid (ie in the list)
 * \param ID ID to test
 * \param list List with ID's
 * \param n
 * \return True if the ID is in the list, false otherwise;
 */
bool is_valid_ID(int ID, const int* list, const int n);


/**\fn int get_idx(int name)
 * \brief Finds the index of the given name in the given list
 * \param Name for which we search the index
 * \param list List in which to search the index
 * \param n Number of elements in list 
 * \return Index of the tag in list, or -1 if not found
 */
int get_idx(int name, const int* list, const int n);


/**\fn bool is_in_image(const position& p)
 * \brief Test whether point p is in the image of dimensions IMG_W * IMG_H
 * \param p Point to test
 * \param IMG_W Width of image in pixels
 * \param IMG_H Height of image in pixels
 * \return True if the point is in the image, false otherwise
 */
bool is_in_image(const position& p, const int IMG_W, const int IMG_H);


/**\fn inline double calculate_distance(const position& p1, const position& p2)
 * \brief Calculates the distance between 2 points
 * \param p1 Position of the point 1 
 * \param p2 Position of the point 2
 * \return Distance between the 2 points
 */ 
inline double calculate_distance(const position& p1, const position& p2){
	int dx = p1.x - p2.x;
	int dy = p1.y - p2.y;
	return (sqrt(dx * dx + dy *dy));
}

/**\fn inline double calculate_distance(const int x1, const int y1, const int x2, const int y2)
 * \brief Calculates the distance between 2 points
 * \param x1 X-coordinate of the point 1 
 * \param y1 Y-coordinate of the point 1 
 * \param x2 X-coordinate of the point 2 
 * \param y2 Y-coordinate of the point 2 
 * \return Distance between the 2 points
 */
inline double calculate_distance(const int x1, const int y1, const int x2, const int y2){
	int dx = x1 - x2;
	int dy = y1 - y2;
	return (sqrt(dx * dx + dy *dy));
}

/**\fn int max(int a1, int a2)
 * \brief Identifies the Maximum between values
 * \param a1 Value 1
 * \param a2 Value 2
 * \return Maximum
 */
int max(int a1, int a2);

/**\fn int min(int a1, int a2)
 * \brief Identifies the minimum of two values
 * \param a1 Value 1
 * \param a2 Value 2
 * \return Minimum
 */
int min(int a1, int a2);


//========================================================================
/**\fn position find_middle(const position& d1, const position& d2)
 * \brief Finds the coordinates of the position half way between 2 positions
 * \param d1 position 1
 * \param d2 position 2
 * \return position in the middle between the 2 given positions
 */ 
position find_middle(const position& d1, const position& d2);



//========================================================================
//			string manipulation functions
//========================================================================
inline int str2int(string s){
	int tmp(0);
	stringstream ss;
	ss.str(s);
	ss>>tmp;
	return (tmp);
}

template <class T>
inline string to_string (const T& t){
	stringstream ss;
	ss << t;
	return ss.str();
}


#endif // __UTILS_H__
