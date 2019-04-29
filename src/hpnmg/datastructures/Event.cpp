#include "Event.h"

namespace hpnmg {
    Event::Event() : generalDependencies(vector<double>(0)), type(EventType::Root), time(0) {

    }

    Event::Event(int numberOfGeneralTransitions) : generalDependencies(vector<double>(numberOfGeneralTransitions, 0)), type(EventType::Root), time(0) {

    }

    Event::Event(EventType type, std::vector<double> generalDependencies, double time): type(type), generalDependencies(generalDependencies), time(time) {

    }

    Event::Event(const Event &event) : type(event.type), generalDependencies(event.generalDependencies),
    time(event.time), immediateTransitionMember(event.immediateTransitionMember),
    generalTransitionMember(event.generalTransitionMember),
    deterministicTransitionMember(event.deterministicTransitionMember), arcMember(event.arcMember),
    placeMember(event.placeMember) {}

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

    shared_ptr<ImmediateTransition> Event::getImmediateTransitionMember() const {return immediateTransitionMember;};
    void Event::setImmediateTransitionMember(shared_ptr<ImmediateTransition> immediateTransitionMember) {this->immediateTransitionMember = immediateTransitionMember;};

    shared_ptr<GeneralTransition> Event::getGeneralTransitionMember() const {return generalTransitionMember;};
    void Event::setGeneralTransitionMember(shared_ptr<GeneralTransition> generalTransitionMember) {this->generalTransitionMember = generalTransitionMember;};

    shared_ptr<DeterministicTransition> Event::getDeterministicTransitionMember() const {return deterministicTransitionMember;};
    void Event::setDeterministicTransitionMember(shared_ptr<DeterministicTransition> deterministicTransitionMember) {this->deterministicTransitionMember = deterministicTransitionMember;};

    shared_ptr<GuardArc> Event::getArcMember() const {return arcMember;};
    void Event::setArcMember(shared_ptr<GuardArc> arcMember) {this->arcMember = arcMember;};

    shared_ptr<ContinuousPlace> Event::getPlaceMember() const {return placeMember;};
    void Event::setPlaceMember(shared_ptr<ContinuousPlace> placeMember) {this->placeMember = placeMember;};
}