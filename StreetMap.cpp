//
//  StreetMap.cpp
//  Goober Eats
//
//  Created by David Dinklage on 3/6/20.
//  Copyright © 2020 David Dinklage. All rights reserved.
//

#include "provided.h"
#include <string>
#include <vector>
#include <functional>
#include "ExpandableHashMap.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment>> m_Map;
    
};

StreetMapImpl::StreetMapImpl() {}

StreetMapImpl::~StreetMapImpl() {}


bool StreetMapImpl::load(string mapFile)
{
    ifstream mapDataFile(mapFile);
    if ( !mapDataFile ) // unable to open file
        return false;
    
    string line;
    int nSegments = 0;
    string sLattitude, sLongitude;
    string eLattitude, eLongitude;
    string streetName;
    
    int lineCounter = -1;
    while (getline(mapDataFile, line)) // read each line from text file
    {
        istringstream iss(line); // create string stream to gather data from each line

        if (lineCounter == -1) // contains street name
            getline(iss, streetName);
        if ( lineCounter == 0) // contains number of segments
            iss >> nSegments;
        if (lineCounter >= 1 && lineCounter <= nSegments) // lines with geocoord data
        {
            // get data from line
            iss >> sLattitude >> sLongitude >> eLattitude >> eLongitude;
            GeoCoord sCoord(sLattitude,sLongitude);
            GeoCoord eCoord(eLattitude,eLongitude);
            
            // add the forward street segment to the vector
            StreetSegment forward(sCoord,eCoord,streetName);
            vector<StreetSegment> sCord_vec;
            sCord_vec.push_back(forward);
            
            //check if already another segment starting with sCoord
            vector<StreetSegment>* sCoordExistingSegment_vec;
            sCoordExistingSegment_vec = m_Map.find(sCoord);
            if ( sCoordExistingSegment_vec != nullptr)
            {
 
                for (int i = 0 ; i < (*sCoordExistingSegment_vec).size() ; i++)
                    sCord_vec.push_back((*sCoordExistingSegment_vec)[i]);
            }
            
            m_Map.associate(sCoord, sCord_vec);
            
            // add the backward segment to the vector
            StreetSegment backward(eCoord,sCoord,streetName);
            vector<StreetSegment> eCord_vec;
            eCord_vec.push_back(backward);
            
            //check if already an existing segment starting with eCoord
            vector<StreetSegment>* eCoordExistingSegment_vec;
            eCoordExistingSegment_vec = m_Map.find(eCoord);
            if ( eCoordExistingSegment_vec != nullptr)
            {

                for (int i = 0 ; i < (*eCoordExistingSegment_vec).size() ; i++)
                    eCord_vec.push_back((*eCoordExistingSegment_vec)[i]);
            }
            
            m_Map.associate(eCoord, eCord_vec);
        
            if ( lineCounter == nSegments)
                lineCounter = -2;
        }
        lineCounter++;
    }
    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    const vector<StreetSegment>* segsPtr = m_Map.find(gc);
    if ( segsPtr == nullptr )
        return false;
    segs.clear();
    for ( int i = 0 ; i < (*segsPtr).size() ; i++ )
        segs.push_back( (*segsPtr)[i] );
    return true;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}


