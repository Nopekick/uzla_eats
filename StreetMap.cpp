#include "provided.h"
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include "ExpandableHashMap.h"
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
    
private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment> > map;
};

StreetMapImpl::StreetMapImpl(){
    
}

StreetMapImpl::~StreetMapImpl(){
    
}

bool StreetMapImpl::load(string mapFile){
    ifstream infile(mapFile);
    if (!infile){
        return false;
    }
    string line;
    while (getline(infile, line))
    {
        string streetName = line;
        getline(infile, line);
        istringstream s2(line);
        double numGeoCoords;
        s2 >> numGeoCoords;
        while(numGeoCoords > 0){
            getline(infile, line);
            istringstream s3(line);
            string lat1, lng1, lat2, lng2;
            s3 >> lat1 >> lng1 >> lat2 >> lng2;
            GeoCoord starting(lat1, lng1);
            GeoCoord ending(lat2, lng2);

            if(map.find(starting) == nullptr){
                vector<StreetSegment> temp(1, StreetSegment(starting, ending, streetName));
                map.associate(starting, temp);
            } else {
                vector<StreetSegment> temp = *(map.find(starting));
                vector<StreetSegment> addOne(temp.begin(), temp.end());
                addOne.push_back(StreetSegment(starting, ending, streetName));
                map.associate(starting, addOne);
            }

            if(map.find(ending) == nullptr){
                vector<StreetSegment> temp(1, StreetSegment(ending, starting, streetName));
                map.associate(ending, temp);
            } else {
                vector<StreetSegment> temp = *(map.find(ending));
                vector<StreetSegment> addOne(temp.begin(), temp.end());
                addOne.push_back(StreetSegment(ending, starting, streetName));
                map.associate(ending, addOne);
            }

            numGeoCoords--;
        }
    }
    
    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const {
    vector<StreetSegment> temp;
    if(map.find(gc) != nullptr){
        segs= *map.find(gc);
        return true;
    }
    return false;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap(){
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile){
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const {
   return m_impl->getSegmentsThatStartWith(gc, segs);
}



//JUST FOR TESTING STREEMAP.CPP
//int main() {
//  vector<StreetSegment> a;
//    StreetMap smap;
//    smap.load("mapdata.txt");
//  smap.getSegmentsThatStartWith(GeoCoord("34.0356922", "-118.4937358"),a);
//    cout << a.size() << endl;
//  for(int i = 0; i < a.size(); i++){
//    cout << a[i].name << endl;
//  }
//}

