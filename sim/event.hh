#ifndef EVENT_HH
#define EVENT_HH

#include "G4UserEventAction.hh"
#include "G4Event.hh"
#include "G4AnalysisManager.hh"

#include "run.hh"

class MyEventAction : public G4UserEventAction{
    public:
        MyEventAction(MyRunAction*);
        ~MyEventAction();

        virtual void BeginOfEventAction(const G4Event*);
        virtual void EndOfEventAction(const G4Event*);

        void AddEdep(G4double edep);

        void StoreParentMomentum(G4int trackID, G4ThreeVector momentum);
        G4ThreeVector GetParentMomentum(G4int trackID);

        void IncrementCherenkovPhotonCount() { fCherenkovPhotonCount++; }

    private:
        G4double fEdep;
        std::map<G4int, G4ThreeVector> fParentMomentumMap;
        G4int fCherenkovPhotonCount;
};

#endif