#include "provided.h"
#include <vector>
#include <string>
using namespace std;

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
    PointToPointRouter ptpr;
    DeliveryOptimizer dopt;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm) : ptpr(sm), dopt(sm){
}

DeliveryPlannerImpl::~DeliveryPlannerImpl(){
}

string getDirectionFromAngle(double angle){
    if(angle >= 0 && angle < 22.5)
        return "east";
    else if(angle >= 22.5 && angle < 67.5)
        return "northeast";
    else if(angle >= 67.5 && angle < 112.5)
        return "north";
    else if(angle >= 112.5 && angle < 157.5)
        return "northwest";
    else if (angle >= 157.5 && angle < 202.5)
        return "west";
    else if(angle >= 202.5 && angle < 247.5)
        return "southwest";
    else if(angle >= 247.5 && angle < 292.5)
        return "south";
    else if (angle >= 292 && angle < 337.5)
        return "southeast";
    else
        return "east";
}

//return type: DELIVERY_SUCCESS, NO_ROUTE, BAD_COORD
DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot, const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands, double& totalDistanceTravelled) const
{
    double ocd = 0, ncd = 0, distance = 0;
    vector<DeliveryRequest> optDeliveries(deliveries.begin(), deliveries.end());
    dopt.optimizeDeliveryOrder(depot, optDeliveries, ocd, ncd);
    totalDistanceTravelled = ncd;
    
    //generate vector of all streetsegments representing delivery path
    list<StreetSegment> temp;
    vector<StreetSegment> allRoutes;
    DeliveryResult res;
    for(int i = 0; i < optDeliveries.size(); i++){
        if(i == 0){
            res = ptpr.generatePointToPointRoute(depot, optDeliveries[i].location, temp, distance);
            if(res == NO_ROUTE) return NO_ROUTE;
            if(res == BAD_COORD) return BAD_COORD;
        } else {
            res = ptpr.generatePointToPointRoute(optDeliveries[i-1].location, optDeliveries[i].location, temp, distance);
            if(res == NO_ROUTE) return NO_ROUTE;
            if(res == BAD_COORD) return BAD_COORD;
        }
        for(auto j = temp.begin(); j != temp.end(); j++)
            allRoutes.push_back((*j));
    }
    res = ptpr.generatePointToPointRoute(optDeliveries[optDeliveries.size()-1].location, depot, temp, distance);
    if(res == NO_ROUTE) return NO_ROUTE;
    if(res == BAD_COORD) return BAD_COORD;
    for(auto j = temp.begin(); j != temp.end(); j++)
        allRoutes.push_back((*j));

    if(allRoutes.size() == 0) return NO_ROUTE;
    
    //loop over point-to-point street segments, generate DeliveryCommands
    string prevStreetName = allRoutes[0].name;
    double streetDis = 0;
    int curDeliveryNum = 0;
    DeliveryRequest curDeliveryRequest = optDeliveries[curDeliveryNum];
    double startAngle = angleOfLine(allRoutes[0]);
    
    for(int i = 0; i < allRoutes.size(); i++){
        //3 CASES:
        //((1) on same street, issue no delivery commands, add distance
        // (2) at delivery location, issue proceed command on current road with current distance, issue delivery command, reset distance/angle
        // (3) at new street, issue proceed command for previous street, likely issue turn command onto new street, reset distance/angle/prevStreet
        DeliveryCommand proc, turn, deliver;
        if(allRoutes[i].start == curDeliveryRequest.location){
            if(streetDis > 0){
                proc.initAsProceedCommand(getDirectionFromAngle(startAngle), prevStreetName, streetDis);
                commands.push_back(proc);
            }
            
            deliver.initAsDeliverCommand(curDeliveryRequest.item);
            commands.push_back(deliver);
            curDeliveryNum++;
            startAngle = angleOfLine(allRoutes[i]);
            if(curDeliveryNum < optDeliveries.size())
                curDeliveryRequest = optDeliveries[curDeliveryNum];
            streetDis = 0;
        }
        else if(allRoutes[i].name == prevStreetName){
            streetDis += distanceEarthMiles(allRoutes[i].start, allRoutes[i].end);
        } else {
            if(streetDis > 0){
                proc.initAsProceedCommand(getDirectionFromAngle(startAngle), prevStreetName, streetDis);
                commands.push_back(proc);
            }
            
            double angleBetweenDiffStreets = angleBetween2Lines(allRoutes[i-1], allRoutes[i]);
            if(angleBetweenDiffStreets >= 1 && angleBetweenDiffStreets < 180){
                turn.initAsTurnCommand("left", allRoutes[i].name);
                commands.push_back(turn);
            }
            else if(angleBetweenDiffStreets >= 180 && angleBetweenDiffStreets <= 359){
                turn.initAsTurnCommand("right", allRoutes[i].name);
                commands.push_back(turn);
            }
      
            streetDis = 0;
            prevStreetName = allRoutes[i].name;
            startAngle = angleOfLine(allRoutes[i]);
            streetDis += distanceEarthMiles(allRoutes[i].start, allRoutes[i].end);
            
        }
        
        if(i == allRoutes.size() - 1){
            proc.initAsProceedCommand(getDirectionFromAngle(startAngle), prevStreetName, streetDis);
            commands.push_back(proc);
        }
    }
   
    
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

//BELOW FOR TESTING
//int main() {
//    StreetMap sm;
//    sm.load("mapdata.txt");
//    DeliveryPlanner dp(&sm);
//    vector<DeliveryCommand> dcs;
//    double distance = 0;
////     vector<DeliveryRequest> drs{
////                DeliveryRequest("Bread", GeoCoord("34.0419265","-118.5010322")),
////                DeliveryRequest("Fish", GeoCoord("34.0379395", "-118.4946729")),
////                DeliveryRequest("Milk", GeoCoord("34.0508737", "-118.4947372")),
////                DeliveryRequest("Burger", GeoCoord("34.0431928","-118.4958717")),
////                DeliveryRequest("Donut", GeoCoord("34.0438060", "-118.4914386")),
////                DeliveryRequest("Steak", GeoCoord("34.0414298", "-118.4838109")),
////                DeliveryRequest("Fries", GeoCoord("34.0884064", "-118.4432976")),
////
////    };
//    vector<DeliveryRequest> drs{
//        DeliveryRequest("Chicken tenders", GeoCoord("34.0712323", "-118.4505969")),
//        DeliveryRequest("B-Plate salmon", GeoCoord("34.0687443", "-118.4449195")),
//        DeliveryRequest("Pabst Blue Ribbon beer", GeoCoord("34.0685657", "-118.4489289"))
//    };
//    dp.generateDeliveryPlan(GeoCoord("34.0625329", "-118.4470263"), drs, dcs, distance);
//    for(int i = 0; i < dcs.size(); i++){
//        cout << dcs[i].description() << endl;
//    }
//    cout << "total distance " << distance << endl;
//}
//
//
//
//
