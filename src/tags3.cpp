/*
 *  tags.cpp
 *  
 *
 *  Created by Danielle Mersch on 10/30/08.
 *  Copyright 2008 __UNIL__. All rights reserved.
 *
 */

#include "tags3.h"
 
const string HEADER = "#tag,count,last_det,rot,displacement_distance,displacement_angle,antenna_reach,trapezoid_length,type,size,headwidth,death,age";
const int HL = HEADER.length();
const int NCOLUMNS = 14; // nb of columns of fixed type



//constructor
TagsFile::TagsFile(){
	//for (int i(0); i< tag_count; i++){	//init temp
	//	vector <string> e; 
	//	tags tmp = {false, tag_list[i], 0, 0, 0, true, 0, 0.0, 0.0, 0.0, 0, 0, e};
	//	temp[i] = tmp;
	//}
	nb_elements = 0;
}

TagsFile::~TagsFile(){ }

//=================== methods =================================

void TagsFile::read_file(const char* nomfichier){		
	ifstream f;
	f.open(nomfichier);
	if (!f.is_open()){
		string info = string(nomfichier);
		throw Exception (CANNOT_OPEN_FILE, info);
	}	
	
	// get metadata (indicated by $METADATA=x x being the number of lines of subsequent metadata)
	int meta (0);
	if (!f.eof()){
		string s1;
		getline(f, s1);
		if (s1[0]=='$'){
			size_t pos = s1.find('=');
			if (pos==string::npos){
				string info = string(nomfichier) + ". Error: metadata format wrong.";
				throw Exception (CANNOT_READ_FILE, info);
			}
			// number of rows of metadata: format it KEY=value
			meta = atoi(s1.substr(pos+1).c_str());
			if (meta < 0){
				string info = string(nomfichier) + ". Error: metadata format wrong.";
				throw Exception (CANNOT_READ_FILE, info);
			}
			// for each row of metadata, find the key and the value
			for(int i(0); i< meta; i++){
				if (!f.eof()){
					string s1;
					getline(f, s1);
					if (s1[0]!='$'){
						string info = string(nomfichier) + ". Error: no metadata details.";
						throw Exception (CANNOT_READ_FILE, info);
					}else{
						size_t pos = s1.find('=');
						if (pos==string::npos){
							string info = string(nomfichier) + ". Error: format of metadata details wrong.";
							throw Exception (CANNOT_READ_FILE, info);
						}
						string key = s1.substr(1, pos-1);
						string value = s1.substr(pos+1);
						if (key.empty() || value.empty()){
							string info = string(nomfichier) + ". Error: incomplete metadata information.";
							throw Exception (CANNOT_READ_FILE, info);
						}
						metadata[key]=value;
					}
				}else{
					string info = string(nomfichier) + ". Error: metadata missing.";
					throw Exception (CANNOT_READ_FILE, info);
				}
			}
		}else{
			string info = string(nomfichier) + ". Error: no metadata information.";
			throw Exception (CANNOT_READ_FILE, info);
		}
	}else{
		string info = string(nomfichier) + ". Error: no data in file.";
		throw Exception (DATA_ERROR, info);
	}
	
	// test if still data in file
	if (f.eof()){
		string info = string(nomfichier) + ". Error: no comments and no tag data.";
		throw Exception (DATA_ERROR, info);
	}
	
	// get comments and tag data
	while (!f.eof()){
		string s1;
		
		// extract comments and header
		do {
			getline(f, s1);
			if (s1[0]=='#'){
				int n = s1.length();
				//cout<<"real header length: "<<HEADER.length()<<endl;
				//cout<<"length header:"<<n<<endl;
				//cout<<"HL:"<<HL<<endl;
				//cout<<"supposed header: "<<s1.substr(0, HL)<<endl;
				// test if header ligne
				if (n >= HL && (s1.substr(0, HL) == HEADER)){
					if (n> HL){
						// get elements after standart header and extract them
						string remainder = s1.substr(HL+1);
						stringstream sr;
						sr.str(remainder);
						while(!sr.fail()){
							string e;
							getline(sr, e, ',');
							if (!e.empty()){
								//cout<<"e:"<<e<<endl;
								header_elements.push_back(e);
								nb_elements++;
							}
						}
					}
				}else{
					string s = s1.substr(1);
					comments.push_back(s);
				}
			}
		} while (s1[0]=='#');
		
		// extract tag data
		vector <string> el;
		tags t;// = {false, -1, 0, 0, 0, true, 0, 0.0, 0.0, 0.0, 0, 0, el};
		if (!f.fail() && !s1.empty()){
			istringstream ligne;
			try{
				ligne.exceptions(istringstream::failbit); // active les exceptions en cas de fail sur le stringstream
				ligne.str(s1);
				ligne >>t.tag;
				ligne.ignore(1, ',');
				ligne >>t.count;
        ligne.ignore(1, ',');
        ligne >>t.last_det;
				ligne.ignore(1, ',');
        
				ligne >>t.rot;
				ligne.ignore(1, ',');
        ligne >>t.displacement_distance;
				ligne.ignore(1, ',');
        ligne >>t.displacement_angle;
        ligne.ignore(1, ',');
        
        ligne >>t.rayon;
        ligne.ignore(1, ',');
        ligne >>t.trapezoid_length;
        ligne.ignore(1, ',');
        
				char tmp;
				ligne >> tmp; 
				switch (tmp) {
					case 'S':
						t.type = 1;
						break;
					case 'M':
						t.type = 2;
						break;
					case 'L':
						t.type = 3;
						break;
					case 'Q':
						t.type = 4;
						break;
					case 'N':
					default:
						t.type = 0;
						break;
				}
				ligne.ignore(1, ',');
				ligne >>t.size;
				ligne.ignore(1, ',');
				ligne >>t.headwidth;
				ligne.ignore(1, ',');
				ligne >>t.death;
				ligne.ignore(1, ',');
				ligne >>t.age;
				ligne.ignore(1,',');
				while(!ligne.eof()){
					string l;
					getline(ligne, l, ',');
					//cout<<"element:"<<l<<endl;
					if (!l.empty()){
						t.elements.push_back(l);
					}
				}
				// check whether there are the correct number of entries
				if (t.elements.size() > nb_elements){
					// a header is missing
					
				}else if (t.elements.size() < nb_elements){
					// content of an element is missing
					
				}
				
			}catch (istringstream::failure e){
				if(!ligne.eof()){
					string info = string(nomfichier) + ". Error while reading input values.";
					throw Exception (CANNOT_READ_FILE, info);
				}
			}
			bool tag_found (false);
			for (int i(0); i < tag_count; i++){
				if (t.tag == tag_list[i]){
					
					temp[i] = t;
					temp[i].state = true;
					tag_found = true;
				}
			}
			if (!tag_found){
				stringstream ss;
				ss<<t.tag;
				string tag_id = ss.str();
				throw Exception(TAG_NOT_FOUND, tag_id);
			}
		}
	}
	f.close();
}


