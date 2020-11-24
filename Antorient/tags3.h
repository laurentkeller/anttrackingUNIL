/*
 *  tags.h
 *  
 *
 *  Created by Danielle Mersch on 10/30/08.
 *  Copyright 2008 __UNIL__. All rights reserved.
 *
 */

#ifndef __tags__
#define __tags__

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <cstdlib>
#include <map>
#include "trackcvt.h"
#include "exception.h"

using namespace std;

struct tags {
  // info about ant specific tracking data
	bool state;		///< indicates if tag exists
	int tag;			///< ID of the tag
	int count;		///< number of frames in which tag detected
  int last_det;	///< frame of last detection
	
  // variables used to set orientation to front of ant (instead of front of tag) and center tag on ant
  double rot;			///< rotation correction, default zero
  double displacement_distance;		///< distance over which a uncentered tag needs to be moved to be centered on the individual, default zero
  double displacement_angle; ///< displacement angle if the tag is uncentered
  
  // used of interaction detection
  double rayon; ///< antenna reach of the ant
  double trapezoid_length; ///<length of trapezoid
	
  // ant data variables
  char type;		///< worker type: small (S), medium (M), large (L), queen (Q), undefined (N)
	double size;	///< size of worker: length abdomen-head
	double headwidth; ///< headwidth of the ant
	int death;		///< frame in which the ant is declared dead
  
	int age;			///< age of the ant
	vector <string> elements; ///< additional elements/columns separated by commas
	// constructor
	tags(): state(0), tag(-1), count(0), last_det(0), rot(0.0), displacement_distance(0.0), displacement_angle(0.0), rayon(0.0), trapezoid_length(0.0), type('N'), size(0.0), headwidth(0.0),death(0),age(0){};
};

//===================================================================

class TagsFile{
  
public:
	// constructor
	TagsFile();
	//destructor
	~TagsFile();
	
	/**\brief Read file and put data in temp attribute
	 * \param nomfichier Name of file to read
	 */
	void read_file(const char* nomfichier);
	
	/**\brief Write data of temp to file
	 * \param nomfichier Name of file to write
	 */
	void write_file(const char* nomfichier);
  
  /**\brief Sets state of the tag
   * \param i Index of tag in the table
   * \param s State of the tag
   */
	void set_state(int i, bool s);
	
	/**\brief Sets ID of the tag
	 * \param i Index of tag in the table
	 * \param tag ID of the tag
	 */
	void set_tag (int i, int tag);
	
	/**\brief Sets count of the tag
	 * \param i Index of tag in the table
	 * \param c Number of time the tag was detected
	 */
	void set_count(int i, int c);
  
  /**\brief Sets the fram eof last detection
	 * \param i Index of tag in the table
	 * \param d Frame of last detection
	 */
	void set_last_det(int i, int d);
	
	/**\brief Set the rotation of the tag
	 * \param i Index of tag in the table
	 * \param Angle of the tag on the ant (in degrees)
	 */
	void set_rot(int i, double r);
  
  /**\brief Sets the antenna reach of the ant
	 * \param i Index of tag in the table
	 * \param r Antenna reach of ant (in pixels)
	 */
	void set_rayon(int i, double r);
	
	/**\brief Sets the distance over which a tag needs to be moved to be centered
	 * \param i Index of tag in the table
	 * \param d Moving distance in pixels
	 */
	void set_displacement_distance(int i, double d);
  
  /**\brief Sets length of trapezoid
	 * \param i Index of tag in the table
	 * \param l Length of trapezoid (in pixels)
	 */
  void set_trapezoid_length(int i, double l);
  
  /**\brief Sets displacement angle
	 * \param i Index of tag in the table
	 * \param a Displacement angle (in degrees)
	 */
  void set_displacement_angle(int i, double a);
	
	/**\brief Sets the type of the ant
	 * \param i Index of tag in the table
	 * \param ty Type of the ant (small (S), medium (M), large(L), queen (Q), undefined (N))
	 */
	void set_type(int i, char ty);
	
	/**\brief Sets the size of the ant
	 * \param i Index of tag in the table
	 * \param s Size of the ant
	 */
	void set_size(int i, double s);
	
