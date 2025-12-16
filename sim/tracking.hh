#ifndef TRACKING_HH
#define TRACKING_HH

#include "G4UserTrackingAction.hh"
#include "G4OpticalPhoton.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4AnalysisManager.hh"

#include "event.hh"

class MyTrackingAction : public G4UserTrackingAction
{
public:
    MyTrackingAction(MyEventAction*);
    virtual ~MyTrackingAction();
    
    virtual void PreUserTrackingAction(const G4Track* track);

private:
    MyEventAction* fEventAction;
};

#endif