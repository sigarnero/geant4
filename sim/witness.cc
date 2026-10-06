#include "witness.hh"

MyWitnessSD::MyWitnessSD(G4String name)
    : G4VSensitiveDetector(name)
{}

MyWitnessSD::~MyWitnessSD(){}

G4bool MyWitnessSD::ProcessHits(G4Step *aStep, G4TouchableHistory*)
{
    G4Track *track = aStep->GetTrack();

    // Escludi i fotoni Cherenkov: il pannello deve misurare solo il fondo
    // adronico dal polietilene, non la luce che sfugge dalla faccia del radiatore
    if(track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()){
        return false;
    }

    G4StepPoint *preStepPoint = aStep->GetPreStepPoint();

    G4double edep = aStep->GetTotalEnergyDeposit();  // sarà quasi sempre ~0 in aria: è normale, vedi nota sotto

    G4int pdg = track->GetDefinition()->GetPDGEncoding();
    G4double kinE = preStepPoint->GetKineticEnergy();
    G4ThreeVector pos = preStepPoint->GetPosition();
    G4double time = preStepPoint->GetGlobalTime();
    G4ThreeVector dir = preStepPoint->GetMomentumDirection();
    G4double angleFromBeamAxis = dir.angle(G4ThreeVector(0,0,1)) * 180.0/CLHEP::pi;  // deg, rispetto a Z

    const G4VTouchable *touchable = preStepPoint->GetTouchable();
    G4int side = touchable->GetCopyNumber();   // 0 = pannello +X, 1 = pannello -X

    G4int evt      = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    G4int trackID  = track->GetTrackID();
    G4int parentID = track->GetParentID();     // 0 = primario, >0 = secondaria

    G4AnalysisManager *man = G4AnalysisManager::Instance();
    man->FillNtupleIColumn(15, 0, evt);
    man->FillNtupleIColumn(15, 1, side);
    man->FillNtupleIColumn(15, 2, trackID);
    man->FillNtupleIColumn(15, 3, parentID);
    man->FillNtupleIColumn(15, 4, pdg);
    man->FillNtupleDColumn(15, 5, kinE/MeV);
    man->FillNtupleDColumn(15, 6, edep/MeV);
    man->FillNtupleDColumn(15, 7, pos.x()/mm);
    man->FillNtupleDColumn(15, 8, pos.y()/mm);
    man->FillNtupleDColumn(15, 9, pos.z()/mm);
    man->FillNtupleDColumn(15, 10, time/ns);
    man->FillNtupleDColumn(15, 11, angleFromBeamAxis);
    man->AddNtupleRow(15);

    return true;
}