//===================================================================
void TagsFile::write_file(const char* nomfichier){
	ofstream f;
	f.open(nomfichier);
	if (!f.is_open()){
		string info = string(nomfichier);
		throw Exception (CANNOT_READ_FILE, info);
	}	
	
	f<<"$METADATA="<<(int)metadata.size()<<endl;
	map<string,string>::iterator it;
	for ( it=metadata.begin() ; it != metadata.end(); it++ ){
    f<<"$"<< it->first << "=" << it->second << endl;
	}
	f<<"#----------------------------------------------------------------"<<endl;
	for (int i(0); i< comments.size(); i++){
		f<<"#"<<comments[i]<<endl;
	}
	f<<get_header()<<endl;
	for (int j(0); j< tag_count; j++){
		if (temp[j].state){
			f<<tag_list[j]<<","<<temp[j].count<<","<<temp[j].last_det<<","<<temp[j].rot<<","<<temp[j].displacement_distance<<","<<temp[j].displacement_angle<<","<<temp[j].rayon<<","<<temp[j].trapezoid_length<<",";
			switch (temp[j].type){
				case 1:
					f<<"S,";
					break;
				case 2:
					f<<"M,";
					break;
				case 3:
					f<<"L,";
					break;
				case 4:
					f<<"Q,";
					break;
				case 0:
				default:
					f<<"N,";
					break;
			}
			f<<temp[j].size<<","<<temp[j].headwidth<<","<<temp[j].death<<","<<temp[j].age;
			for (int e(0); e< nb_elements; e++){
				f<<","<<temp[j].elements[e];
			}
			f<<endl;
		}
	}
	
	f.close();
}


