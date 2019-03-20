/*
 *  datFile is a class to manipulate easiliy the .dat files
 *  the class implements methods to read one or more frames at a time from an existing file, move within the file,
 *  write new files, find frames with specific tags
 *
 *  Created by Danielle Mersch on 8/16/08.
 *  Copyright 2008 __UNIL__. All rights reserved.
 *
 */

#ifndef __datFile__
#define __datFile__

#include <cstdlib>
#include <fstream>
#include <string>
#include <iostream>
#include <ctime>
#include "trackcvt.h"  ///< file containing tag_list table and the description of the framerec structure 
#include "exception.h"

using namespace std;

class DatFile{

	public:
		DatFile();
		~DatFile();
		
		/**\brief Tests whether the .dat file exists 
		 * \param nomfichier Path of the .dat file to test
		 * \return True if the file exists, false otherwise
		 */
		bool exists(string nomfichier);
		
		/**\brief Opens dat file and identifies first and last frame in file
		 * \param nomfichier Name of dat file to open
		 * \param write Boolean parameter to specify if the file is opened in read and write mode (write is set to true) or read only mode (write is set to false)
		 */
		void open(string nomfichier, const bool write);	
	
		/** \brief read a frame into framerec structure and puts the frame number in the current attribute of the class
		 *  \param temp Framerec into which we read the data
		 *  \return True if frame was read
		 */
		bool read_frame(framerec& temp);
		
		/** \brief Reads bufcount frames at once
		 * \param buffer Buffer into which frame are read
		 * \param bufcount Number of frames to read
		 * \return True if more than zero frames were read
		 */
		bool read_frame(framerec* buffer, const int bufcount);
		
		/**\brief Create a datfile with the name nomfichier
		 * \param nomfichier Name of the file to create
		 */
		void create_dat(string nomfichier);
		
		/** \brief Goes to a given frame and displays its content
		 *  \param i Number of the frame
		 */
		void show_frame(const unsigned int i);
	
		/**\brief Displays the content of the given frame
		 * \param temp Framerec to be displayed
		 */
		void show_frame(const framerec& temp);
	
		/**\brief Displays the information of a given tag for a given frame
		 * \param tag Id of tag for which you want to display the information
		 * \param temp Frame for which you want to display the information
		 */
		void show_tag(const int& tag, const unsigned int& frame);
	
		/**\brief Displays the information of a given tag for of the given framerec
		 * \param tag Id of tag for which you want to display the information
		 * \param temp Framerec for which you want to display the information
		 */
		void show_tag(const int& tag, const framerec& temp);
		
		/**\brief Goes to frame fr
		 * \param fr Frame to which the pointer should be set. This allows to read this frame next
		 */
		bool go_to_frame(const unsigned int fr);
		
		/**\brief Goes to streamposition p (Beware there is no validation of the stream position!)
		 * \param p Streamposition to which the pointer should be set. This allows to read this frame next
		 */
		void go_to_streampos(streampos p);
		
		/**\brief Goes to frame with the given time 
		 * \param time Time of the frame which should be read next
		 */
		void go_to_time(double time);
	
		/**\brief Moves x frames forward or backward. x is an integrer
		 * \param x Number of frames to move, a negative x allows to move backwards
		 */
		void move(int x);
		
		/**\brief Returns the total number of frames in the dat file
		 * \return The number of frames in the file
		 */
		unsigned long get_frame_count();
	
		/**\brief Returns the last frame number of the dat file
		 * \return lastframe 
		 */
		unsigned int get_last_frame();
	
		/**\brief Returns first frame number of the dat file
		 * \return firstframe
		 */
		unsigned int get_first_frame();
		
		/**\brief Returns the current frame number
		 * \return The current frame number (an attribut of the class)
		 */
		unsigned int get_current_frame();
		
		/**\brief Returns the count
		 * \return The number of frames read in last read_frame operation (an attribut of the class)
		 */
		unsigned int get_count();
		
		/**\brief Gets the current position of the stream
		 * \return The current position of the stream
		 */
		streampos get_streampos();
	
		/**\brief Gets the time of the first frame
		 * \return The time of the first frame
		 */
		double get_first_time();
		
		/**\brief Gets the time of the last frame
		 * \return The time of the last frame
		 */
		double get_last_time();
		
		/**\brief Gets the time of the current frame
		 * \return The time of the current frame
		 */
		double get_current_time();
	
		/**\brief Tests whether the given frame is valid, i.e. within the limits of the file
		 * \param frame Number of frame to be tested
		 * \return True if the frame number is valid, false otherwise 
		 */
		bool is_valid(const unsigned int frame);
		
