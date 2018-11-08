#ifndef PLUME_H
#define PLUME_H

/*
 *  plume.h
 *  
 *
 *  Created by Danielle Mersch on 24/6/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <string>
#include <iostream>
#include <map>
#include "trackcvt.h"
#include "utils.h"

using namespace std;

const int NUMBER_LINES_COLOR = 6;

typedef map <position,double, position_compare> lut_type;

class Plume{

public:


  static const int YELLOW = 1;		  ///< code of yellow color in bitmap
  static const int ORANGE = 2;		  ///< code of orange color in bitmap
  static const int RED = 3;		  	  ///< code of red color in bitmap
  static const int PURPLE = 4;		  ///< code of purple color in bitmap
  static const int BLUE = 5;			  ///< code of blue color in bitmap
  static const int GREEN = 6;		    ///< code of green color in bitmap
  static const int RUBBER = 255;	 	///< code of rubber color = default color: white -> will not be painted

	Plume();
	~Plume();
	
	//====================================================================================
	/**\fn bool read_plume(const string& plumefile)
	 * \brief Read the plume file and extracts the code of the zones and the validity of the plume file
	 * \param plumefile Path of the file to be opened
	 * \return True if the plume file was read successfully, false otherwise
	 */
	bool read_plume(const string& plumefile);
	
	//====================================================================================
	/**\fn bool write_plume(const string& plumefile)
	 * \brief Write the plume file
	 * \param plumefile Path of the file to be opened
	 * \return True if the plume file was written successfully, false otherwise
	 */
	bool write_plume(const string& plumefile);
	
	//========================================================================
	/**\fn void init_LUT(int code)
	 * \brief Inits the look-up table with distances between the brood and any position in the image.
	 * \param code Code of the zone to which the distance will be calculated
	 * \return True if the code is valid and the LUT has been initialized, false if not
	 */
	bool init_LUT(const int code);

	//========================================================================
	/**\fn void init_LUT(const string zone)
	 * \brief Inits the look-up table with distances between the brood and any position in the image.
	 * \param code Code of the zone to which the distance will be calculated
	 * \return True if the code is valid and the LUT has been initialized, false if not
	 */
	bool init_LUT(const string zonename);
	
	//========================================================================
	/**\fn double calculate_dist(const position& p, int bitmap_index)
	 * \brief Calculates distances between a zone position and a position in the image
	 * \param p Position in the image
	 * \param bitmap_index Position of the zone point
	 * \return Distance between the 2 points
	 */
	static double calculate_dist(const position& p, int bitmap_index);
	
  //========================================================================
  /// Gets first validity frame of the plume image
	int get_firstframe();
	/// Sets first validity frame of the plume image
	void set_firstframe(const int frame);

	//========================================================================
	/// Gets last validity frame of the plume image
	int get_lastframe();
	/// Sets last validity frame of the plume image
	void set_lastframe(const int frame);

  //========================================================================
  /// Gets the pointer to the Plume image
  uint8_t* get_bitmap();
	
  //========================================================================
  /// Gets the size of the bitmap
  int size();	
	
  //========================================================================
  /// Gets a reference of a LUT
  /// \param code The code of the wanted LUT
  const lut_type& get_LUT(const int code);

  /// Gets a reference of a LUT
  /// \param zonename The code of the wanted LUT
  const lut_type& get_LUT(const string zonename);
	
	/// checks whether a given codename exists, if so returns the ID, otherwise returns -1
	int exists(string codename);
  
  //========================================================================
  bool load_LUT(const string filename, const int code);
  bool load_LUT(const string filename, const string zonename);
  
  bool save_LUT(const string filename, const int code);
  bool save_LUT(const string filename, const string zonename);
  
  //========================================================================
  
  /// Clears all LUTs
  void clear_LUT();
  
  /// Clears a LUT
  /// \param code The code of the LUT to clear
  void clear_LUT(const int code);
  
  //========================================================================
  
  /// Get the code of a given coordinate (in high resolution coordinates)
  /// \return The code number of the given point, 255 if none
  int get_code(position p);
	
  /// Get the code of a given index (index between 0 and REDUCED_SIZE)
	/// \return The code number of the given point, 255 if none
	int get_code(const int idx);
  
  /// Gets the name of the specified zone
  /// \param code Code number of zone
  /// \return The zone name of the specified zone code
  string get_zonename(const int code);
  
  /// Sets the name of the specified zone
  /// \param code Code number of zone
  /// \param name Name of the zone
  void set_zonename(const int code, const string name);

private:

  /// converts a zone name to the numeric code
  /// \param zonename The zone name to convert
  /// \param code Reference to an integer that will receive the zone code
  /// \return True if successfully converted the name, false if not
  bool zone_to_code(const string zonename, int& code);
  
	uint8_t* bitmap;  ///< the plume bitmap
	int firstframe;   ///< first frame of validity of the loaded bitmap
	int lastframe;    ///< last frame of validity of the loaded bitmap
	
	/// \warning indexes for zone, code, LUT and LUT_state are 0 ... NUMBER_LINES_COLOR-1, thus
	/// the codes have to be decremented by one (<i>i.e.</i>, zone[YELLOW - 1]).
	
	string zone[NUMBER_LINES_COLOR];  ///< descriptions of the zones
	//unsigned int code[NUMBER_LINES_COLOR];
	
	lut_type LUT[NUMBER_LINES_COLOR];  ///< LUTs
	bool LUT_state[NUMBER_LINES_COLOR]; ///< are the LUTs valid?
};

#endif // PLUME_H
