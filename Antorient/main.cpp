/*
 *  Created by Danielle Mersch
 *  Copyright UNIl. All rights reserved.
 *
 */

#include "mainwin.h"

#include <wx/wx.h>
#include <wx/sysopt.h>
#include <iostream>

using namespace std;

class AntOrientApp: public wxApp
{
public:
  virtual bool OnInit();
};

DECLARE_APP(AntOrientApp)

IMPLEMENT_APP(AntOrientApp)

bool AntOrientApp::OnInit()
{
cout << "Je suis AntOrient" << endl;
wxSystemOptions::SetOption(wxT("mac.listctrl.always_use_generic"), true);
MainWin* mw = new MainWin((wxWindow*) NULL);
mw->Show();
SetTopWindow(mw);
return true;
}

