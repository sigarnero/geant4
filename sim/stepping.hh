#ifndef STEPPING_HH
#define STEPPING_HH

#include "G4UserSteppingAction.hh"
#include "G4Step.hh"
#include "G4OpticalPhoton.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4ProcessManager.hh"
#include "G4RunManager.hh"

#include "event.hh"
#include "construction.hh"

class MySteppingAction : public G4UserSteppingAction
{
public:
    MySteppingAction(MyEventAction* eventAction);
    virtual ~MySteppingAction();

    virtual void UserSteppingAction(const G4Step*);

private:
    MyEventAction* fEventAction;
    
    // Map to track reflections per photon
    std::map<G4int, G4int> fPhotonReflectionCount;
    std::map<G4int, G4double> fPhotonPathLength;
};

#endif