		/**\brief Tests whether the given time is valid, i.e. within the limits of the file
		 * \param time Time of frame to be tested
		 * \return True if the time is in the file, false otherwise 
		 */
		bool is_valid_time(const double time);
		
		/**\brief Finds the next frame containing data for the given tag, starts searching at the current frame
		 * \param tag Id of tag for which you search data
		 * \param frame Frame containing the result of the search (-1 if no next detection was found)
		 * \return True is the tag has been found, false otherwise
		 */
		bool find_next_frame_with_tag(const int& tag, int& frame);
	
		/**\brief Finds the previous frame containing data for the given tag, starts searching at the current frame
		 * \param tag Id of tag for which you search data
		 * \param frame Frame containing the result of the search (-1 if no previous detection was found)
		 * \return True is the tag has been found, false otherwise
		 */
		bool find_previous_frame_with_tag(const int& tag, int& frame);	
	
		/**\brief Finds the last frame containing data for the given tag, starts searching from the end of the file
		 * \param tag Id of tag for which you search data
		 * \param frame Frame containing the result of the search (-1 if no detection was found)
		 * \return True is the tag has been found, false otherwise
		 */
		bool find_last_frame_with_tag(const int& tag, int& frame);
	
		/**\brief Finds the first frame containing data for the given tag, starts searching from the start of the file
		 * \param tag Id of tag for which you search data
		 * \param frame Frame containing the result of the search (-1 if no detection was found)
		 * \return True is the tag has been found, false otherwise
		 */
		bool find_first_frame_with_tag(const int& tag, int& frame);
		
		/**\brief Finds the next frame containing the tag with the given index, search starts at the current frame
		 * \param idx Index of the tag in the tag_list table
		 * \param frame Frame that will contain the result (-1 if no frame found) 
		 * \return True if a next frame was found, false otherwise
		 */
		bool find_next_frame_with_index(const int& idx, int& frame);
	
		/**\brief Finds the previous frame containing the tag with the given index, search starts at the current frame
		 * \param idx Index of the tag in the tag_list table
		 * \param frame Frame that will contain the result (-1 if no frame found) 
		 * \return True if a previous frame was found, false otherwise
		 */
		bool find_previous_frame_with_index(const int& idx, int& frame);
	
		/**\brief Finds the last frame containing the tag with the given index, search starts at the end of the file
		 * \param idx Index of the tag in the tag_list table
		 * \param frame Frame that will contain the result (-1 if no frame found) 
		 * \return True if the a first frame was found, false otherwise
		 */
		bool find_last_frame_with_index(const int& idx, int& frame);
		
		/**\brief Finds the first frame containing the tag with the given index, search starts at the beginning of the file
		 * \param idx Index of the tag in the tag_list table
		 * \param frame Frame that will contain the result (-1 if no frame found) 
		 * \return True if the a first frame was found, false otherwise
		 */
		bool find_first_frame_with_index(const int& idx, int& frame);
		
		/**\brief Closes the dat file
		 */
		void close();
	
		/**\brief Test whether the pointer is at the end of the file
		 * \return True if the pointer is at the end, false otherwise
		 */
		bool eof();
	
		/**\brief Clears flags (eof, fail, etc)
		 */
		void clear();
		
		/**\brief Tests whether the an error occured while reading the file 
		 * \return True if an error occured, false otherwise
		 */
		bool bad();
		
		/**\brief Writes a framerec to the file
		 * \param temp Framerec to be written to file
		 * \param a Number of frames to be written at the same time (default a = 1)
		 * \return True id the frame was written with success false otherwise
		 */
		bool write_frame(const framerec* temp, const int a = 1);
	
	protected:
		/**\brief Get the index of a given tag in the tag_list table
		 * \param tag Id of tag fow which you wan the index in tag_list
		 * \return Returns the index or -1 if the tag was not found in tag_list
		 */
		int get_tag_index(const int& tag);
	
	private:
		fstream f;					///< file stream
		streampos pos;				///< current streamposition
		unsigned int firstframe;	///< first frame of dat file
		unsigned int lastframe;		///< lastframe of dat file
		double firsttime;			///< time of first frame of dat file
		double lasttime;			///< time of last frame of dat file
		unsigned int current;		///< current frame of dat file (if several frames were read, current contains the last one)
		double currenttime;			///< time of current frame
		unsigned int count;			///< number of frames read by last read operation
};


#endif // __datFile__