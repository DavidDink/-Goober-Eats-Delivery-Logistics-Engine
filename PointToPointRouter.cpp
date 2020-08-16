//
//  PointToPointRouter.cpp
//  Goober Eats
//
//  Created by David Dinklage on 3/6/20.
//  Copyright © 2020 David Dinklage. All rights reserved.
//

#include "provided.h"
#include "ExpandableHashMap.h"
#include <list>
#include <queue>
#include <set>
using namespace std;

struct geoStruct
{
    geoStruct()
    {
        coord = GeoCoord("34.0547000","-118.4794734");
        end = GeoCoord("34.0544590","-118.4801137");
        pathLengthSoFar = 0;
        streetName = "";
        
    }
    geoStruct(GeoCoord c, GeoCoord e, double pathLength, string sn)
    {
        coord = c;
        end = e;
        pathLengthSoFar = pathLength;
        streetName = sn;
    }
    
    GeoCoord coord;
    GeoCoord end;
    string streetName;
    double pathLengthSoFar;
};

bool operator==(const geoStruct& lhs, const geoStruct& rhs)
{
    if ( lhs.coord == rhs.coord)
        return true;
    return false;
}

unsigned int hasher(const geoStruct& gs)
{
    return std::hash<string>()(gs.coord.latitudeText + gs.coord.longitudeText);
}

class cmpFunction
{
public:
    bool operator()(const geoStruct& a, const geoStruct& b)
    {
        if ( a.pathLengthSoFar + distanceEarthMiles(a.coord, a.end) >
             b.pathLengthSoFar + distanceEarthMiles(b.coord, b.end) )
            return true;
        else
            return false;
    }
};
 

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_streetMap;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
    m_streetMap = sm;
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
    
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    
    vector<StreetSegment> dummy;
    if ( m_streetMap->getSegmentsThatStartWith(start, dummy) == false)
        return BAD_COORD;
    if (m_streetMap->getSegmentsThatStartWith(end, dummy) ==  false)
        return BAD_COORD;
    
    route.clear();
    
    if (start == end)
    {
        totalDistanceTravelled = 0;
        return DELIVERY_SUCCESS;
    }
        
    set<GeoCoord> geoCoordVisited;
    priority_queue<geoStruct, vector<geoStruct>, cmpFunction> coordQueue;
    ExpandableHashMap<geoStruct, geoStruct> locationOfPreviousCoord;
    
    coordQueue.push(geoStruct(start, end, 0, ""));
    geoCoordVisited.insert(start);
    
    while (coordQueue.empty() == false)
    {
        // pop the top of the stack
        geoStruct current;
        current.coord = coordQueue.top().coord;
        current.end = end;
        current.pathLengthSoFar = coordQueue.top().pathLengthSoFar;
        current.streetName = coordQueue.top().streetName;
        coordQueue.pop();
        
        // if the current coord is the end, we're done
        if (current.coord == end)
        {
            totalDistanceTravelled = current.pathLengthSoFar;
            
            geoStruct cc = current;
            geoStruct backwards = *(locationOfPreviousCoord.find(cc));
            while ( true )
            {
                route.push_front(StreetSegment(backwards.coord, cc.coord, cc.streetName));
                cc = backwards;
                if (locationOfPreviousCoord.find(backwards) != nullptr)
                    backwards = *(locationOfPreviousCoord.find(backwards));
                else
                    break;
            }
            return DELIVERY_SUCCESS;
        }
            
        // get the street segments connecting to the current coord
        vector<StreetSegment> connectingSegments;
        m_streetMap->getSegmentsThatStartWith(current.coord, connectingSegments);
        for ( int i = 0 ; i < connectingSegments.size() ; i++)
        {
            set<GeoCoord> :: iterator it;
            it = geoCoordVisited.find(connectingSegments[i].end);
            if ( it == geoCoordVisited.end() ) // the connecting coord has not been visited
            {
                GeoCoord nextCoord(connectingSegments[i].end);
                double dist = current.pathLengthSoFar + distanceEarthMiles(current.coord, nextCoord);
                geoStruct nextStruct(nextCoord, end, dist,connectingSegments[i].name);
                
                coordQueue.push(nextStruct);
                locationOfPreviousCoord.associate(nextStruct, current);
                geoCoordVisited.insert(nextCoord);
            }
        }
    }
    return DELIVERY_SUCCESS;
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}

