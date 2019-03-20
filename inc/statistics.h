/*
 *  statistics.h
 *  
 *
 *  Created by Danielle Mersch on 10/28/10.
 *  Copyright 2010 __UNIL__. All rights reserved.
 *
 */
 
#ifndef __statistics__
#define __statistics__
 
#include <cmath>
#include <iostream>
#include <list>
#include <cstdlib>
 
 using namespace std;

 const double MAX_DOUBLE =  1.7e308;  //< maximum value that can be expressed in a double
 const double MIN_DOUBLE = 4.94e-324; //< minimum value that can be expressed in a double
 
 class Stats{
 
 public:
	 Stats();
	 ~Stats();
		
		/**\brief Reset all the values and clears the lists
		*/
		void reset();
	 
		/**\brief Adds a value to the sum, counter, sum of square and to the list
		*\param value The element to be added
		*/
	 void add_element(double value);
	 
	 /**\brief Adds an angle to the sums, counters and to the list
		*\param value The element to be added
		*/
	 void add_angle(double angle);
	 
		/**\brief Calculates the arithmetric mean of the values
		*\return The mean
		*/
	 double calculate_mean();
	 
	 /**\brief Calculates the geometric mean of the values
		*\return The geometric mean
		*/
	 double calculate_geometric_mean();
	 
	 /**\brief Calculates the harmonic mean of the values
		*\return The harmonic mean
		*/
	 double calculate_harmonic_mean();
	 
	 /**\brief Calculates the standard deviation of the values
		*\return The standard deviation
		*/
	 double calculate_stdev();
	 
	 /**\brief Finds the median of the values
		* \return The value of the median
		*/
	 double find_median();			
 
 	/**\brief Calculates the arithmetric mean of the angles
		*\return The mean angle in degrees
		*/
	 double calculate_mean_angle();
	
	 /**\brief Finds the median angle
		* \return The median angle in degrees
		*/
	 double find_median_angle();	
 
 protected:
 
 private:
	 double sum_square;		///< sum of square of elements
	 double sum;				//< sum of elements
	 unsigned int ctr;		//< number of elements
	 list <double> value_list;	//< list of elements
	 
	 double produit;				// product of values
	 bool produit_flag;		//< flag to indicate if the product of values can be used and if mean is valid. Goal: prevent usage of inf or zeros 
	 double rapport;			//< sum of ratios of values
	 bool rapport_flag;    //< flag to indicate if the sum of ratios can be used and if the mean is valid. Goal: prevent usage of zero values.
	 
	 double sum_sin;  //< sum of sine of angles
	 double sum_cos;  //< sum of cosine of angles
	 unsigned int actr;  //< counter of angles
	 list <double> angle_list;  //< list of angles
 };

#endif // --statistics__