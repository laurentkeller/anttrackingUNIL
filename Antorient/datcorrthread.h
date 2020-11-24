/*
 *  datcorrthread is a class that allows datcorr to run in a distinc thread 
 *  and hence display its progression on a progression bar  
 *
 *  Created by Danielle Mersch on 10/27/10.
 *  Copyright 2010 __UNIL__. All rights reserved.
 *
 */
 
#ifndef __datcoorthread__
#define __datcorrthread__
 
#include "progressbarwin.h"
#include <wx/thread.h>
#include <iostream>
#include <wx/msgdlg.h>
#include "datcorr.h"
#include <string>
#include <wx/msgdlg.h>
#include <wx/string.h>
 
 using namespace std;
 

/// Thread class: extends the wxThread class and implements the virtual Entry() method
class DatcorrThread : public wxThread{

	public:
		//constructor
		DatcorrThread(ProgressbarWin* somewin, wxString dat, wxString tags, wxString outf, wxString logf, update_status* cbk);
		
		/** \brief Methods that the thread for the datcorr with the progression bar of datcorr
		 *  \return ExitCode
		 */
		virtual ExitCode Entry();

	private:
		ProgressbarWin* win;  ///< windows indicating progress of datcorr
		wxString datfile;  ///< Path of .dat file with input data needed for datcorr
		wxString tagsfile;  ///< Path of .tags file with input data needed for datcorr
		wxString output;		///< Path of .dat output file with processed data needed for datcorr
		wxString logfile;		///< Path of .log file with postprocessing information
		update_status* callback;	///< callback function to update progression bar in antorient
};


#endif // __datcorrthread__
 
