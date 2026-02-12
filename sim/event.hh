#ifndef EVENT_HH
#define EVENT_HH

#include "G4UserEventAction.hh"
#include "G4Event.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include <vector>

#include "run.hh"

// Forward declaration
class MySensitiveDetector;

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
        void AddPhoton() { nPhotonsDetected++; }

        // Methods for coincidence detection
        void SetSensitiveDetector(MySensitiveDetector* det);
        void SetCoincidenceWindow(G4double window) { fCoincidenceWindow = window; }

    private:
        G4double fEdep;
        std::map<G4int, G4ThreeVector> fParentMomentumMap;
        G4int nPhotonsDetected;
        G4int fCherenkovPhotonCount;
        
        // For coincidence detection
        MySensitiveDetector* fSensitiveDetector;
        G4double fCoincidenceWindow;
};

#endif