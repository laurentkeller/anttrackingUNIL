/*
 *  mainwine.cpp
 *  
 *
 *  Created by Danielle Mersch and Alessandro Crespi.
 *  Copyright UNIL and EPFL. All rights reserved.
 *
 */

#include "mainwin.h"
#include "utils.h"

#include <wx/wx.h>
#include <wx/filedlg.h>
#include <wx/numdlg.h>
#include <wx/filename.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>

#include "coche.xpm"
#include "croix.xpm"
#include "croix2.xpm"
#include "croix3.xpm"
#include "croix4.xpm"

const string VERSION = "AntOrient 1.0";

const unsigned int IMG_WIDTH = 3048;  ///< width of the image
const unsigned int IMG_HEIGHT = 4560;  ///< height of image
const unsigned int IMG_SIZE = IMG_WIDTH * IMG_HEIGHT;  ///< size of image
const signed int HALF_SMALL_SIZE = 250;  ///< half size of the small image displayed in antorient
const signed int SMALL_SIZE = (2 * HALF_SMALL_SIZE);  ///< size of the small image in antorient
const unsigned int FRAME_NUMBER_LENGTH = 8; //< number of digits to specify the framenumber in the filename

//====================================================================================
// Functions
//====================================================================================


/* \brief Reads an image file in format PPM
 * \param filename Name of the image file
 * \return wxImage Pointer to image that is opened
 */
wxImage* load_pgm(const char* filename){
	char tmp[128];

	/// opens the image file 
	ifstream f;
	f.open(filename, ios::in | ios::binary);
	if (!f.is_open()){
		::wxMessageBox(wxT("Unable to open the specified file."), wxT("Error"), wxICON_ERROR);
		return NULL;
	}
	
	// check whether the image type is a binary pgm file
	string s; //< string in which we read the data
	getline(f, s); 
	rtrim(s);
	if (s.compare("P5") != 0){
		f.close();
		::wxMessageBox(wxT("The file contains data in an unsupported format."), wxT("Error"), wxICON_ERROR);
		return NULL;
	}
	s.clear();
	
	// check whether the image read has the correct size
	getline(f, s);
	int xs(0), ys(0);  //< size of the image read
	istringstream ss;
	ss.str(s);
	ss>> xs;
	ss>>ys;
	if (xs!=IMG_WIDTH || ys!=IMG_HEIGHT) {
		f.close();
		::wxMessageBox(wxT("The image contained in the file doesn't have the expected size."), wxT("Error"), wxICON_ERROR);
		return NULL;
	}
	
	// check whether the maximum value a pixel can have is 255
	getline(f, s);
	if (atoi(s.c_str())!=255) {
		f.close();
		::wxMessageBox(wxT("The image pixel format is not supported."), wxT("Error"), wxICON_ERROR);
		return NULL;
	}
	
	// read image
	unsigned char* buffer = (unsigned char*) malloc(IMG_SIZE);
	f.read((char*) buffer, IMG_SIZE);
	f.close();

	// wxImage require a color image, hence we allocate space for a color copy (color image = 3 * gray image)
	unsigned char* buffer2 = (unsigned char*) malloc(IMG_SIZE * 3);
	unsigned char* bptr = buffer2;
	for (int p(0); p<IMG_SIZE; p++){
		*bptr++ = buffer[p];
		*bptr++ = buffer[p];
		*bptr++ = buffer[p];
	}
	free(buffer);
	wxImage* img = new wxImage(IMG_WIDTH, IMG_HEIGHT, buffer2);
	return img;
}

//====================================================================================
/* \brief Asks the user to enter a number (allows to enter -1)
 * \param parent Window that opens for the user
 * \param message Message shown in the window
 * \param promt 
 * \return bool True if the user clicked ok, false if the user clicked cancel
 */
bool input_number(wxWindow* parent, const wxString& message, const wxString& prompt,
  const wxString& caption, long value, long min, long max, int& out){
	
	// wxwidgets class not documented
	wxNumberEntryDialog dlg(parent, message, prompt, caption, value, min, max);
	if (dlg.ShowModal()==wxID_CANCEL){ 
		return false;
	}
	out = dlg.GetValue();
	return true;
}

//====================================================================================
/*\brief converts index to the corresponding letter
 * \param idx Index in the table
 * \return char Returns the letter corresponding to the index in the table
 */
/*char convert_to_type(int idx){
	switch (idx) {
		case 0:
			return 'S';
		case 1:
			return 'M';
		case 2:
			return 'L';
		case 3:
			return 'Q';
		case 4:
		default:
			return 'N';
	}
}*/

//====================================================================================
/* \brief Converts a letter to the corresponding index in the table
 * \param  type Letter to convert into an index
 * \return int Index of the letter in the table
 */
/*int convert_to_name(char type){
	switch (type) {
		case 'S':
			return 0;
		case 'M':
			return 1;
		case 'L':
			return 2;
		case 'Q':
			return 3;
		case 'N':
		default:
			return 4;
	}
}*/

//====================================================================================
//function angle x_axis  © Nathalie Stroeymeyt
//takes as input coordinates of a vector defined by two points (origin A (x_origin, y_origin) to target B (x_target,y_target))
//returns the oriented angle (in degrees, comprised between -180 and +180) between oriented x axis and that vector
double angle_xaxis(double x_origin, double y_origin, double x_target, double y_target){
	double angle;
	double slope(-(y_target-y_origin)/(x_target-x_origin));//minus necessary because yaxis goes down 
	double sign(abs(x_target-x_origin)/(x_target-x_origin));
	double cosx(sign/(sqrt(1+(slope*slope))));
	double sinx(slope*cosx);
	if (cosx==0){
		angle=acos(cosx);
		if (sinx<0){
			angle = angle - M_PI;
		}//sinx<0
	}//cosx==0
	if (cosx!=0){
		angle=atan(sinx/cosx);
		if (cosx < 0){
			angle = angle + M_PI;
			if (angle > M_PI){
				angle = angle - 2*M_PI;
			}//angle > M_PI
		}//cosx < 0
	}//cosx!=0
	angle = limit_angle(angle * 180 /M_PI);	
	return(angle);
};//function angle_axis

//====================================================================================

//function calculate_calibration_parameters  © Nathalie Stroeymeyt
// takes as input "ant" (coordinates and orientation, in degrees * 100, of the tag of the considered ant on the picture), "tailend" (coordinates of the tip of the gaster of that ant on the picture), "antenna reach" (coordinates of the midpoint between the tip of the antennae of that ant on the picture) and "param" (calibration parameters for that specific ant)
// returns nothing, but modifies by reference the content of "param"

