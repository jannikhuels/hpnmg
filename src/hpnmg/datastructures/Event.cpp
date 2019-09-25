#include <ParametricLocationTree.h>
#include "Event.h"

namespace hpnmg {
    Event::Event() : generalDependencies(vector<double>(0)), type(EventType::Root), time(0) {

    }

    Event::Event(int numberOfGeneralTransitions) : generalDependencies(vector<double>(numberOfGeneralTransitions, 0)), type(EventType::Root), time(0) {

    }

    Event::Event(EventType type, std::vector<double> generalDependencies, double time): type(type), generalDependencies(generalDependencies), time(time) {

    }

    Event::Event(const Event &event) : type(event.type), generalDependencies(event.generalDependencies), generalDependenciesNormed(event.generalDependenciesNormed), time(event.time) {

    }

    void Event::print() const {
        printf("t=%f [ ", getTime());
        for (double d : getGeneralDependencies()) {
            printf(" %f ", d);
        }
        printf(" ]\n");
    }

    EventType Event::getEventType() const {return type;}
    void Event::setEventType(const EventType &eventType) {this->type = eventType;}

    std::vector<double> Event::getGeneralDependencies() const{return generalDependencies;}
    void Event::setGeneralDependencies(const std::vector<double> generalDependencies){this->generalDependencies = generalDependencies;}

    std::vector<double> Event::getGeneralDependenciesNormed() const{return generalDependenciesNormed;}
    void Event::setGeneralDependenciesNormed(const std::vector<double> generalDependenciesNormed){this->generalDependenciesNormed = generalDependenciesNormed;}

  //  RateDependencies Event::getRateDependencies() const {return rateDependencies;}
  //  void Event::setRateDependencies(const RateDependencies rateDependencies) {this->rateDependencies = rateDependencies;}

    double Event::getTime() const{return time;}
    void Event::setTime(double time){this->time = time;}

    std::vector<double> Event::getTimeVector(int dimension) {
        std::vector<double> timeVector(dimension);
        timeVector[0] = this->time;
        for (int i = 1; i < dimension; i++) {
            if (i <= this->generalDependencies.size()) {
                timeVector[i] = this->generalDependencies[i-1];
            } else {
                timeVector[i] = 0;
            }
        }
        return timeVector;
    }

}