#ifndef STEPPING_HH
#define STEPPING_HH

#include "G4UserSteppingAction.hh"
#include "G4Step.hh"

#include "construction.hh"
#include "event.hh"
#include "detector.hh"
#include "G4OpticalPhoton.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4StepPoint.hh"

class MySteppingAction : public G4UserSteppingAction
{
    public:
        MySteppingAction(MyEventAction *eventAction);
        ~MySteppingAction();

        virtual void UserSteppingAction(const G4Step *step);
    
    private:
        MyEventAction *fEventAction;
};

#endif