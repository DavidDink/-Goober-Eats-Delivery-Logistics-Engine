//
//  DeliveryPlanner.cpp
//  Goober Eats
//
//  Created by David Dinklage on 3/6/20.
//  Copyright © 2020 David Dinklage. All rights reserved.
//


#include "provided.h"
#include <vector>
#include <cassert>

using namespace std;

vector<DeliveryCommand> segmentsToCommands (list<StreetSegment>& segments, DeliveryRequest& request)
{
    vector<DeliveryCommand> commandVec;
    // iterate through the segments
    
    if ( segments.empty() )
    {
        DeliveryCommand deliver;
        deliver.initAsDeliverCommand(request.item);
        commandVec.push_back(deliver);
        return commandVec;
    }
    
    GeoCoord uninit;
    StreetSegment previousSegment(uninit, uninit, "");
    
    for (list<StreetSegment>::iterator it = segments.begin(); it != segments.end() ; it++)
    {
        double segmentLength = distanceEarthMiles(it->start, it->end);
        string streetName = it->name;
        double angleBetweenCurrentAndPrevious = angleBetween2Lines(previousSegment, (*it));
        double streetAngle = angleOfLine(*it);
        
        bool doProceedCommand = false;
        
        DeliveryCommand command;
        // at depot or after delivery
        if (commandVec.empty())
            doProceedCommand = true;
        // change streets but not direction
        if (!commandVec.empty() && previousSegment.name != streetName && (angleBetweenCurrentAndPrevious < 1 || angleBetweenCurrentAndPrevious > 359))
            doProceedCommand = true;
        
        // left turn
        if (!commandVec.empty() && previousSegment.name != streetName && angleBetweenCurrentAndPrevious >= 1 && angleBetweenCurrentAndPrevious < 180)
        {
            command.initAsTurnCommand("left", streetName);
            commandVec.push_back(command);
            doProceedCommand = true;
        }
        
        // right turn
        if (!commandVec.empty() && previousSegment.name != streetName && angleBetweenCurrentAndPrevious >= 180 && angleBetweenCurrentAndPrevious <= 359)
        {
            command.initAsTurnCommand("right", streetName);
            commandVec.push_back(command);
            doProceedCommand = true;
        }
        
        // update segment
        if (!commandVec.empty() && previousSegment.name == streetName)
            commandVec.back().increaseDistance(segmentLength);
        
        // do proceed command
        if (doProceedCommand)
        {
            string direction;
            if (streetAngle >= 0 && streetAngle < 22.5)
                direction = "east";
            if (streetAngle >= 22.5 && streetAngle < 67.5)
                direction = "northeast";
            if (streetAngle >= 67.5 && streetAngle < 112.5)
                direction = "north";
            if (streetAngle >= 112.5 && streetAngle < 157.5)
                direction = "northwest";
            if (streetAngle >= 157.5 && streetAngle < 202.5)
                direction = "west";
            if (streetAngle >= 202.5 && streetAngle < 247.5)
                direction = "southwest";
            if (streetAngle >= 247.5 && streetAngle < 292.5)
                direction = "south";
            if (streetAngle >= 292.5 && streetAngle < 337.5)
                direction = "southeast";
            if (streetAngle >= 337.5)
                direction = "east";
            
            command.initAsProceedCommand(direction, streetName, segmentLength);
            commandVec.push_back(command);
        }

        // our segment is the end segment, so we must deliver too
        if ( it->end == request.location)
        {
            DeliveryCommand deliver;
            deliver.initAsDeliverCommand(request.item);
            commandVec.push_back(deliver);
            return commandVec;
        }
        
        previousSegment = (*it);
    }
    return commandVec;
}

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_streetMap;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
{
    m_streetMap = sm;
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    
    totalDistanceTravelled = 0;
    
    // first, reorder the deliveries vector
    double oldCrows;
    double newCrows;
    DeliveryOptimizer optimizer(m_streetMap);
    vector<DeliveryRequest> orderedDeliveries = deliveries;
    optimizer.optimizeDeliveryOrder(depot, orderedDeliveries, oldCrows, newCrows);
    
    // initilaize some temp data structures that we need
    PointToPointRouter segmentRouter(m_streetMap);
    list<StreetSegment> segmentRoute;
    vector<DeliveryCommand> commandsPerSegment;
    double segmentDistance;
    DeliveryResult routerResult;
    
    // generate point to point route from depot to first location
    routerResult = segmentRouter.generatePointToPointRoute(depot, orderedDeliveries[0].location, segmentRoute, segmentDistance);
    if ( routerResult != DELIVERY_SUCCESS )
        return routerResult;
    totalDistanceTravelled += segmentDistance;
    
    // convert the segment route to commands, and add the commands to the commands vector
    commandsPerSegment = segmentsToCommands(segmentRoute, orderedDeliveries[0]);
    for ( int k = 0 ; k < commandsPerSegment.size() ; k++)
        commands.push_back(commandsPerSegment[k]);
    commandsPerSegment.clear();
    
    // for each ordered delivery point, find point to point route between
    for (int i = 0 ; i < orderedDeliveries.size() - 1 ; i++)
    {
        routerResult = segmentRouter.generatePointToPointRoute(orderedDeliveries[i].location, orderedDeliveries[i+1].location, segmentRoute, segmentDistance);
        if ( routerResult != DELIVERY_SUCCESS )
        return routerResult;
        totalDistanceTravelled += segmentDistance;
        
        // convert segment route to commands, and add the commands to the commands vector
        commandsPerSegment = segmentsToCommands(segmentRoute, orderedDeliveries[i + 1]); // start at 1 because we already delivered 0
        for ( int k = 0 ; k < commandsPerSegment.size() ; k++)
            commands.push_back(commandsPerSegment[k]);
        commandsPerSegment.clear();
    }
    
    // find point to point route from last ordered delivery
    routerResult = segmentRouter.generatePointToPointRoute(orderedDeliveries[orderedDeliveries.size() - 1].location, depot, segmentRoute, segmentDistance);
    if ( routerResult != DELIVERY_SUCCESS )
    return routerResult;
    totalDistanceTravelled += segmentDistance;
    
    // convert segment route to commands, and add the commands to the commands vector
    
    // we must make a psuedo delivery to the depot, that we can later delete
    
    DeliveryRequest endRequest("TO THE DEPOT", depot);
    commandsPerSegment = segmentsToCommands(segmentRoute, endRequest);
    for ( int k = 0 ; k < commandsPerSegment.size() ; k++)
        commands.push_back(commandsPerSegment[k]);
    commandsPerSegment.clear();
    
    // now just remove that last pseudo delivery request
    commands.pop_back();
    
    return DELIVERY_SUCCESS;
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}