//============  modifies values of temp =========================
void TagsFile::set_state(int i, bool s){
	temp[i].state = s;
}

//===================================================================
void TagsFile::set_tag(int i, int tag){
	temp[i].tag = tag;
}

//===================================================================
void TagsFile::set_count(int i, int c){
	temp[i].count = c;
}

//===================================================================
void TagsFile::set_last_det(int i, int d){
	temp[i].last_det = d;
}

//===================================================================
void TagsFile::set_rot(int i, double r){
	temp[i].rot = r;
}

//===================================================================
void TagsFile::set_displacement_distance(int i, double l){
	temp[i].displacement_distance = l;
}

//===================================================================
void TagsFile::set_displacement_angle(int i, double a){
	temp[i].displacement_angle = a;
}

//===================================================================
void TagsFile::set_rayon(int i, double r){
	temp[i].rayon = r;
}

//===================================================================
void TagsFile::set_trapezoid_length(int i, double l){
  temp[i].trapezoid_length = l;
}

//===================================================================
void TagsFile::set_size(int i, double s){
	temp[i].size = s;
  
}

//===================================================================
void TagsFile::set_type(int i, char ty){
	switch (ty) {
		case 'S':
		case 's':
			temp[i].type = 1;
			break;
		case 'M':
		case 'm':
			temp[i].type = 2;
			break;
		case 'L':
		case 'l':
			temp[i].type = 3;
			break;
		case 'Q':
		case 'q':
			temp[i].type = 4;
			break;
		case 'N':
		case 'n':
		default:
			temp[i].type = 0;
			break;
	}
}
	
//===================================================================
void TagsFile::set_death(int i, int d){
	temp[i].death = d;
}

//===================================================================
void TagsFile::set_age(int i, int d){
	temp[i].age = d;
}

//===================================================================
void TagsFile::set_headwidth(int i, double h){
	temp[i].headwidth = h;
}

//===================================================================
void TagsFile::set_tag(int i, const tags& tmp){
  temp[i].state = tmp.state;
	temp[i].tag = tmp.tag;
	temp[i].count = tmp.count;
  temp[i].last_det = tmp.last_det;
	temp[i].rot = tmp.rot;
	temp[i].displacement_distance = tmp.displacement_distance;
	temp[i].displacement_angle = tmp.displacement_angle;
  temp[i].rayon = tmp.rayon;
  temp[i].trapezoid_length = tmp.trapezoid_length;
	temp[i].type = tmp.type;
	temp[i].size = tmp.size;
	temp[i].headwidth = tmp.headwidth;
	temp[i].death = tmp.death;
	temp[i].age = tmp.age;
	temp[i].elements = tmp.elements;
}

//===================================================================
bool TagsFile::set_element(int i, int idx, string e){
	if (idx < 0 || idx >= temp[i].elements.size()){
		return false;
	}
	temp[i].elements[idx] = e;
	return true;
}

//===================================================================
bool TagsFile::set_element(int i, string colname, string e){
	// identify index of column
	int idx(-1);
	int j(0);
	do{
		if (header_elements[j] == colname){
			idx = j;
			if(set_element(i, idx, e)){
				return true;
			}else{
				return false;
			}
		}
		j++;
	}while(idx == -1 && j < header_elements.size());
	return false;
}

//===================================================================
int TagsFile::add_column(const string header){
	// need to add checking of header name to avoid having twice the same name
	if (header == "tag" || header == "count" || header == "rot" || header == "last_det" || header == "last" || header == "type"
		|| header == "size" || header == "headwidth" || header == "radius" || header == "death" || header == "age"){
		cerr<<"Cannot add column, header names is identical to one of a mandatory column."<<endl;
		return -1;
	}
	for (int i(0); i< nb_elements; i++){
		if (header == header_elements[i]){
			cerr<<"Cannot add header, column exists already."<<endl;
			return -1;
		}
	}
	header_elements.push_back(header);
	nb_elements++;
	for (int i(0); i < tag_count; i++){
		temp[i].elements.push_back("NA");
	}
	return column_index(header);
}

