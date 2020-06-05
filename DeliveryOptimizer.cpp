#include "provided.h"
#include <math.h>
#include <list>
#include <vector>
#include <random>
#include <map>

using namespace std;

struct GeoObj;
class DeliveryOptimizerImpl {
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,double& newCrowDistance) const;
    
private:
    double getTotalDistance(vector<DeliveryRequest>& deliveries, const GeoCoord& depot, vector<GeoObj>& aux) const;
    double getTotalEuclidian(vector<DeliveryRequest>& deliveries, const GeoCoord& depot) const;
    vector<DeliveryRequest> getRandomChange(vector<DeliveryRequest>& deliveries) const;
    PointToPointRouter ptpr;
};

struct GeoObj {
    GeoObj(GeoCoord x, GeoCoord y, double d) : a(x), b(y), dis(d) {}
    GeoCoord a;
    GeoCoord b;
    double dis;
};

//checks if the distance has already been calculated between two points,
//meaning the shortest route has already been found between them, and the calculation
//need not be performed again
bool checkIfDistanceExists(GeoCoord first, GeoCoord second, double& distance, vector<GeoObj>& aux){
    for(int i = 0; i < aux.size(); i++){
        if((aux[i].a == first && aux[i].b == second) || (aux[i].b == first && aux[i].a == second)){
            distance = aux[i].dis;
            return true;
        }
    }
    return false;
}

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm) : ptpr(sm){
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl(){
}


//Any distance between two locations is only calculated once and then stored in a vector
double DeliveryOptimizerImpl::getTotalDistance(vector<DeliveryRequest>& deliveries, const GeoCoord& depot, vector<GeoObj>& aux) const{
    if(deliveries.size() == 0) return 0;
    list<StreetSegment> route;
    double total = 0, distance = 0;
    
    for(int i = 0; i < deliveries.size(); i++){
        if(i == 0){
            if(!checkIfDistanceExists(depot, deliveries[i].location, distance, aux)){
                ptpr.generatePointToPointRoute(depot, deliveries[i].location, route, distance);
                aux.push_back(GeoObj(depot, deliveries[i].location, distance));
            }
        } else {
            if(!checkIfDistanceExists(deliveries[i-1].location, deliveries[i].location, distance, aux)){
                ptpr.generatePointToPointRoute(deliveries[i-1].location, deliveries[i].location, route, distance);
                aux.push_back(GeoObj(deliveries[i-1].location, deliveries[i].location, distance));
            }
        }
        total += distance;
    }
    if(!checkIfDistanceExists(deliveries[deliveries.size()-1].location, depot, distance, aux)){
        ptpr.generatePointToPointRoute(deliveries[deliveries.size()-1].location, depot, route, distance);
        aux.push_back(GeoObj(deliveries[deliveries.size()-1].location, depot, distance));
    }
    total += distance;
   
    return total;
}

//calculates the euclidian distances through the given delivery route
double DeliveryOptimizerImpl::getTotalEuclidian(vector<DeliveryRequest>& deliveries, const GeoCoord& depot) const{
    if(deliveries.size() == 0) return 0;
    double distance = 0;
    for(int i = 0; i < deliveries.size(); i++){
        if(i == 0){
            distance += distanceEarthMiles(deliveries[i].location, depot);
        } else {
            distance += distanceEarthMiles(deliveries[i-1].location, deliveries[i].location);
        }
    }
    distance += distanceEarthMiles(deliveries[deliveries.size()-1].location, depot);
    return distance;
}

vector<DeliveryRequest> DeliveryOptimizerImpl::getRandomChange(vector<DeliveryRequest>& deliveries) const{
    vector<DeliveryRequest> newDeliveryOrder(deliveries.begin(), deliveries.end());
    int pos1 = 0, pos2 = 0;
    while(pos1 == pos2){
        pos1 = (int) rand()%newDeliveryOrder.size();
        pos2 = (int) rand()%newDeliveryOrder.size();
    }
    DeliveryRequest temp = newDeliveryOrder[pos1];
    newDeliveryOrder[pos1] = newDeliveryOrder[pos2];
    newDeliveryOrder[pos2] = temp;
    return newDeliveryOrder;
}



void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot, vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance, double& newCrowDistance) const
{
    oldCrowDistance = getTotalEuclidian(deliveries, depot);
    vector<GeoObj> aux;
    
    double temp = 1000;
    double distanceChange = 0;
    double coolingRate = 0.99;
    double minTemp = 0.01;
    double distance = getTotalDistance(deliveries, depot, aux);

    while (temp > minTemp){
        //BELOW FOR TESTING ONLY
        //cerr << "Distance: " << distance << "  Temperature: " << temp << endl;
        
        vector<DeliveryRequest> possibleDeliveryRoute = getRandomChange(deliveries);
        distanceChange = getTotalDistance(possibleDeliveryRoute, depot, aux) - distance;

        uniform_real_distribution<double> unif(0,1);
        default_random_engine re;
        
        if ((distanceChange < 0) || (distance > 0 && exp(-distanceChange / temp) > unif(re) )){
            deliveries = possibleDeliveryRoute;
            distance = distanceChange + distance;
        }

        temp *= coolingRate;
    }
    newCrowDistance = distance;
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm){
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer(){
    delete m_impl;
}

//DeliveryRequest(std::string it, const GeoCoord& loc)
void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot, vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}


//BELOW FOR TESTING
//int main(){
//    StreetMap smap;
//    smap.load("mapdata.txt");
//    DeliveryOptimizer dopt(&smap);
//    double oldC = 0, newC = 0;
//    vector<DeliveryRequest> deliveries{
//            DeliveryRequest("A", GeoCoord("34.0419265","-118.5010322")),
//            DeliveryRequest("B", GeoCoord("34.0379395", "-118.4946729")),
//            DeliveryRequest("Z", GeoCoord("34.0508737", "-118.4947372")),
//            DeliveryRequest("C", GeoCoord("34.0431928","-118.4958717")),
//            DeliveryRequest("D", GeoCoord("34.0438060", "-118.4914386")),
//            DeliveryRequest("E", GeoCoord("34.0414298", "-118.4838109")),
//            DeliveryRequest("E", GeoCoord("34.0884064", "-118.4432976")),
//
//        };
//    dopt.optimizeDeliveryOrder(GeoCoord("34.0468376", "-118.4494449"), deliveries, oldC, newC);
//}
