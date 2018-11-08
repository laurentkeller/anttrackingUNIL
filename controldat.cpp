/*
 *  controldat.cpp
 *  
 *
 *  Created by Danielle Mersch on 11/29/10.
 *  Copyright 2010 __UNIL__. All rights reserved.
 *
 */


#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>

#include "trackcvt.h"
#include "datfile.h"
#include "tags3.h"
#include "exception.h"

using namespace std;

const int TIMEMAX = 2;

struct paire{
	int frame;
	double time;
	int tags;
};

int main(int argc, char* argv[]){
try {
			
	if (argc != 2){
		string info = (string) argv[0] + " input.dat";
		throw Exception (USE, info);
	}

	DatFile dat;
	dat.open((string) argv[1], false);
	paire last;
	last.frame = (dat.get_first_frame()-1);
	last.time = (dat.get_first_time()-0.5);
	int detection[tag_count];
	memset(detection, 0, sizeof(detection));
	double timegap(0);
	
	while(!dat.eof()){
		framerec temp;
		temp.frame = 0;
    
    if (temp.frame%10000 == 0){
			cout<<"frame: "<<temp.frame<<"\r"<<flush;
		}
    
		if (dat.read_frame(temp)){
			int det(0);
			for (int i(0); i< tag_count; i++){
				if (temp.tags[i].x !=-1){
					detection[i]++;
					det++;
				}
			}
			
			if (temp.frame <= last.frame || temp.frame > last.frame +1){
				if (temp.frame != 0){
					cout<<"Frames are unordered: last frame "<<last.frame<<" and current frame: "<<temp.frame<<endl;
					dat.close();
					return 1;
				}
				
			}else if (temp.time <= last.time){
				cout<<"Frames have conflicting time stamps: last frame ("<<last.frame<<"): ";
				cout.precision(12);
				cout<<last.time<<" and current current frame ("<<temp.frame<<"): ";
				cout.precision(12);
				cout<<temp.time<<"; tags detected: "<<last.tags<<" and "<<det<<endl;
				//dat.close();
				//return 1;
			}else if (temp.time > last.time){
				if (last.time - temp.time  > timegap){
					timegap = last.time - temp.time;
				}
        if (timegap > TIMEMAX){
          cout<<"Time gap between last frame ("<<last.frame<<") and current frame "<<timegap<<endl;
        }
				/*cout<<"Time gap between successive frames: last frame ("<<last.frame<<"): ";
				cout.precision(12);
				cout<<last.time<<" and current current frame ("<<temp.frame<<"): ";
				cout.precision(12);
				cout<<temp.time<<"; tags detected: "<<last.tags<<" and "<<det<<endl;
				 */
			}
			
			last.frame = temp.frame;
			last.time = temp.time;
			last.tags = det;
		}
		if (temp.frame > 172750 || temp.frame == 0){
			//cout<<"frame :"<<temp.frame<<endl;
		}
	}
	dat.close();
	cout << "all frames in order"<<endl;
	cout << "biggest time gap is: "<<timegap<<endl;
	
	//for (int i(0); i < tag_count; i++){
	//	if (detection[i]>0){
	//		cout<<"Tag "<<tag_list[i]<<" : "<<detection[i]<<endl;
	//	}
	//}
	
	return 0;
}catch (Exception e) {
}	
	
}
