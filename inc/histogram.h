/*
 *  histogram.h
 *  
 *
 *  Created by Danielle Mersch on 7/28/10.
 *  Copyright 2010 __UNIL__. All rights reserved.
 *
 */

#ifndef __histogram__
#define __histogram__

#include <iostream>
#include <vector>


using namespace std;


/// group structure defining histogram groups
struct group{
	double min;  //< lower group boundary
	double max;  //< upper group boundary
	unsigned long long ctr;  //< number of elements in the group
};

class Histogram{

public:
	/// constructor
	Histogram();
	
	/// destructor
	~Histogram();
	
	/** \brief Adds a group to the histogram
	 * \param min Lower boundary of the group
	 * \param max Upper boundary of the group
	 * \return true if the group could be added, false otherwise
	 */
	bool add_group(const double min, const double max);
	
	/** \brief Add an element to the histogram
	 * \param d Element to add
	 * \return True if element could be classified, false otherwise
	 */
	bool add_data(const double d);
	
	/** \brief Gets the number of groups
	 * \return Number of groups
	 */
	unsigned int get_group_count();
	
	/** \brief Gets the group i in the table
	 * \param i Index of group in table
	 * \return The group at location i
	 */
	group get_group(const unsigned int i);
	
	/** \brief Retrieves the number of elements that were unclassified
	 * \return NUmber of elements that remained unclassified
	 */
	unsigned long long get_unclassified_count();
	
private:
	vector <group> hist;  //< table of groups
	unsigned long long unclassified;  //< number of unclassified elements
};

#endif //__histogram__
