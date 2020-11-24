/*
 *  Created by Danielle Mersch.
 *  Copyright UNIL. All rights reserved.
 *  datcorrwin is a class inherited from wx_datcorr and impletments its virtual methods
 *  the class allows to set the files used in datcorr
 */

#ifndef __datcorrwin__
#define __datcorrwin__

#include <wx/string.h>
#include <wx/msgdlg.h>
#include "wx_datcorr.h"

using namespace std;

class DatcorrWin : public DatcorrWindow {
	
	public:
		
		DatcorrWin( wxWindow* parent);
		~DatcorrWin();
		
		/**\brief Gets the path of the output
		 * \return Path of the output
		 */
		wxString get_output_name();
		
		/**\brief Gets the path of the datfile
		 * \return Path of the datfile
		 */
		wxString get_datfile_name();
		
		/**\brief Gets the oath of the tagsfile 
		 * \return Path of the tagsfile
		 */
		wxString get_tags_name();
		
		/**\brief Gets the path of the logfile 
		 * \return Path of Log file
		 */
		wxString get_log_name();
		
		/**\brief Sets the tagsfile name
		 * \param tags Path of the .tags file
		 */
		void set_tags_name(wxString tags);
		
		/**\brief Gets the tagsfile attribut
		 * \return Path of the .dat file
		 */
		void set_datfile_name(wxString dat);
	
	protected:
		
		/**\brief Closes the dialog window of datcorr
		 * \param event Cancel event triggered by user
		 */
		void close_datcorr( wxCommandEvent& event );
		
		/**\brief Retrieves the names of the files to use for datcorr
		 * \param event Ok event triggered by user
		 */
		void start_datcorr( wxCommandEvent& event );
		
	private:
		wxString output;  ///< path of the datfile output
		wxString datfile;  ///< path of the datfile input
		wxString tagsfile; ///< path of the tagsfile input
		wxString logfile; ///< path of the log file output
};

#endif //__datcorrwin__
