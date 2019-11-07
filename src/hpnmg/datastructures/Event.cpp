#include <ParametricLocationTree.h>
#include "Event.h"

namespace hpnmg {
    Event::Event() : generalDependencies(vector<double>(0)), type(EventType::Root), time(0) {

    }

    Event::Event(int numberOfGeneralTransitions) : generalDependencies(vector<double>(numberOfGeneralTransitions, 0)), type(EventType::Root), time(0) {

    }


    Event::Event(EventType type, std::vector<double> generalDependencies, double time): type(type), generalDependencies(generalDependencies), time(time){

    }

//    Event::Event() : generalDependencies(vector<double>(0)), type(EventType::Root), time(0) {
//
//    }

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


    std::string Event::getMemberID(){


        switch (this->type){

            case Immediate:
                return  (*(this->immediateTransitionMember)).getId();

            case General:
                return  (*(this->generalTransitionMember)).getId();

            case Timed:
                return  (*(this->deterministicTransitionMember)).getId();

            case Continuous:
                return  (*(this->placeMember)).id;

            case Root :
                return "";

            default:
                return "";

        }
    }
}