	/**\brief Sets the frame of death of the ant
	 * \param i Index of tag in the table
	 * \param r Frame of death
	 */
	void set_death(int i, int d);
	
	/**\brief Sets the age in days of the ant
	 * \param i Index of tag in the table
	 * \param d Age in days of the ant
	 */
	void set_age(int i, int d);
	
	/**\brief Sets the headwidth of the ant
	 * \param i Index of tag in the table
	 * \param h Headwidth of the ant (in pixel)
	 */
	void set_headwidth(int i, double h);
	
	/**\brief Sets all characteristics of an ant
	 * \param i Index of tag in the table
	 * \param tmp Tags characteristics
	 */
  
	void set_tag(int i, const tags& tmp);
	
	/**\brief Changes the content of an element for a given ant
	 * \param i Index of tag in the table
	 * \param idx Index of element to change 
	 * \param e New element
	 * \return True is element was changed successfully, false otherwise
	 */
	bool set_element(int i, int idx, string e);
	
	/**\brief Changes the content of an element for a given ant
	 * \param i Index of tag in the table
	 * \param colname Name of the column 
	 * \param e New element
	 * \return True is element was changed successfully, false otherwise
	 */
	bool set_element(int i, string colname, string e);
	
	//====================== MISCELLANEOUS METHODS  ===========
	
	/**\brief Adds a new column to the tags file
	 * \param header Title of the column
	 * \return Index of the added column, or -1 if column could not be added (because its name existed already)
	 */
	int add_column(const string header);
	
	/**\brief Deletes column from the tags file (only successful for optional columns)
	 * \param header Title of the column
	 * \return True if sucessful deletion, false if header not found or column mandatory
	 */
	bool delete_column(const string header);
	
	/**\brief Finds the index of the column header
	 * \param header Title of the column
	 * \param all Whether the index is relative to all columns (all = true) or only to the optional columns (all = false)
	 * \return Index of the optional column, or -1 if not found
	 */
	int find_column(const string header, bool all=false);
	
	/**\brief Utility function: verifies if ant is alive at the given frame
	 * \param i Index of tag in table
	 * \param frame The frame to test
	 * \return true if ant is alive, false if not
	 */
	bool is_alive(const int i, const int frame);
	
	//====================== SET METHODS comments & metadata ===========
	
	/**\brief Adds an element (key and value) to the metadata
	 * \param key The key of the metadata element
	 * \param value The value of the metadata element
	 * \return True if the element could be added, false otherwise (i.e. element existed already)
	 */
	bool add_metadata(string key, string value);
	
	/**\brief Modifies the value of the key in the metadata
	 * \param key The key of the metadata element to be modified
	 * \param value The new value of the metadata element
	 * \return True if the element could be changed, false otherwise (i.e. element was not found)
	 */
	bool change_metadata(string key, string value);
	
	/**\brief Add a comment to the .tags file
	 * \param comment Comment to be added
	 */
	void add_comment(string comment);
	
	/**\brief Add a spacer comment to the .tags file
	 */
	void add_spacer();
	
	//====================== GET METHODS ===============================
	
	/**\brief Gets the state of the tag
	 * \param i Index of tag in table
	 * \return True if the ant is in the tags file, false otherwise
	 */
	bool get_state(int i);
	
	/**\brief Get the count of the tag
	 * \param i Index of tag in table
	 * \return Number of time the tag was detected
	 */
	int get_count(int i);
	
	/**\brief Get the ID of the tag
	 * \param i Index of tag in table
	 * \return ID of the tag
	 */
	int get_tag (int i);
	
	
	/**\brief Get the frame of the last detection
	 * \param i Index of tag in table
	 * \return Frame of last detection
	 */
	int get_last_det(int i);
  
  /**\brief Get the rotation
	 * \param i Index of tag in table
	 * \return Angle of the tag on the ant (degrees)
	 */
	double get_rot(int i);
	
	/**\brief Get displacement angle of the tag
	 * \param i Index of tag in table
	 * \return Deplacement angle (degrees)
	 */
	double get_displacement_angle(int i);
  
