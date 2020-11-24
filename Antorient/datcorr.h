/*
 * Created by Danielle Mersch.
 * Copyright UNIL. All rights reserved.
 * datcorr - Reads a dat input file and the correspoonding .tags file containing 
 * the relative angle of the tags on the ants and generates a new dat file in which all 
 * angles are corrected with their relative angle. Tags that are not in the .tags file are
 * deleted (i.e. set to zero) from the .dat file 
 * The program also marks false positives with y = -3. False postives are detected thanks to 2 criteria: 
 * 1. the angular and linear speed of a ant should be different from zero and 2. the angular and linear acceleration
 * should be zero
 */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <sstream>
#include <fstream>
#include <time.h>
#include "trackcvt.h"
#include "exception.h"
#include "tags3.h"
#include "datfile.h"
#include "utils.h"


using namespace std;

const string VERSION = "DatCorrector 1.2.0";


/** structure previous contains the frame, x and y coordinate and the angle of the previous detection of a tag
 */
struct previous{
	unsigned int f; //< frame
	int x;					//< x coordinate of position
	int y;					//< y coordinate of position
 	int a;					//< angle of tag
};


/** structure dernier stores the characteristics of the last mouvement
 */
struct dernier{
	int t;		//< tag id
	int f;		//< frame
	double a; //< angle change
	double d;	//< distance
};

const unsigned int BUFCOUNT = 1000;  //< number of frames read at once
const unsigned int DIST_MAX = 1000;  //< maximal distance an ant can move in a single frame
const unsigned int A_MAX = 90;  //< threshold for angle change
const unsigned int DIST_MAX2 = 300;	//threshold for distance (only if combined with angle)

/// Type definition of a (function taking an int and a void*, and returning void) used for callback
typedef void(update_status)(int, void*);


/** \brief Calculates the distance and angle change between a current position and a previous position
 * \param t Current position and angle of tag
 * \param frame current frame
 * \param p Previous position and angle of tag
 * \param a  Angle change to be calculated
 * \param dist Distance to be calculated
 * \return bool True if the distance exceeds the maximal distance or the distanceand the angle exceeds 
 *				 the respective distance and angle thresholds, false otherwise
 */

bool distance_to_last(const tag_pos& t, const int frame, const previous& p, double& a, double& dist);


/** \brief function that executes datcorr
 * \param datfile
 * \param tagsfile
 * \param output
 * \param callback
 * \param user_pointer
 * \return bool 
 */
bool execute_datcorr(string datfile, string tagsfile, string output, string logfile, update_status* callback, void* user_pointer);