void calculate_calibration_parameters(const tag_pos& ant, const position& tailend, const position&  antenna_reach, calibration_parameters& param)
	{

 	//::wxMessageBox(wxString::Format(wxT("Ant.x %d ant.y. %d tail.x %d tail.y %d antenna.x %d antenna.y %d"), ant.x, ant.y, tailend.x,tailend.y, antenna_reach.x, antenna_reach.y));

	//calculate distance between antennal reach and tail end
	param.trapezoid_length = calculate_distance_double(antenna_reach.x,antenna_reach.y, tailend.x,tailend.y);

	//general case: if ant not parallel to x or y axis
	if((antenna_reach.x!=tailend.x)&&(antenna_reach.y!=tailend.y))
		{
		//calculate necessary linear equations for ant axis and its perpendicular
		double antslope(double((tailend.y-antenna_reach.y))/(tailend.x-antenna_reach.x));
		double antintercept(antenna_reach.y - antslope*antenna_reach.x);

		double shiftslope(-1/antslope);
		double shiftintercept(ant.y - shiftslope*ant.x);


		//calculate the coordinates for intersection between these two lines, that corresponds to the corodinates of the desired tag position
		double new_tag_position_x((antintercept-shiftintercept)/(shiftslope-antslope));
		double new_tag_position_y(shiftslope*new_tag_position_x+shiftintercept);

		//calculate distance between original position of tag center and desired, new position of tag center
		param.displacement_distance = calculate_distance_double(ant.x,ant.y,new_tag_position_x,new_tag_position_y);
		
		//calculate new antenna reach
		param.corrected_antenna_reach=calculate_distance_double(new_tag_position_x,new_tag_position_y,antenna_reach.x,antenna_reach.y);
		

		if (param.displacement_distance!=0)
			{
			//calculate angle between x axis and direction in which we want to displace the tag (in degrees)
			double displacementangle_xaxis(angle_xaxis(ant.x,ant.y,new_tag_position_x,new_tag_position_y));
			//calculate angle between original tag angle and direction in which we want to displace the tag (in degrees)
			param.displacement_angle = limit_angle(displacementangle_xaxis-(ant.a/100));
			}
		if (param.displacement_distance==0)
			{
			param.displacement_angle = 0;
			}
		//calculate angle between x axis and actual ant orientation (in degrees)
		double orientationangle_xaxis(angle_xaxis(new_tag_position_x,new_tag_position_y,antenna_reach.x,antenna_reach.y));
		//calculate angle between original tag angle and actual ant orientation (in degrees)
		param.orientation_angle = limit_angle(orientationangle_xaxis-(ant.a/100));

		}//(antenna_reach.x!=tailend.x)&&(antenna_reach.y!=tailend.y)

	//otherwise, if parallel to y axis
	if(antenna_reach.x==tailend.x)//fixed x
		{
		//calculate the coordinates for intersection between these two lines, that corresponds to the corodinates of the desired tag position
		double new_tag_position_x(antenna_reach.x);
		double new_tag_position_y(ant.y);
		
		//calculate distance between original position of tag center and desired, new position of tag center
		param.displacement_distance = calculate_distance_double(ant.x,ant.y,new_tag_position_x,new_tag_position_y);
		
		//calculate new antenna reach
		param.corrected_antenna_reach=calculate_distance_double(new_tag_position_x,new_tag_position_y,antenna_reach.x,antenna_reach.y);

		//calculate angle between original tag angle and direction in which we want to displace the tag (in degrees)
		if (ant.x <= antenna_reach.x)
			{
			param.displacement_angle = limit_angle(0-(ant.a/100));
			}		
		if (ant.x > antenna_reach.x)
			{
			param.displacement_angle = limit_angle(180-(ant.a/100));
			}

		//calculate angle between original tag angle and actual ant orientation (in degrees)
		if(antenna_reach.y>=tailend.y)
			{
			param.orientation_angle = limit_angle(-90-(ant.a/100));
			}		
		if(antenna_reach.y<tailend.y)
			{
			param.orientation_angle = limit_angle(90-(ant.a/100));
			}

		}//(antenna_reach.x==tailend.x)

	//otherwise, if parakllel to x axis
	if(antenna_reach.y==tailend.y)//fixed x
		{
		//calculate the coordinates for intersection between these two lines, that corresponds to the corodinates of the desired tag position
		double new_tag_position_x(ant.x);
		double new_tag_position_y(antenna_reach.y);
		
		//calculate distance between original position of tag center and desired, new position of tag center
		param.displacement_distance = calculate_distance_double(ant.x,ant.y,new_tag_position_x,new_tag_position_y);

		//calculate new antenna reach
		param.corrected_antenna_reach=calculate_distance_double(new_tag_position_x,new_tag_position_y,antenna_reach.x,antenna_reach.y);

		//calculate angle between original tag angle and direction in which we want to displace the tag (in degrees)
		if (ant.y <= antenna_reach.y)
			{
			param.displacement_angle = limit_angle(-90-(ant.a/100));
			}		
		if (ant.y > antenna_reach.y)
			{
			param.displacement_angle = limit_angle(90-(ant.a/100));
			}

		//calculate angle between original tag angle and actual ant orientation (in degrees)
		if(antenna_reach.x>=tailend.x)
			{
			param.orientation_angle = limit_angle(0-(ant.a/100));
			}		
		if(antenna_reach.x<tailend.x)
			{
			param.orientation_angle = limit_angle(180-(ant.a/100));
			}
	
		}//(antenna_reach.y==tailend.y)
	
	}//function calculate_calibration_parameters



