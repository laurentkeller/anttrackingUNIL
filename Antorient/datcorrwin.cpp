/*
 *  Created by Danielle Mersch.
 *  Copyright UNIL. All rights reserved.
 *
 */

#include "datcorrwin.h"

//constructor
DatcorrWin::DatcorrWin( wxWindow* parent):DatcorrWindow(parent){ 
	output = wxT("");
	datfile = wxT("");
	tagsfile = wxT("");
}
// destructor
DatcorrWin::~DatcorrWin(){ }


void DatcorrWin::close_datcorr( wxCommandEvent& event ){
	this->Close();
}

void DatcorrWin::start_datcorr( wxCommandEvent& event ){
	datfile = dat_n -> GetPath();
	tagsfile = tags_n -> GetPath();
	output = output_n -> GetPath();
	logfile = log_n -> GetPath();
	if (datfile.IsEmpty()){
			::wxMessageBox(wxT("Please provide a .dat file."), wxT("Error"), wxICON_ERROR);
			return;
	}
	if (output.IsEmpty()){
		::wxMessageBox(wxT("Please provide an output file."), wxT("Error"), wxICON_ERROR);
		return;
	}
	if (tagsfile.IsEmpty()){
		::wxMessageBox(wxT("Please provide a .tags file."), wxT("Error"), wxICON_ERROR);
		return;
	}
	
	EndModal(0);
}
		
wxString DatcorrWin::get_output_name(){
	return output;
}

wxString DatcorrWin::get_datfile_name(){
	return datfile;
}

wxString DatcorrWin::get_tags_name(){
	return tagsfile;
}

wxString DatcorrWin::get_log_name(){
	return logfile;
}

void DatcorrWin::set_tags_name(wxString tags){
	tags_n ->SetPath(tags);
}

void DatcorrWin::set_datfile_name(wxString dat){
	dat_n ->SetPath(dat);
}