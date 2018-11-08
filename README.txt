#######################################################################################################################################

TRACKING DATA PROCESSING SOFTWARE FOR MANUSCRIPT 'Social network plasticity decreases disease transmission in a eusocial insect'

#######################################################################################################################################
Information:

The repository https://github.com/laurentkeller/anttrackingUNIL contains tools for the processing and analysis of automated tracking data 
 
#######################################################################################################################################
Installation instructions (linux):

1. Download the repository content (anttrackingUNIL-master.zip file) 
2. Unzip the anttrackingUNIL-master.zip file
3. Create a folder which will hold all executables files (the full path to that folder is later referred to as 'executable_path')
4. Open a command window
5. Navigate to the anttrackingUNIL-master folder

###Installation of main analysis programs ####
6. Run the following commands:

g++ -o executable_path/change_tagid change_tagid.cpp exception.cpp utils.cpp datfile.cpp tags3.cpp;
g++ -o executable_path/controldat controldat.cpp datfile.cpp tags3.cpp exception.cpp;
g++ -o executable_path/define_death define_death.cpp exception.cpp datfile.cpp tags3.cpp utils.cpp;
g++ -o executable_path/filter_interactions_cut_immobile filter_interactions_cut_immobile.cpp exception.cpp tags3.cpp utils.cpp;
g++ -o executable_path/filter_interactions_no_cut filter_interactions_no_cut.cpp exception.cpp tags3.cpp utils.cpp;
g++ -o executable_path/heatmap3_tofile heatmap3_tofile.cpp datfile.cpp exception.cpp tags3.cpp histogram.cpp statistics.cpp utils.cpp;
g++ -o executable_path/interaction_all_close_contacts interaction_all_close_contacts.cpp exception.cpp tags3.cpp utils.cpp;
g++ -o executable_path/interaction_any_overlap interaction_any_overlap.cpp exception.cpp tags3.cpp utils.cpp;
g++ -o executable_path/interaction_close_front_contacts interaction_close_front_contacts.cpp exception.cpp tags3.cpp utils.cpp;
g++ -o executable_path/time_investment time_investment.cpp exception.cpp utils.cpp plume.cpp datfile.cpp tags3.cpp;
g++ -o executable_path/trackconverter trackconverter_modular.cpp exception.cpp tags3.cpp utils.cpp trackconverter_functions.cpp;
g++ -o executable_path/trajectory trajectory.cpp datfile.cpp exception.cpp tags3.cpp;
g++ -o executable_path/zone_converter zone_converter.cpp exception.cpp utils.cpp plume.cpp datfile.cpp;


###Installation of Antorient ####
7. Unzip the Antorient.zip file
8. In the command window, navigate to the Antorient folder, and run the following commands:
make clean
make
9. Copy the executable file named 'datcorr', which was produced within the Antorient folder during step 7, into the executable_path folder 

###Plume ####
10. Unzip the Plume.zip file (Please note that the executable contained in the Plume folder is a Windows executable only)