//====================================================================================
MainWin::MainWin(wxWindow* parent): MainWinGui(parent){
	// set default values  for class variable
	framenum = 0;
	memset(&fdata, -1, sizeof(fdata));
	memset(&tag_mod, false, sizeof(tag_mod));
	dat = NULL;
	image = NULL;
	tgs = new TagsFile();
	draw_overlay = false;
	disp_tag_idx = -1;
	sel_tag_idx = -1;
	init_gamma(1.0);

	m_bitmap1->Show(false);

	img_liste = new wxImageList(16, 16, true, 1);
	img_liste->Add(wxIcon(coche_bitmap));

	m_listCtrl1->SetImageList(img_liste, wxIMAGE_LIST_SMALL);

	// initialize the labels of the columns of the list
	wxListItem itemCol;

	m_listCtrl1->InsertColumn(0, wxT(""), wxLIST_FORMAT_CENTER, 0);
	m_listCtrl1->InsertColumn(1, wxT("V"), wxLIST_FORMAT_CENTER, 22);
	m_listCtrl1->InsertColumn(2, wxT("Tag"), wxLIST_FORMAT_RIGHT, 50);
	m_listCtrl1->InsertColumn(3, wxT("Count"), wxLIST_FORMAT_RIGHT, 60);
  
	m_listCtrl1->InsertColumn(4, wxT("Rotation"), wxLIST_FORMAT_RIGHT, 60);
	m_listCtrl1->InsertColumn(5, wxT("Displace distance"), wxLIST_FORMAT_RIGHT, 60);
	m_listCtrl1->InsertColumn(6, wxT("Displace angle"), wxLIST_FORMAT_RIGHT, 60);

	m_listCtrl1->InsertColumn(7, wxT("Ant reach"), wxLIST_FORMAT_RIGHT, 50);
	m_listCtrl1->InsertColumn(8, wxT("Trapezoid"), wxLIST_FORMAT_RIGHT, 50);
 
  m_listCtrl1->InsertColumn(9, wxT("Size"), wxLIST_FORMAT_RIGHT, 50);
	m_listCtrl1->InsertColumn(10, wxT("Head"), wxLIST_FORMAT_RIGHT, 70);
	
	coor1 = wxPoint(-1, -1); //coordinates for ant size
	coor2 = coor1;
	stateR = 0; // ruler state
	size = 0.0;
	
	stateS = false; // antenna reach state
	coor3 = coor1;
	
  stateH = 0; // headwidth state
	coor4 = coor1; // headwidth coordinates
	coor5 = coor1;
  
  stateT = false; // tail reach state
  coor6 = coor1; // tail reach coordinate 
  
	datfile = wxT("");
	tagsfile = wxT("");
	modified = false;
	saved = false;
  
  // initialize calibration parameters
  for (int i(0); i < tag_count; i++){
    param[i].displacement_distance = 0;
    param[i].trapezoid_length = 0;
    param[i].displacement_angle = 0;
    param[i].orientation_angle = 0;
    param[i].corrected_antenna_reach = 0;
  }
  calibration = false;
}

// Destructor
MainWin::~MainWin(){
  m_listCtrl1->SetImageList(NULL, wxIMAGE_LIST_SMALL);
  delete img_liste;
  delete tgs;
  if (image!=NULL){ 
		delete image;
	}
}

//====================================================================================
//methods
//====================================================================================

