/*
 *  progressbarwin.cpp
 *  
 *
 *  Created by Danielle Mersch.
 *  Copyright UNIL. All rights reserved.
 *
 */


#include "progressbarwin.h"
#include "datcorrthread.h"

// constructor
ProgressbarWin::ProgressbarWin( wxWindow* parent, wxString dat, wxString tags, wxString outf, wxString logf): ProgressbarWindow( parent ), timer(this, TIMER_ID){
	datfile = dat;
	tagsfile = tags;
	output = outf;
	logfile = logf;
	datcorr_thread = NULL;
	finished = false;
}

//destructor
ProgressbarWin::~ProgressbarWin(){
	timer.Stop();
  // Destroys the thread object (if it exists)
  if (datcorr_thread) {
    datcorr_thread->Wait();   // mandatory even if we know the thread is ended
    delete datcorr_thread;
  }
}

//========================================================================
void ProgressbarWin::initdlg( wxInitDialogEvent& event ){
	datcorr_thread = new DatcorrThread(this, datfile, tagsfile, output, logfile, update_callback);
	datcorr_thread->Create();
	timer.Start(100);
	datcorr_thread->Run();
}

//========================================================================
void ProgressbarWin::set_gauge(int i){
  gauge->SetValue(i);
  percentage->SetLabel(wxString::Format(wxT("%d %%"), i));
}

//========================================================================
void ProgressbarWin::closer(wxCloseEvent& event){
  if (event.CanVeto()) {
    ::wxMessageBox(wxT("Don't even try this ;)"));
    event.Veto();
  }
}

//========================================================================
void ProgressbarWin::thread_ended(){
	finished = true;
}

//========================================================================
void ProgressbarWin::update_callback(int percent, void* instance){
  ProgressbarWin* inst = reinterpret_cast<ProgressbarWin*>(instance);
  //inst->set_gauge(percent);
	inst->gauge_position = percent;
}

//========================================================================
void ProgressbarWin::OnTimer(wxTimerEvent& event){
	set_gauge(gauge_position);
	if (finished){
		EndModal(0);
	}
}

BEGIN_EVENT_TABLE(ProgressbarWin, ProgressbarWindow)
EVT_TIMER(TIMER_ID, ProgressbarWin::OnTimer)
END_EVENT_TABLE()