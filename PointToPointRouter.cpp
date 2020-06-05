#include "provided.h"
#include <list>
#include <map>
#include <queue>
#include <set>
#include <algorithm>
using namespace std;

//struct HashF {
//    unsigned int hash(const GeoCoord& g);
//    unsigned int h()(const GeoCoord& a){
//        return hash(a);
//    }
//};

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
    const StreetMap* smap;
    vector<GeoCoord> reconstructPath(map<GeoCoord, GeoCoord>& cameFrom, GeoCoord current) const;
    void getStreetSegmentsFromPath(vector<GeoCoord> path, list<StreetSegment>& route) const;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm){
    smap = sm;
}

PointToPointRouterImpl::~PointToPointRouterImpl(){
}


vector<GeoCoord> PointToPointRouterImpl::reconstructPath(map<GeoCoord, GeoCoord>& cameFrom, GeoCoord current) const{
    vector<GeoCoord> totalPath;
    totalPath.push_back(current);
    while(cameFrom[current] != current){
        current = cameFrom[current];
        totalPath.push_back(current);
    }
    reverse(totalPath.begin(), totalPath.end());
    return totalPath;
}

GeoCoord smallestF(set<GeoCoord>& s, map<GeoCoord, double>& fScore){
    bool isSet = false;
    GeoCoord smallest;
    for(auto i = s.begin(); i != s.end(); i++){
        if(!isSet || fScore[(*i)] < fScore[smallest]){
            smallest = (*i);
            isSet = true;
        }
            
    }
    return smallest;
}

double calcDistance(vector<GeoCoord>& path){
    double distance = 0;
    for(auto it = path.begin()+1; it != path.end(); it++){
        distance += distanceEarthMiles(*(it-1), *it);
    }
    return distance;
}

double getGValue(map<GeoCoord, double>& fScore, GeoCoord cur){
    if(fScore.find(cur) == fScore.end()){
        //arbitrarily high number, will be replaced by lower g values in A* code
        fScore[cur] = 999999999;
        return fScore[cur];
    }
    return fScore[cur];
}

void PointToPointRouterImpl::getStreetSegmentsFromPath(vector<GeoCoord> path, list<StreetSegment>& route) const{
    list<StreetSegment> tempRoutes;
    for(int i = 0; i < path.size()-1; i++){
        vector<StreetSegment> n;
        smap->getSegmentsThatStartWith(path[i], n);
        for(int j = 0; j < n.size(); j++){
            if(n[j].end == path[i+1]){
                tempRoutes.push_back(StreetSegment(path[i], n[j].end, n[j].name));
            }
        }
    }
    route = tempRoutes;
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start, const GeoCoord& end,
        list<StreetSegment>& route, double& totalDistanceTravelled) const
{
    vector<StreetSegment> a;
    if(!smap->getSegmentsThatStartWith(start, a) || !smap->getSegmentsThatStartWith(end, a)){
        return BAD_COORD;
    }

    
    bool routeFound = false;
    vector<GeoCoord> path;
    map<GeoCoord, double> fScore;
    map<GeoCoord, double> gScore;
    
    //g score is distance from a node to starting node, h is heuristic score (euclidian distance from node to ending node)
    //f score is f = g + h(n)
    fScore[start] = distanceEarthMiles(start, end);
    gScore[start] = 0;
    
    set<GeoCoord> openSet;
    set<GeoCoord> closedSet;
    openSet.insert(start);
    
//    // For node n, cameFrom[n] is the node immediately preceding it on the cheapest path from start
//    // to n currently known.
    map<GeoCoord, GeoCoord> cameFrom;
    cameFrom[start] = start;

    while(!openSet.empty()){
        GeoCoord current = smallestF(openSet, fScore);
        //BELOW: USEFUL FOR TESTING
        //cerr << current.latitudeText << " " << current.longitudeText << " :: " << end.latitudeText << " " << end.longitudeText << endl;
        if(current == end){
            path = reconstructPath(cameFrom, current);
            routeFound = true;
            break;
        }

        openSet.erase(current);
        closedSet.insert(current);
        
        vector<StreetSegment> neighbors;
        smap->getSegmentsThatStartWith(current, neighbors);
        
        //checks all of the geoCoord's neighbors, potentially recalculates g and f scores
        for(auto it = neighbors.begin(); it != neighbors.end(); it++){
            GeoCoord neighbor = (*it).end;
            if(closedSet.find(neighbor) != closedSet.end())
                continue;
            if(openSet.find(neighbor) == openSet.end())
                openSet.insert(neighbor);
            
            double tentative_gScore = getGValue(gScore, current) + distanceEarthMiles(current, neighbor);
            if(tentative_gScore < getGValue(gScore, neighbor)){
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative_gScore;
                fScore[neighbor] = gScore[neighbor] + distanceEarthMiles(neighbor, end);
//                if(openSet.find(neighbor) == openSet.end())
//                    openSet.insert(neighbor);
            }
        }
    }

    if(!routeFound)
        return NO_ROUTE;
       
    //add up the total distance between all of the geocoordinates in the path
    totalDistanceTravelled = calcDistance(path);
    
    //using path, construct a vector of streetsegments of start GeoCoord to end GeoCoord AND calculate distance from start to end
    getStreetSegmentsFromPath(path, route);
    
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
        const GeoCoord& start, const GeoCoord& end,
        list<StreetSegment>& route, double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}



//int main(){
//    StreetMap sm;
//    sm.load("mapdata.txt");
//    PointToPointRouter ptpr(&sm);
//    list<StreetSegment> route;
//    double distance = 0;
//    ptpr.generatePointToPointRoute(GeoCoord("34.0419265", "-118.5010322"), GeoCoord("34.0304673", "-118.4876393"), route, distance);
//    for(auto it = route.begin(); it != route.end(); it++){
//        cout << (*it).start.latitudeText << " " << (*it).start.longitudeText << " :: " << (*it).end.latitudeText << " " << (*it).end.longitudeText << " :: " << (*it).name << endl;
//    }
//}