//===================================================================
bool TagsFile::delete_column(const string header){
	int idx = column_index(header);
	if (idx ==-1){
		cerr<<"Column does not exist."<<endl;
		return false;
	}
	if (idx < NCOLUMNS){
		cerr<<"Cannot delete mandatory column."<<endl;
		return false;
	}
	return (delete_column(idx - NCOLUMNS));
}


//===================================================================
int TagsFile::find_column(const string header, bool all){
	 
	int index = column_index(header);
	if (index == -1){
		cerr<<"Column does not exist."<<endl;
		return index;
	}
	if (!all){
		index -=NCOLUMNS;
	}
	return index;
}




//============  modifies values of comments and metadata ======
bool TagsFile::add_metadata(string key, string value){
	map<string,string>::iterator it;
	it = metadata.find(key);
	if (it != metadata.end() || key=="METADATA"){
		return false;
	} 
	metadata[key]= value;
	return true;
}

//===================================================================
bool TagsFile::change_metadata(string key, string value){
	map<string,string>::iterator it;
	it = metadata.find(key);
	if (it == metadata.end()){
		return false;
	} 
	metadata[key]= value;
	return true;
}

//===================================================================
void TagsFile::add_comment(string comment){
	comments.push_back(comment);
}

//===================================================================
void TagsFile::add_spacer(){
	string comment = "#----------------------------------------------------------------";
	comments.push_back(comment);
}


//============  acceses values of temp =========================
bool TagsFile::get_state (int i){
	return temp[i].state;
}

int TagsFile::get_tag (int i){
	return temp[i].tag;
}

int TagsFile::get_count(int i){
	return temp[i].count;
}

int TagsFile::get_last_det(int i){
	return temp[i].last_det;
}

double TagsFile::get_rot(int i){
	return temp[i].rot;
}

double TagsFile::get_displacement_distance(int i){
	return temp[i].displacement_distance;
}

double TagsFile::get_displacement_angle(int i){
	return temp[i].displacement_angle;
}

double TagsFile::get_rayon(int i){
	return temp[i].rayon;
}

double TagsFile::get_trapezoid_length(int i){
	return temp[i].trapezoid_length;
  
}

char TagsFile::get_type(int i){
	switch (temp[i].type) {
		case 1:
			return 'S';
		case 2: 
			return 'M';
		case 3: 
			return 'L';
		case 4:
			return 'Q';
		case 0:
		default:
			return 'N';
	}
}

int TagsFile::get_type_as_int(int i){
	return temp[i].type;
}

double TagsFile::get_size(int i){
	return temp[i].size;
}

int TagsFile::get_death(int i){
	return temp[i].death;
}

int TagsFile::get_age(int i){
	return temp[i].age;
}

double TagsFile::get_headwidth(int i){
	return temp[i].headwidth;
  
}

string TagsFile::get_element(int i, int idx){
	if (idx < 0 || idx >= temp[i].elements.size() || !temp[i].state ){
		return "";
	}
	return temp[i].elements[idx];
}

int TagsFile::get_nb_elements(){
	return nb_elements;
}


string TagsFile::get_element_header(int i){
	if (i < 0 || i >= header_elements.size()){
		return "";
	}
	return header_elements[i];
}


bool TagsFile::get_content(int i, string colname, string& content){	
	int index = column_index(colname);
	content ="";
	if (index == -1 ){
		return false;
	}else{
		if (!temp[i].state){
			return false;
		}
		return (get_content(i, index, content));
	}
}