  /**\brief Get displacement distance of the tag
	 * \param i Index of tag in table
	 * \return Deplacement distance (pixels)
	 */
	double get_displacement_distance(int i);
  
  /**\brief Get the antenna reach of the ant
	 * \param i Index of tag in table
	 * \return Antenna reach (pixels)
	 */
	double get_rayon(int i);
  
  /**\brief Get trapezoid length of the tag
	 * \param i Index of tag in table
	 * \return Trapezoid length (pixels)
	 */
  double get_trapezoid_length(int i);
	
	/**\brief Get the type of the ant
	 * \param i Index of tag in table
	 * \return Tye of the ant (small (S), medium (M), large(L), queen (Q), undefined (N))
	 */
	char get_type(int i);
  
  /**\brief Get the type of the ant as integrer
	 * \param i Index of tag in table
	 * \return Type of the ant as integrer 1 (small), 2 (medium), 3 (large), 4 (queen), 0 (undefined))
	 */
  int get_type_as_int(int i);
	
	/**\brief Get the size of the ant
	 * \param i Index of tag in table
	 * \return Size
	 */
	double get_size(int i);
	
	/**\brief Get the frame of death of the ant
	 * \param i Index of tag in table
	 * \return Frame of death or 0 if the ant did not die
	 */
	int get_death(int i);
	
  /**\brief Get the age in days of the ant
	 * \param i Index of tag in table
	 * \return Age in days
	 */
	int get_age(int i);
	
	/**\brief Get the headwidth of the ant
	 * \param i Index of tag in table
	 * \return Headwisth
	 */
  
	double get_headwidth(int i);
	
	/**\ brief get_element
	 * \param i Index of tag in table
	 * \param idx Index of element
	 * \return Element idx
	 */
	string get_element(int i, int idx);
	
	/**\brief get element count
	 * \return Number of additional columns in file (--> elements vector)
	 */
	int get_nb_elements();
	
	/**\brief get header of a column in the element table
	 * \param Index of column
	 * \return Header of column i
	 */
	string get_element_header(int i);
	
	// gets the content of column colname for the ants with index i
	// for optional columns with tags that have state false, it returns false
	bool get_content(int i, string colname, string& content);
	
	// gets the content of column with index for the ants with index i
	// for optional columns with tags that have state false, it returns false
	bool get_content(int i, int index, string& content);
	
	/**\brief Tests whether the given content exists in the given column
	 * \param colname Title of column
	 * \param content Data value of content
	 * \return True is the given content exist in this column, false otherwise
	 */
	bool content_exists(const string& colname, const string& content);
	
  
  /**\brief Returns tag data of a given tag
    *\param idx Index of tag in temp table
    *\return Pointer on the tags data of the specified tag.
    */
  const tags* get_taginfo(int idx);
  
	//====================== GET METHODS comments & metadata ===========
	
	/**\brief Get the comments of the tags file
	 * \return Comments of the tags file (without the header)
	 */
	string get_comments();
	
	/**\brief Get the header row of the tags data
	 * \return The header of the tags data
	 */
	string get_header();
	
	/**\brief Retrieves the value of a specific key
	 * \param key Keyword of the map
	 * \return Texte associated to the key
	 */
	string find_metadata(string key);
	
	/**\brief Deletes the element key from the metadata
	 * \param key The key of the element to delete
	 * \return True is the element could be erased, false otherwise (element not found)
	 */
	bool delete_metadata(string key);

	
private:
	
	/**\brief Finds the index of the column header among all columns
	 * \param header Title of the column
	 * \return Index of the column, or -1 if not found
	 */
	int column_index(const string colname);
	
	// deletes the column idx, returns true on success
	bool delete_column(const int idx);
	
	fstream f;					///< stream from .tags file
	tags temp[tag_count];		///<table with the data from the .tags file
	map <string, string> metadata;			///< map with metadata of colony
	vector <string> comments;			///< table with comments of the .tags file
	int nb_elements;				///< number of columns in elements table
	vector <string> header_elements;	///< header of columns in elements table
};

#endif //__tags__

