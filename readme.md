# Tracking Data Post Processing Software
for manuscript [Social network plasticity decreases disease transmission in a eusocial insect](http://doi.org/10.1126/science.aat4793)

## Information:

The repository https://github.com/laurentkeller/anttrackingUNIL contains tools for the processing and analysis of automated tracking data 
 
## Pipeline installation instructions (linux only)

The project contains a cmake build system. Preferably cmake is used, because it allows to install the header files, so the trk-vid-overlay project (which also contains a cmake build system) can be compiled very easily as well. The minimum cmake version required is 3.10, but it might be possible to use an older version. In that case, the first line in the file anttrackingUNIL/CMakeLists.txt needs to be changed accordingly. cmake can be downloaded here: cmake.org or on ubuntu via "sudo apt install cmake".
Instructions for compilation without cmake are given below.

### With cmake
1. Navigate to the project folder
```shell
cd anttrackingUNIL
```

2. Make and enter build folder
```shell
mkdir build
cd build
```

3. Generate
```shell
cmake ..
```

4. Compile
```shell
make
```

5. Optionally, but recommended if [trk-vid-overlay](https://github.com/laurentkeller/trk-vid-overlay) needs to be compiled later on, install the headers and `atrkutil` library
```shell
sudo make install
```

6. The executables can be found in anttrackingUNIL/build/bin, for usage instructions type for example:
```shell
./change_tagid
```

### Without cmake:

1. Download and unzip or clone the repository content (anttrackingUNIL-master.zip file) 
2. Create a folder which will hold all executables files (the full path to that folder is later referred to as `build`)
3. Open a command window and navigate to the anttrackingUNIL-master folder

#### Installation of main analysis programs
4. Run the following commands:

```shell
cd src
mkdir build
```

```shell
g++ -o build/change_tagid change_tagid.cpp exception.cpp utils.cpp datfile.cpp tags3.cpp -I ../inc;
g++ -o build/controldat controldat.cpp datfile.cpp tags3.cpp exception.cpp -I ../inc;
g++ -o build/define_death define_death.cpp exception.cpp datfile.cpp tags3.cpp utils.cpp -I ../inc
g++ -o build/filter_interactions_cut_immobile filter_interactions_cut_immobile.cpp exception.cpp tags3.cpp utils.cpp -I ../inc;
g++ -o build/filter_interactions_no_cut filter_interactions_no_cut.cpp exception.cpp tags3.cpp utils.cpp -I ../inc;
g++ -o build/heatmap3_tofile heatmap3_tofile.cpp datfile.cpp exception.cpp tags3.cpp histogram.cpp statistics.cpp utils.cpp -I ../inc;
g++ -o build/interaction_all_close_contacts interaction_all_close_contacts.cpp exception.cpp tags3.cpp utils.cpp -I ../inc;
g++ -o build/interaction_any_overlap interaction_any_overlap.cpp exception.cpp tags3.cpp utils.cpp -I ../inc;
g++ -o build/interaction_close_front_contacts interaction_close_front_contacts.cpp exception.cpp tags3.cpp utils.cpp -I ../inc;
g++ -o build/time_investment time_investment.cpp exception.cpp utils.cpp plume.cpp datfile.cpp tags3.cpp -I ../inc;
g++ -o build/trackconverter trackconverter_modular.cpp exception.cpp tags3.cpp utils.cpp trackconverter_functions.cpp -I ../inc;
g++ -o build/trajectory trajectory.cpp datfile.cpp exception.cpp tags3.cpp -I ../inc;
g++ -o build/zone_converter zone_converter.cpp exception.cpp utils.cpp plume.cpp datfile.cpp -I ../inc;
```

5. The executables are then built in the folder anttrackingUNIL/src/build/, for usge instructions type for example:
```shell
./change_tagid
```

## Installation of Antorient
Preliminaries: You need to install wxWidgets 3.0.0 beforehand. On Ubuntu/Debian, you can do as described here:
https://wiki.codelite.org/pmwiki.php/Main/WxWidgets30Binaries#toc2

7. Unzip the Antorient.zip file
8. In the command window, navigate to the Antorient folder, and run the following commands:
```shell
make clean
make
```
9. Copy the executable file named 'datcorr', which was produced within the Antorient folder during step 7, into the executable_path folder 

## Installation of Plume (windows only)
10. Unzip the Plume.zip file
