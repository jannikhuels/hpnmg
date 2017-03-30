#include "Event.h"

namespace hpnmg {
    Event::Event() : generalDependencies(std::vector<Number>(0)), type(EventType::Root), time(0) {

    }

    Event::Event(EventType type, std::vector<Number> generalDependencies, int time): type(type), generalDependencies(generalDependencies), time(time) {

    }

    EventType Event::getEventType() const {return type;}
    void Event::setEventType(const EventType &eventType) {this->type = eventType;}

    std::vector<Number> Event::getGeneralDependencies() const{return generalDependencies;}
    void Event::setGeneralDependencies(const std::vector<Number> generalDependencies){this->generalDependencies = generalDependencies;}

    int Event::getTime() const{return time;}
    void Event::setTime(int time){this->time = time;}
}