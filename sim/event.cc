#include "event.hh"

MyEventAction::MyEventAction(MyRunAction*)
{
    fEdep = 0.;
}

MyEventAction::~MyEventAction()
{}

void MyEventAction::AddEdep(G4double edep)
{
    fEdep += edep;
}

void MyEventAction::BeginOfEventAction(const G4Event*)
{
    fEdep = 0.;
    fParentMomentumMap.clear(); // Clear the map at the beginning of each event
}

void MyEventAction::EndOfEventAction(const G4Event*)
{
    G4cout << "Total energy deposited in event: " << fEdep  << G4endl;

    G4AnalysisManager *man = G4AnalysisManager::Instance();

    man->FillH1(0, fEdep);
    man->FillNtupleDColumn(2, 0, fEdep);
    man->AddNtupleRow(2);
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
    return G4ThreeVector(0, 0, 1); // Default fallback
}
