#include "event.hh"
#include "detector.hh"
#include <cmath>

MyEventAction::MyEventAction(MyRunAction*)
{
    fEdep = 0.;
    fSensitiveDetector = nullptr;
    fCoincidenceWindow = 5.0*ns;  // Default 5 ns coincidence window
}

MyEventAction::~MyEventAction()
{}

void MyEventAction::AddEdep(G4double edep)
{
    fEdep += edep;
}

void MyEventAction::SetSensitiveDetector(MySensitiveDetector* det)
{
    fSensitiveDetector = det;
}

void MyEventAction::BeginOfEventAction(const G4Event*)
{
    fEdep = 0.;
    fCherenkovPhotonCount = 0;
    nPhotonsDetected = 0;
    fParentMomentumMap.clear();
    
    // Clear detector times
    if(fSensitiveDetector) {
        fSensitiveDetector->ClearEvent();
    }
}

void MyEventAction::EndOfEventAction(const G4Event* event)
{
    #ifndef G4MULTITHREADED
        G4cout << "Total energy deposited in event: " << fEdep  << G4endl;
    #endif

    G4AnalysisManager *man = G4AnalysisManager::Instance();
    G4int evt = event->GetEventID();

    // Existing ntuples (unchanged)
    man->FillH1(0, fEdep);
    man->FillNtupleDColumn(2, 0, fEdep);
    man->AddNtupleRow(2);

    man->FillNtupleIColumn(4, 0, fCherenkovPhotonCount);
    man->AddNtupleRow(4);

    // Coincidence analysis
    if(fSensitiveDetector) {
        const std::vector<G4double>& times1 = fSensitiveDetector->GetDetector1Times();
        const std::vector<G4double>& times2 = fSensitiveDetector->GetDetector2Times();

        // Ntuple 5: Time distribution for detector 1 (at +105mm)
        for(auto t : times1) {
            man->FillNtupleIColumn(5, 0, evt);
            man->FillNtupleIColumn(5, 1, 1);  // Detector ID
            man->FillNtupleDColumn(5, 2, t/ns);
            man->AddNtupleRow(5);
        }

        // Ntuple 6: Time distribution for detector 2 (at -105mm)
        for(auto t : times2) {
            man->FillNtupleIColumn(6, 0, evt);
            man->FillNtupleIColumn(6, 1, 2);  // Detector ID
            man->FillNtupleDColumn(6, 2, t/ns);
            man->AddNtupleRow(6);
        }

        // Ntuple 7: Coincidence events
        G4int nCoincidences = 0;
        for(auto t1 : times1) {
            for(auto t2 : times2) {
                G4double timeDiff = std::abs(t1 - t2);
                
                if(timeDiff < fCoincidenceWindow) {
                    man->FillNtupleIColumn(7, 0, evt);
                    man->FillNtupleDColumn(7, 1, t1/ns);
                    man->FillNtupleDColumn(7, 2, t2/ns);
                    man->FillNtupleDColumn(7, 3, timeDiff/ns);
                    man->FillNtupleDColumn(7, 4, (t1+t2)/(2*ns));
                    man->AddNtupleRow(7);
                    nCoincidences++;
                }
            }
        }

        // Ntuple 8: Event summary
        man->FillNtupleIColumn(8, 0, evt);
        man->FillNtupleIColumn(8, 1, times1.size());
        man->FillNtupleIColumn(8, 2, times2.size());
        man->FillNtupleIColumn(8, 3, nCoincidences);
        man->AddNtupleRow(8);
    }
}

void MyEventAction::StoreParentMomentum(G4int trackID, G4ThreeVector momentum)
{
    fParentMomentumMap[trackID] = momentum;
}

G4ThreeVector MyEventAction::GetParentMomentum(G4int trackID)
{
    if(fParentMomentumMap.find(trackID) != fParentMomentumMap.end()){
        return fParentMomentumMap[trackID];
    }
    return G4ThreeVector(0, 0, 1);
}