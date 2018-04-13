#include "Event.h"

namespace hpnmg {
    Event::Event() : generalDependencies(vector<double>(0)), type(EventType::Root), time(0) {

    }

    Event::Event(int numberOfGeneralTransitions) : generalDependencies(vector<double>(numberOfGeneralTransitions, 0)), type(EventType::Root), time(0) {

    }

    Event::Event(EventType type, std::vector<double> generalDependencies, double time): type(type), generalDependencies(generalDependencies), time(time) {

    }

    Event::Event(const Event &event) : type(event.type), generalDependencies(event.generalDependencies), time(event.time) {

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

    double Event::getTime() const{return time;}
    void Event::setTime(double time){this->time = time;}
}