bool TagsFile::get_content(int i, int index, string& content){
	content = "";
	if (index < 1 || index > (nb_elements + NCOLUMNS)){
		return false;
	}else if (index <= NCOLUMNS){
		ostringstream os;
		switch (index){
			case 1:		//tag
				os<<temp[i].tag;
				break;
			case 2:		// count
				os<<temp[i].count;
				break;
			case 3:		// last detection
				os<<temp[i].last_det;
				break;
      case 4:		// rotation
				os<<temp[i].rot;
				break;
			case 5:		// last
				os<<temp[i].displacement_distance;
				break;
      case 6:
        os<<temp[i].displacement_angle;
				break;
      case 7:		// antenna reach
				os<<temp[i].rayon;
				break;
      case 8:		// action radius
				os<<temp[i].trapezoid_length;
				break;
			case 9:		// type
				os<<temp[i].type;
				break;
			case 10:		// size
				os<<temp[i].size;
				break;
			case 11:		// headwidth
				os<<temp[i].headwidth;
				break;
			case 12:	// death frame
				os<<temp[i].death;
				break;
			case 13:	// age
				os<<temp[i].age;
				break;
		}
		content = os.str(); 
	}else if (index <= (nb_elements + NCOLUMNS)){
		index -= (NCOLUMNS + 1);
		content = temp[i].elements[index];
	}
	return true;
}


bool TagsFile::content_exists(const string& colname, const string& content){
	int index = column_index(colname);
	if (index == -1){
		cerr<<"Column "<<colname<<" does not exist in the tagsfile."<<endl;
		return false;
	}
	int i(0);
	do {
		if (temp[i].state){
			string tmp = "";
			if (!get_content(i, index, tmp)){
				return false;
			}
			if (tmp == content){
				return true;
			}
		}
		i++;
	}while(i < tag_count);
	return false;
}

bool TagsFile::is_alive(const int i, const int frame){
	if (temp[i].death == 0) {
		return temp[i].state;
	} else {
		return (temp[i].state && frame < temp[i].death);
	}
}

const tags* TagsFile::get_taginfo(int idx){
  return &temp[idx];
}


//============  accesses values of comments and metadata =========
string TagsFile::get_comments(){
	if (comments.empty()){
		return "";
	}else{
		string comment ("");
		for (int i(0); i< comments.size(); i++){
			comment +=comments[i]+'\n';
		}
		return comment;
	}
}

//===================================================================
string TagsFile::get_header(){	
	ostringstream heading;
	heading<<HEADER;
	for (int i(0); i < header_elements.size(); i++){
		heading<<","<<header_elements[i];
	}
	return heading.str();
}

//===================================================================
string TagsFile::find_metadata(string key){
	 map<string,string>::iterator it;
	 it = metadata.find(key);
	 if (it == metadata.end()){
		return "";
	 }
	 return it->second;
}

//===================================================================
bool TagsFile::delete_metadata(string key){
	map<string,string>::iterator it;
	it = metadata.find(key);
	if (it == metadata.end()){
		return false;
	}
	metadata.erase(it);
	return true;
}

//===================================================================
int TagsFile::column_index(const string colname){
	int index(-1);
	//tag,count,rot,last_det,last,type,size,headwidth,radius,death,age
	if (colname == "tag"){
		index = 1;
	}else if (colname == "count"){
		index = 2;
	}else if (colname == "rot"){
		index = 3;
	}else if (colname == "last_det"){
		index = 4;
	}else if (colname == "last"){
		index = 5;
	}else if (colname == "type"){
		index = 6;
	}else if (colname == "size"){
		index = 7;
	}else if (colname == "headwidth"){
		index = 8;
	}else if (colname == "radius"){
		index = 9;
	}else if (colname == "death"){
		index = 10;
	}else if (colname == "age"){
		index = 11;
	}else{
		int j(0);
		while (index == -1 && j < nb_elements){
			if (header_elements[j] == colname){
				index = j + 1 + NCOLUMNS;
			}
			j++;
		}
	}
	return index;
}


//===================================================================
// deletes the column idx, returns true on success
bool TagsFile::delete_column(const int idx){
	if (idx >= nb_elements){
		cerr<<"Column does not exist."<<endl;
		return false;
	}
	for (int i(0); i< tag_count; i++){
		temp[i].elements.erase(temp[i].elements.begin() + idx);
	}
	header_elements.erase(header_elements.begin() + idx);
	nb_elements--;
	return true;
}