void MainWin::load_tracking_file(wxCommandEvent& event){
// opens dialog windows and requests user to chose a dat file
wxFileDialog odlg(this, wxT("Open tracking file"), wxT(""), wxT(""), wxT("Data files|*.dat"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
if (odlg.ShowModal()!=wxID_OK){ 
	return;
}

// if a tracking file is open, closes the file
if (dat!=NULL){
  dat->close();
  delete dat;
  dat = NULL;
}

// Opens the dat file, displays an error if it fails
dat = new DatFile();
try{
	datfile = odlg.GetPath();
	dat->open((const char*)datfile.mb_str(),false);
}catch (Exception e){
  wxString error (e.get_error().c_str(), wxConvUTF8);
  ::wxMessageBox(error, wxT("Error"), wxICON_ERROR);
  delete dat;
  dat = NULL;
  return;
}

/// Determines the number of frames in file using the size of the file 
unsigned long cnt = dat->get_frame_count();

// Displays the number of frames in the file on the status bar 
m_statusBar1->SetStatusText(wxString::Format(wxT("%lu frames in file."), cnt));

//Updates the display
load_current_frame();
update_tags();
}

//====================================================================================
void MainWin::load_frame_bitmap(wxCommandEvent& event){
// opens dialog windows and requests user to chose pgm file
wxFileDialog odlg(this, wxT("Open frame bitmap"), wxT(""), wxT(""), wxT("PGM files|*.pgm"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
if (odlg.ShowModal()!=wxID_OK){ 
	return;
}

// if an image was loaded, deletes it
if (image!=NULL) {
  delete image;
  image = NULL;
}
  
// Loads image chosen by user
wxString fname = odlg.GetPath();
this->SetCursor(*wxHOURGLASS_CURSOR);
wxImage* img = load_pgm(fname.mb_str());
this->SetCursor(*wxSTANDARD_CURSOR);

// if the image was read correctly, updates the framenumber
if (img!=NULL) {
  wxFileName fn(fname);
	string name = (const char*)fn.GetFullName().mb_str();
	string::size_type n = name.find_last_of('.');
	if (n >= FRAME_NUMBER_LENGTH && n != string::npos){
		string s = name.substr(n-FRAME_NUMBER_LENGTH, FRAME_NUMBER_LENGTH);
		framenum = atoi(s.c_str());
	}else{
		framenum = 0;
	}
	if (framenum == 0){
		framenum = ::wxGetNumberFromUser(wxT("Insert the frame number corresponding to this image."), wxT("Number:"), wxT("Frame number"), 0, 0, 0x7fffffff, this);
	}
  // updates the display
  load_current_frame();
  update_tags();
  image = img;
  update_selected_tag_image();
}
}

//====================================================================================
void MainWin::load_rotation_data( wxCommandEvent& event ){
// opens dialog windows and requests user to chose tags file
wxFileDialog odlg(this, wxT("Open tags file"), wxT(""), wxT(""), wxT("Tag info files|*.tags"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
if (odlg.ShowModal()!=wxID_OK){
	return;
}
// if a tags file was open, closes it
if (tgs!=NULL){
  delete tgs;
  tgs = NULL;
}

// loads tags file
tagsfile = odlg.GetPath();
tgs = new TagsFile();
try{
  tgs->read_file(tagsfile.mb_str());
}catch (Exception e){
  ::wxMessageBox(wxT("Unable to load the tags file."), wxT("Error"), wxICON_ERROR);
  return;
}

// count the number of tags to modify
int ctr(0);
for (int i(0); i<tag_count; i++){
  if (tgs->get_state(i)){ 
		ctr++;
  }
	tag_mod[i] = false;
}

// update display
m_statusBar1->SetStatusText(wxString::Format(wxT("Loaded %d tags."), ctr));
load_current_frame();
update_tags();
}

//====================================================================================
void MainWin::save_rotation_data(wxCommandEvent& event){
// opens dialog windows and asks user where to save the modified tags file
wxFileDialog sdlg(this, wxT("Save rotation file"), wxT(""), wxT(""), wxT("Tag info files|*.tags"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
if (sdlg.ShowModal()!=wxID_OK){
	return;
}
if (tgs==NULL){
  ::wxMessageBox(wxT("Internal error: tags object not found."), wxT("Error"), wxICON_ERROR);
  return;
}

// writes output, sends error message if it fails
try{
	time_t t = time(NULL);
	string date = time_to_str(t);
	string comment = "Tags rotated with " + VERSION + " on " + date;
	tgs->add_comment(comment);
	tgs->add_spacer();
  tgs->write_file(sdlg.GetPath().mb_str());
}catch (Exception e){
  ::wxMessageBox(wxT("Unable to save the tags file."), wxT("Error"), wxICON_ERROR);
  return;
}

// counts the number of tags for which data was saved.
int ctr(0);
for (int i(0); i<tag_count; i++){
  if (tgs->get_state(i)) ctr++;
}

m_statusBar1->SetStatusText(wxString::Format(wxT("Saved %d tags."), ctr));
saved = true;
modified = false;

}

//====================================================================================
void MainWin::exit_application(wxCommandEvent& event){
  this->Close();
}

//====================================================================================
void MainWin::draw_bitmap(wxPaintEvent &event){
  //needed on windows otherwise everything blocks, the function controls the drawing events for m_scrolledWindow1
  wxPaintDC dc1(m_scrolledWindow1);
  
  //wxPaintDC dc(m_bitmap1);
  wxRect re(m_bitmap1->GetRect());
  
  // if there is no image to display, display a empty rectangle
	if (!image || disp_tag_idx == -1){
    wxBrush bgcolor(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    dc1.SetBrush(bgcolor);
    dc1.SetPen(*wxTRANSPARENT_PEN);
    dc1.DrawRectangle(0, 0, re.width, re.height);
    return;
  }
  
  // Displays image (if an image is loaded)
  wxBitmap bmp_obj = m_bitmap1->GetBitmap();
  dc1.DrawBitmap(bmp_obj, 0, 0, false);

  // displays the orientation of the tag (and text : currently not used)
	if (draw_overlay){
    int x_center = tag_position.x;
    int y_center = tag_position.y;
    dc1.SetPen(*wxGREEN_PEN);
    dc1.SetBrush(*wxTRANSPARENT_BRUSH);
    dc1.DrawCircle(x_center, y_center, 30);
    dc1.SetBrush(*wxGREEN_BRUSH);
    dc1.DrawText(overlay_text, x_center + 25, y_center);
    int dx((int)(cos(overlay_orient) * 45.0));
    int dy(-(int)(sin(overlay_orient) * 45.0));
    dc1.DrawLine(x_center-dx, y_center-dy, x_center+dx, y_center+dy);
    dc1.SetPen(*wxCYAN_PEN);
    dc1.SetBrush(*wxBLUE_BRUSH);
    dc1.DrawCircle(x_center+dx, y_center+dy, 4);
    
    dc1.SetPen(*wxRED_PEN);
    if (stateR == 0 && coor1.x!=-1 && coor2.x!=-1){
    	dc1.DrawLine(coor1.x, coor1.y, coor2.x, coor2.y);
    }
    // ant size with ruler
    if (stateR == 2){
      dc1.DrawLine(coor1.x - 2, coor1.y, coor1.x + 2, coor1.y);
      dc1.DrawLine(coor1.x, coor1.y - 2, coor1.x, coor1.y + 2);
    }
	
    wxPen pink = wxPen(wxColour(255,20,147));
    dc1.SetPen(pink);
    if (stateH == 0 && coor4.x!=-1 && coor4.x!=-1){
      dc1.DrawLine(coor4.x, coor4.y, coor5.x, coor5.y);
    }
    // headwidth
    if (stateH == 2){
      dc1.DrawLine(coor4.x - 2, coor4.y, coor4.x + 2, coor4.y);
      dc1.DrawLine(coor4.x, coor4.y - 2, coor4.x, coor4.y + 2);
    }
    
    wxPen orange = wxPen(wxColour(255,128,0));
    dc1.SetPen(orange);
    if (stateS){ // antenna reach
      // draws a cross in the middle of tag
      dc1.DrawCircle(x_center, y_center, 3);
    }else if (!stateS && coor3.x != -1){
      //draws line from tag center to coor3
      dc1.DrawLine(x_center, y_center, coor3.x, coor3.y);
    }
    
    wxPen blue = wxPen(wxColour(0,191,255));
    dc1.SetPen(blue);
    if (stateT){ // tail reach
      // draws a cross in the middle of tag
      dc1.DrawCircle(x_center, y_center, 3);
    }else if (!stateT && coor6.x != -1){
      //draws line from tag center to coor6
      dc1.DrawLine(x_center, y_center, coor6.x, coor6.y);
    }
    
  }
}


//====================================================================================
/*void MainWin::rotate_btn(wxCommandEvent& event){
	// finds the first selected line in the tag list (there should be only one)
  long item = m_listCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
  if (item == -1){ 
		return;
  }
  // Retrieves the position of the tag in the tag list (user data)
  int id = m_listCtrl1->GetItemData(item);
  int angle = tgs->get_rot(id);
  

	// Adjusts the angle of the tag depending on the button clicked
  if (event.GetId() == m_leftbtn->GetId()){
    angle += 90;
  }else if (event.GetId() == m_rightbtn->GetId()){
    angle -= 90;
  }else if (event.GetId() == m_mbtn->GetId()){
    angle--;
  }else if (event.GetId() == m_pbtn->GetId()){
    angle++;
  }else if (event.GetId() == m_setbtn->GetId()){
    if (!input_number(this, wxT("Enter the desired rotation in degrees."), wxT("Rotation:"), wxT("Set rotation"), 0, -180, 360, angle)){ 
			return;
		}
    if (angle > 180){ 
			angle = angle - 360;  // corrects the sign of the angle for angles > 180°
		}
  }

	// if the resulting angle is outside the limit, nothing happens
  if (angle < -180 || angle > 180){
    ::wxBell();
    return;
  }
  
	// updates the data
  tgs->set_rot(id, angle);
  tag_mod[id] = true;
	modified = true;
	saved = false;
  
  // updates the display of the tag list and the image
  m_listCtrl1->SetItem(item, 4, wxString::Format(wxT("%d"), tgs->get_rot(id)));
  m_listCtrl1->SetItemBackgroundColour(item, wxColour(181, 228, 245));
  if (disp_tag_idx == id){
    overlay_orient = (fdata.tags[id].a / 100.0 + tgs->get_rot(id)) * M_PI / 180.0;
    m_scrolledWindow1->Refresh();
	}
}*/

//====================================================================================
void MainWin::set_gamma(wxScrollEvent& event){
	// retrieves the position of the slider and calculates the value (float)
  float gamma = (m_slider1->GetValue()) / 10.0;
	// changes the text next to the slider
  m_staticText2->SetLabel(wxString::Format(wxT("%0.2f"), gamma));
	// reinits the look-up table (LUT)
  init_gamma(gamma);
  // updates image with corrected gamma
  update_tag_image();
}

//====================================================================================
void MainWin::update_selected_tag(wxListEvent& event){

  long item = m_listCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
  if (item == -1){
    sel_tag_idx = -1;
    update_controls();
    return;
  }
  
	// retrieves the position of the tag in the table (user data)
  int tidx = m_listCtrl1->GetItemData(item);
  sel_tag_idx = tidx;
  update_controls();
  if (tidx == disp_tag_idx){
		return;
	}

	// deletes the position of the line (ruler)
  coor1 = wxPoint(-1, -1);
  coor2 = coor1;
  coor3 = coor1;
  coor4 = coor1;
  coor5 = coor1;
  coor6 = coor1;
  calibration = false;
  
  if (fdata.tags[tidx].x!=-1){
    disp_tag_idx = tidx;
    update_controls();
    update_selected_tag_image();
  }
}

//====================================================================================
/*void MainWin::change_type( wxCommandEvent& event ){
	// finds the first selected line in the list (there should not be more than one anyhow)
  long item = m_listCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
  if (item == -1 || disp_tag_idx == -1){
		return;
  }

	// converts the choice index to the type and updates in the list
  char newtype = convert_to_type(m_typechoice->GetSelection());
  tgs->set_type(disp_tag_idx, newtype);
  
	// tag has been modified
  tag_mod[disp_tag_idx] = true;
	modified = true;
	saved = false;
  
	// add a check mark next to the type  (only if type is different from N)
  wxListItem it;
  
  it.SetId(item);
  it.SetColumn(5);
  it.SetText(wxString::Format(wxT("%c"), newtype));
  
  it.SetImage(newtype != 'N' ? 0 : -1);  // i.e. SetImage(0) if !='N' else SetImage(-1)
  m_listCtrl1->SetItem(it);
  
}*/

//====================================================================================
void MainWin::anti_resize( wxListEvent& event ){
  if (event.GetColumn() == 0)
    event.Veto();
}

//====================================================================================
void MainWin::show_ruler( wxMouseEvent& event ){ 
  
	if (stateR != 0){  // ant size
	  wxImage croix(croix_bitmap);
	  croix.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 7);
	  croix.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 7);
		m_scrolledWindow1 -> SetCursor(wxCursor(croix));
	} else if (stateS){ // antenna reach
		wxImage croix2(croix2_bitmap);
		croix2.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 7);
		croix2.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 7);
		m_scrolledWindow1 -> SetCursor(wxCursor(croix2));
	} else if (stateT){ // tail reach
		wxImage croix4(croix4_bitmap);
		croix4.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 7);
		croix4.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 7);
		m_scrolledWindow1 -> SetCursor(wxCursor(croix4));
	}else if (stateH != 0){ // headwidth
		wxImage croix3(croix3_bitmap);
	  croix3.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 7);
	  croix3.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 7);
		m_scrolledWindow1 -> SetCursor(wxCursor(croix3));
	}else{
	  m_scrolledWindow1 -> SetCursor(*wxSTANDARD_CURSOR);
	}
}
  
//====================================================================================
void MainWin::mark( wxMouseEvent& event ){
  
  // ruler selected
  if (stateR == 1){
		coor1 = event.GetPosition();
		stateR = 2;
		m_statusBar1->SetStatusText(wxT("Select second point..."), 0);
		m_scrolledWindow1->Refresh();
    update_controls();
  }else if (stateR == 2){
		coor2 = event.GetPosition();
		
		int dx = abs(coor1.x - coor2.x);
		int dy = abs(coor1.y - coor2.y);
		size = sqrt(dx*dx + dy*dy);
		tgs->set_size(disp_tag_idx, size);
		tag_mod[disp_tag_idx] = true;
		modified = true;
		saved = false;
		long item = m_listCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
		if (item != -1){
			m_listCtrl1->SetItem(item, 9, wxString::Format(wxT("%0.1f"), tgs->get_size(disp_tag_idx)));
		}
		stateR = 0;
		m_scrolledWindow1->Refresh();
		m_scrolledWindow1 -> SetCursor(*wxSTANDARD_CURSOR);
		update_controls();
	}
	
  if (stateH == 1){
		coor4 = event.GetPosition();
		stateH = 2;
		m_statusBar1->SetStatusText(wxT("Select second point..."), 0);
		m_scrolledWindow1->Refresh();
    update_controls();
  }else if (stateH == 2){
		coor5 = event.GetPosition();
		//calculate size & write in table
		int dx = abs(coor4.x - coor5.x);
		int dy = abs(coor4.y - coor5.y);
		size = sqrt(dx*dx + dy*dy);
		tgs->set_headwidth(disp_tag_idx, size);
		tag_mod[disp_tag_idx] = true;
		modified = true;
		saved = false;
		long item = m_listCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
		if (item != -1){
			m_listCtrl1->SetItem(item, 10, wxString::Format(wxT("%0.1f"), tgs->get_headwidth(disp_tag_idx)));
		}
		stateH = 0;
		m_scrolledWindow1->Refresh();
		m_scrolledWindow1 -> SetCursor(*wxSTANDARD_CURSOR);
		update_controls();
	}
	
  // antenna reach
	if(stateS){
		coor3 = event.GetPosition();
    if (coor6.x != -1){ // if tail reach position is known, calculate calibration parameters
      // convert
      tag_pos ant = {tag_position.x, tag_position.y, fdata.tags[disp_tag_idx].a, fdata.tags[disp_tag_idx].id, fdata.tags[disp_tag_idx].padding};
      
      position tail_reach (coor6.x, coor6.y);
      position antenna_reach (coor3.x, coor3.y);
      calculate_calibration_parameters(ant, tail_reach, antenna_reach, param[disp_tag_idx]);
      
      tgs->set_rot(disp_tag_idx, param[disp_tag_idx].orientation_angle);
      tgs->set_displacement_angle(disp_tag_idx, param[disp_tag_idx].displacement_angle);
      tgs->set_displacement_distance(disp_tag_idx, param[disp_tag_idx].displacement_distance);
      tgs->set_rayon(disp_tag_idx, param[disp_tag_idx].corrected_antenna_reach);
      tgs->set_trapezoid_length(disp_tag_idx, param[disp_tag_idx].trapezoid_length);

      tag_mod[disp_tag_idx] = true;
      modified = true;
      calibration = true;
    }
		
		saved = false;
		long item = m_listCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
		if (item != -1 && calibration){ // if tag selected and calibration is known, update display
      m_listCtrl1->SetItem(item, 4, wxString::Format(wxT("%0.1f"), tgs->get_rot(disp_tag_idx)));
			m_listCtrl1->SetItem(item, 5, wxString::Format(wxT("%0.1f"), tgs->get_displacement_distance(disp_tag_idx)));
			m_listCtrl1->SetItem(item, 6, wxString::Format(wxT("%0.1f"), tgs->get_displacement_angle(disp_tag_idx)));
			m_listCtrl1->SetItem(item, 7, wxString::Format(wxT("%0.1f"), tgs->get_rayon(disp_tag_idx)));
      m_listCtrl1->SetItem(item, 8, wxString::Format(wxT("%0.1f"), tgs->get_trapezoid_length(disp_tag_idx)));
		}
		stateS = false;
		m_scrolledWindow1->Refresh();
		m_scrolledWindow1->SetCursor(*wxSTANDARD_CURSOR);
    update_controls();
	}
  
  // tail reach
  if(stateT){
		coor6 = event.GetPosition();
    if (coor3.x != -1){
      position tail_reach (coor6.x, coor6.y);
      position antenna_reach (coor3.x, coor3.y);
      tag_pos ant = {tag_position.x, tag_position.y, fdata.tags[disp_tag_idx].a, fdata.tags[disp_tag_idx].id, fdata.tags[disp_tag_idx].padding};
      calculate_calibration_parameters(ant, tail_reach, antenna_reach, param[disp_tag_idx]);
      
      tgs->set_rot(disp_tag_idx, param[disp_tag_idx].orientation_angle);
      tgs->set_displacement_angle(disp_tag_idx, param[disp_tag_idx].displacement_angle);
      tgs->set_displacement_distance(disp_tag_idx, param[disp_tag_idx].displacement_distance);
      tgs->set_rayon(disp_tag_idx, param[disp_tag_idx].corrected_antenna_reach);
      tgs->set_trapezoid_length(disp_tag_idx, param[disp_tag_idx].trapezoid_length);

      tag_mod[disp_tag_idx] = true;
      modified = true;
      calibration = true;
    }
		saved = false;
		long item = m_listCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
		if (item != -1 && calibration){
      m_listCtrl1->SetItem(item, 4, wxString::Format(wxT("%0.1f"), tgs->get_rot(disp_tag_idx)));
			m_listCtrl1->SetItem(item, 5, wxString::Format(wxT("%0.1f"), tgs->get_displacement_distance(disp_tag_idx)));
			m_listCtrl1->SetItem(item, 6, wxString::Format(wxT("%0.1f"), tgs->get_displacement_angle(disp_tag_idx)));
			m_listCtrl1->SetItem(item, 7, wxString::Format(wxT("%0.1f"), tgs->get_rayon(disp_tag_idx)));
      m_listCtrl1->SetItem(item, 8, wxString::Format(wxT("%0.1f"), tgs->get_trapezoid_length(disp_tag_idx)));
		}
		stateT = false;
		m_scrolledWindow1->Refresh();
		m_scrolledWindow1->SetCursor(*wxSTANDARD_CURSOR);
    update_controls();
	}
}
   



//====================================================================================
void MainWin::update_controls(){
   
  bool activer;  ///< state of tag selection: true if a tag is selected in the table

  // Buttons are actif if the selected tag is the same as the one shown in the image
  activer = (disp_tag_idx == sel_tag_idx && disp_tag_idx != -1);
  //m_statusBar1->SetStatusText(wxString::Format(wxT("Activated: %d, disp %d, sel %d"), activer, disp_tag_idx, sel_tag_idx));
  set_size_b->Enable(activer);
  reset_size_b->Enable(activer);
  set_headwidth_b->Enable(activer);
  reset_headwidth_b->Enable(activer);
  set_tailreach_b->Enable(activer);
  reset_tailreach_b->Enable(activer);
  set_antennareach_b->Enable(activer);
  reset_antennareach_b->Enable(activer);
    
	// if a set or ruler button is clicked then everthing (including tag list) is inactivated except the corresponding ruler/set tool
  if (stateR != 0 || stateS || stateT || stateH != 0){
    activer = false;
    m_listCtrl1->Enable(false);
  }else{
    m_listCtrl1->Enable(true);
  }

	// updates button and displays corresponding message on status bar
  // no button clicked, all buttons activated
  if (stateR == 0 && !stateS && !stateT && stateH == 0){
    if (disp_tag_idx!=-1 && image!=NULL){
			m_statusBar1->SetStatusText(wxString::Format(wxT("Displayed tag: %d"), tag_list[disp_tag_idx]));
		}
  }else if (stateR == 1){ // measuring ant size: first point
		m_statusBar1->SetStatusText(wxT("Select first point..."));
		reset_size_b->Enable(activer);
    set_headwidth_b->Enable(activer);
		reset_headwidth_b->Enable(activer);
		set_tailreach_b->Enable(activer);
    reset_tailreach_b->Enable(activer);
    set_antennareach_b->Enable(activer);
    reset_antennareach_b->Enable(activer);
    
  }else if (stateR == 2){ // measuring ant size: second point 
    set_size_b->Enable(activer);
    set_headwidth_b->Enable(activer);
		reset_headwidth_b->Enable(activer);
		set_tailreach_b->Enable(activer);
    reset_tailreach_b->Enable(activer);
    set_antennareach_b->Enable(activer);
    reset_antennareach_b->Enable(activer);
		m_statusBar1->SetStatusText(wxT("Select second point..."));
    
  }else if (stateS){ // setting antenna reach
		m_statusBar1->SetStatusText(wxT("Set antenna reach..."));
    set_size_b->Enable(activer);
    reset_size_b->Enable(activer);
		set_headwidth_b->Enable(activer);
		reset_headwidth_b->Enable(activer);
		set_tailreach_b->Enable(activer);
    reset_tailreach_b->Enable(activer);
    
  }else if (stateT){ // setting tail reach
		m_statusBar1->SetStatusText(wxT("Set tail reach..."));
    set_size_b->Enable(activer);
    reset_size_b->Enable(activer);
		set_headwidth_b->Enable(activer);
		reset_headwidth_b->Enable(activer);
		set_antennareach_b->Enable(activer);
    reset_antennareach_b->Enable(activer);
    
  }else if (stateH == 1){// measuring headwidth:first point
		m_statusBar1->SetStatusText(wxT("Select first point..."));
		set_size_b->Enable(activer);
    reset_size_b->Enable(activer);
		reset_headwidth_b->Enable(activer);
		set_tailreach_b->Enable(activer);
    reset_tailreach_b->Enable(activer);
    set_antennareach_b->Enable(activer);
    reset_antennareach_b->Enable(activer);
    
  }else if (stateH == 2){ // measuring headwidth: second point
    set_headwidth_b->Enable(activer);
		set_size_b->Enable(activer);
		reset_size_b->Enable(activer);
		set_tailreach_b->Enable(activer);
    reset_tailreach_b->Enable(activer);
    set_antennareach_b->Enable(activer);
    reset_antennareach_b->Enable(activer);
		m_statusBar1->SetStatusText(wxT("Select second point..."));
  }
}


//====================================================================================
void MainWin::set_rayon( wxCommandEvent& event ){
	// // if the tag selected does not correspond to the tag in the image, then nothing happens
	if (sel_tag_idx!=disp_tag_idx || disp_tag_idx==-1) {
		::wxBell();
		return;
	}
	
	// reset the radius point to -1 and the state ot the button to the opposite state
	if (!stateS){
		if(fdata.tags[disp_tag_idx].x!=-1){	
			coor3 = wxPoint(-1,-1);
			stateS = true;
		}
	}else{
		stateS = false;
		coor3 = wxPoint(-1,-1); 
	}
	//m_scrolledWindow1->Refresh();
  update_controls();
}


//====================================================================================
void MainWin::reset_rayon( wxCommandEvent& event ){
	// if the tag selected does not correspond to the tag in the image, then nothing happens
	if (sel_tag_idx != disp_tag_idx || disp_tag_idx == -1){
		::wxBell();
		return;
	}
	
	// reset the radius to zero and update image
	coor3 = wxPoint(-1,-1);
  calibration = false;
  stateS = false;
	m_scrolledWindow1->Refresh();
	
	//update list with tags
	if (fdata.tags[disp_tag_idx].x!=-1){

		tgs->set_rayon(disp_tag_idx, 0.0);
		tgs->set_rot(disp_tag_idx, 0.0);
		tgs->set_displacement_angle(disp_tag_idx, 0.0);
		tgs->set_displacement_distance(disp_tag_idx, 0.0);
		tgs->set_trapezoid_length(disp_tag_idx, 0.0);

		tag_mod[disp_tag_idx] = true;
		modified = true;
		saved = false;
		long item = m_listCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
		if (item != -1){
      m_listCtrl1->SetItem(item, 4, wxString::Format(wxT("%0.1f"), tgs->get_rot(disp_tag_idx)));
			m_listCtrl1->SetItem(item, 5, wxString::Format(wxT("%0.1f"), tgs->get_displacement_distance(disp_tag_idx)));
			m_listCtrl1->SetItem(item, 6, wxString::Format(wxT("%0.1f"), tgs->get_displacement_angle(disp_tag_idx)));
			m_listCtrl1->SetItem(item, 7, wxString::Format(wxT("%0.1f"), tgs->get_rayon(disp_tag_idx)));
      m_listCtrl1->SetItem(item, 8, wxString::Format(wxT("%0.1f"), tgs->get_trapezoid_length(disp_tag_idx)));
		}
	}
  //m_scrolledWindow1->Refresh();
  update_controls();
}



//====================================================================================
void MainWin::set_tail( wxCommandEvent& event ){
	// // if the tag selected does not correspond to the tag in the image, then nothing happens
	if (sel_tag_idx!=disp_tag_idx || disp_tag_idx==-1) {
		::wxBell();
		return;
	}
	
	// reset the radius point to -1 and the state ot the button to the opposite state
	if (!stateT){
		if(fdata.tags[disp_tag_idx].x!=-1){
			coor6 = wxPoint(-1,-1);
			stateT = true;
		}
	}else{
		stateT = false;
		coor6 = wxPoint(-1,-1);
	}
	//m_scrolledWindow1->Refresh();
  update_controls();
}


//====================================================================================
void MainWin::reset_tail( wxCommandEvent& event ){
	// if the tag selected does not correspond to the tag in the image, then nothing happens
	if (sel_tag_idx != disp_tag_idx || disp_tag_idx == -1){
		::wxBell();
		return;
	}
	
	// reset the radius to zero and update image
	coor6 = wxPoint(-1,-1);
	m_scrolledWindow1->Refresh();
	
	//update list with tags
	if (fdata.tags[disp_tag_idx].x!=-1){
		tgs->set_rayon(disp_tag_idx, 0.0);
		tgs->set_rot(disp_tag_idx, 0.0);
		tgs->set_displacement_angle(disp_tag_idx, 0.0);
		tgs->set_displacement_distance(disp_tag_idx, 0.0);
		tgs->set_trapezoid_length(disp_tag_idx, 0.0);
    calibration = false;
		tag_mod[disp_tag_idx] = true;
		modified = true;
		saved = false;
		long item = m_listCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
		if (item != -1){
			m_listCtrl1->SetItem(item, 4, wxString::Format(wxT("%0.1f"), tgs->get_rot(disp_tag_idx)));
			m_listCtrl1->SetItem(item, 5, wxString::Format(wxT("%0.1f"), tgs->get_displacement_distance(disp_tag_idx)));
			m_listCtrl1->SetItem(item, 6, wxString::Format(wxT("%0.1f"), tgs->get_displacement_angle(disp_tag_idx)));
			m_listCtrl1->SetItem(item, 7, wxString::Format(wxT("%0.1f"), tgs->get_rayon(disp_tag_idx)));
      m_listCtrl1->SetItem(item, 8, wxString::Format(wxT("%0.1f"), tgs->get_trapezoid_length(disp_tag_idx)));
		}
	}
  //m_scrolledWindow1->Refresh();
  update_controls();
}

//====================================================================================
void MainWin::ruler( wxCommandEvent& event ){
	
	/// tests whether the selected tag corresponds to the one in the image, if not the case nothing happens
  if (sel_tag_idx!=disp_tag_idx || disp_tag_idx==-1){
    ::wxBell();
    return;
  }
  
  if (stateR==0){
    if (fdata.tags[disp_tag_idx].x!=-1){
      stateR = 1;
    }
  }else if (stateR==1 || stateR==2){
    coor1 = wxPoint(-1, -1);
    stateR = 0;
  }
  update_controls();
}



//====================================================================================
void MainWin::reset_size( wxCommandEvent& event ){
  
	/// tests whether the selected tag corresponds to the one in the image, if not the case nothing happens
  if (sel_tag_idx!=disp_tag_idx || disp_tag_idx==-1){
    ::wxBell();
    return;
  }
	
	// deletes the size line from the image
	coor1 = wxPoint(-1, -1);
	coor2 = coor1;
  m_scrolledWindow1->Refresh();
  
	/// if the selected tag corresponds to the one in the image, reset the size in the tags table
	/// set the modification status to true
  if (fdata.tags[disp_tag_idx].x!=-1){
    tgs->set_size(disp_tag_idx, 0.0);
    tag_mod[disp_tag_idx] = true;
		modified = true;
		saved = false;
    long item = m_listCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
    if (item != -1){
      m_listCtrl1->SetItem(item, 9, wxString::Format(wxT("%0.1f"), tgs->get_size(disp_tag_idx)));
    }
  }
  update_controls();
}

//====================================================================================
void MainWin::set_headwidth( wxCommandEvent& event ){
	/// tests whether the selected tag corresponds to the one in the image, if not the case nothing happens
  if (sel_tag_idx!=disp_tag_idx || disp_tag_idx==-1){
    ::wxBell();
    return;
  }
  
  if (stateH==0){
    if (fdata.tags[disp_tag_idx].x!=-1){
      stateH = 1;
    }
  }else if (stateH==1 || stateH==2){
    coor1 = wxPoint(-1, -1);
    stateH = 0;
  }
  update_controls();
}

//====================================================================================
void MainWin::reset_headwidth( wxCommandEvent& event ){
	// if the tag selected does not correspond to the tag in the image, then nothing happens
	if (sel_tag_idx != disp_tag_idx || disp_tag_idx == -1){
		::wxBell();
		return;
	}
	
	// reset the radius to zero and update image
	coor4 = wxPoint(-1, -1);
	coor5 = coor4;
  m_scrolledWindow1->Refresh();
	
	//update list with tags
	if (fdata.tags[disp_tag_idx].x!=-1){
		tgs->set_headwidth(disp_tag_idx, 0.0);
		tag_mod[disp_tag_idx] = true;
		modified = true;
		saved = false;
		long item = m_listCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
		if (item != -1){
			m_listCtrl1->SetItem(item, 8, wxString::Format(wxT("%0.1f"), tgs->get_headwidth(disp_tag_idx)));
		}
	}
  update_controls();
}


//====================================================================================
void MainWin::select_datfile( wxCommandEvent& event ){
	
	// if tag data have been modified but not saved, ask if user wants to continue
	if (modified){
		wxMessageDialog mdlg (this, wxT("Your last changes of the tag data have not been saved. Do you really want to continue ?"),  wxT("Warning"), wxOK | wxCANCEL);
		if (mdlg.ShowModal() != wxID_OK){ 
			return;
		}
	}

	DatcorrWin* ma_fenetre = new DatcorrWin(this);
	ma_fenetre -> set_datfile_name(datfile);
	if (saved){
		ma_fenetre -> set_tags_name(tagsfile);
	}
	if ((ma_fenetre -> ShowModal()) == 0){
			wxString dat = ma_fenetre -> get_datfile_name();
			wxString outf = ma_fenetre -> get_output_name();
			wxString tags = ma_fenetre -> get_tags_name();
			wxString logf = ma_fenetre -> get_log_name();
	  	ProgressbarWin* ma_progression = new ProgressbarWin(this, dat, tags, outf, logf);
			
			if (ma_progression -> ShowModal() == 0){ }
			delete ma_progression;
	}
	delete ma_fenetre;
	
	
}


//====================================================================================
void MainWin::update_tags(){
	// if the dat file is not loaded nothing happens
	if (dat == NULL){ 
		return;
	}
	m_listCtrl1->DeleteAllItems();
	int ctr(0);
	for (int i(0); i<tag_count; i++){
		if (tgs->get_state(i)){   
			wxString str(wxT(" "));
			long idx = m_listCtrl1->InsertItem(ctr, wxT(""));
			if (fdata.tags[i].x!=-1){
				str = wxT("*");
			}
			m_listCtrl1->SetItem(idx, 1, str);
			m_listCtrl1->SetItem(idx, 2, wxString::Format(wxT("%d"), tag_list[i]));
			m_listCtrl1->SetItem(idx, 3, wxString::Format(wxT("%d"), tgs->get_count(i)));
      
			m_listCtrl1->SetItem(idx, 4, wxString::Format(wxT("%0.1f"), tgs->get_rot(i)));
			m_listCtrl1->SetItem(idx, 5, wxString::Format(wxT("%0.1f"), tgs->get_displacement_distance(i)));
			m_listCtrl1->SetItem(idx, 6, wxString::Format(wxT("%0.1f"), tgs->get_displacement_angle(i)));
      
			m_listCtrl1->SetItem(idx, 7, wxString::Format(wxT("%0.1f"), tgs->get_rayon(i)));
      m_listCtrl1->SetItem(idx, 8, wxString::Format(wxT("%0.1f"), tgs->get_trapezoid_length(i)));
      
      m_listCtrl1->SetItem(idx, 9, wxString::Format(wxT("%0.1f"), tgs->get_size(i)));
			m_listCtrl1->SetItem(idx, 10, wxString::Format(wxT("%0.1f"), tgs->get_headwidth(i)));
			m_listCtrl1->SetItemData(idx, i); 
			if (tag_mod[i]){ 
				m_listCtrl1->SetItemBackgroundColour(idx, wxColour(181, 228, 245));
			}
			ctr++;
		}
	}
}

//====================================================================================
void MainWin::update_tag_image(){
  if (disp_tag_idx == -1){ 
		return;
	}
  if (image==NULL){ 
		return;
	}
  
  update_controls();
  
	// selects the area of the image comprising the tag
  wxRect re(-HALF_SMALL_SIZE, -HALF_SMALL_SIZE, SMALL_SIZE, SMALL_SIZE);
  wxRect img(0, 0, IMG_WIDTH, IMG_HEIGHT);
  re.Offset(fdata.tags[disp_tag_idx].x, fdata.tags[disp_tag_idx].y);
  
  int delta_x(0), delta_y(0);
  
	// checks image border conditions to recenter image (i.e there may be ants very close to the border)
  if (re.x<0){
    delta_x = -re.x;
    re.Offset(delta_x, 0);
  }else if (re.x + re.width>IMG_WIDTH){
    delta_x = IMG_WIDTH - (re.x + re.width);
    re.Offset(delta_x, 0);
  }
  if (re.y<0){
    delta_y = -re.y;
    re.Offset(0, delta_y);
  }else if (re.y + re.height>IMG_HEIGHT){
    delta_y = IMG_HEIGHT - (re.y + re.height);
    re.Offset(0, delta_y);
  }
  
  re.Intersect(img);
  tag_position.x = HALF_SMALL_SIZE - delta_x;
  tag_position.y = HALF_SMALL_SIZE - delta_y;
  
  // Updates the bitmap in the window
  wxImage sub = image->GetSubImage(re);
  apply_gamma(sub);
  wxBitmap* bmp = new wxBitmap(sub);
  m_bitmap1->SetBitmap(*bmp);
  delete bmp;
  draw_overlay = true;
  overlay_orient = (fdata.tags[disp_tag_idx].a / 100.0 + tgs->get_rot(disp_tag_idx)) * M_PI / 180.0;   // angle in RADIANS!!!
  m_scrolledWindow1->Refresh();
}

//====================================================================================
void MainWin::load_current_frame(){
  if (!dat || framenum==0){ 
		return;
	}
	// goes to requested frame and reads data 
  dat->go_to_frame(framenum);
	if (dat->eof()){ 
		return;
	}
	dat->read_frame(fdata);
}

//====================================================================================
void MainWin::init_gamma(double gc){
  gc = 1.0 / gc;
  for (int i(0); i<255; i++){
    double f = (i / 255.0);
    f = pow(f, gc);
    gamma[i] = (unsigned char) floor(f * 255.0);
  }
}

//====================================================================================
void MainWin::apply_gamma(wxImage& img){
  unsigned char* data = img.GetData();
  long len = img.GetWidth() * img.GetHeight() * 3;
  while (len > 0) {
    *data = gamma[*data];
    data++;
    len--;
  }
}
 
//====================================================================================
void MainWin::update_selected_tag_image(){
  // checks whether the position of the selected tag is known
  if (fdata.tags[disp_tag_idx].x==-1){ 
		return;
  }
  // checks whether there is an image
  if (!image){
		return;
  }
  // update the image
  update_tag_image();
}
