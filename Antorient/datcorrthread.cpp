/*
 *  datcorrthread.cpp
 *  
 *
 *  Created by Danielle Mersch on 10/27/10.
 *  Copyright 2010 __UNIL__. All rights reserved.
 *
 */

#include "datcorrthread.h"


// Constructor, create a joinable thread
DatcorrThread::DatcorrThread(ProgressbarWin* somewin, wxString dat, wxString tags, wxString outf, wxString logf, update_status* cbk): wxThread(wxTHREAD_JOINABLE){
  win = somewin;
	datfile = dat;
	tagsfile = tags;
	output = outf;
	logfile = logf;
	callback = cbk;
}

// The actual code run by the thread
DatcorrThread::ExitCode DatcorrThread::Entry() {
	
	// convert names of files from wxStrings in string type
	string dat = (const char*) datfile.mb_str();
	string tags = (const char*) tagsfile.mb_str();
	string outf = (const char*) output.mb_str();
	string logf = (const char*) logfile.mb_str();
	// start datcorr
	try{
		execute_datcorr(dat, tags, outf, logf, callback, win);
	}catch (Exception e) {
		wxString error (e.get_error().c_str(), wxConvUTF8);
		::wxMessageBox(error, wxT("The correction of the datafile encountered some problems ... "), wxICON_ERROR);
		//::wxMessageBox(wxT("The correction of the datafile encountered some problems ... "));
	}	
	
	// end the thread
  win->thread_ended();
}