#include "stepping.hh"

MySteppingAction::MySteppingAction(MyEventAction *eventAction)
{
    fEventAction = eventAction;
}

MySteppingAction::~MySteppingAction(){}

void MySteppingAction::UserSteppingAction(const G4Step *step){

    const G4Track* track = step->GetTrack();

    // 1) Cherenkov photon handling
    if(track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()){
        const G4VProcess* creator = track->GetCreatorProcess();
        if(creator && creator->GetProcessName() == "Cerenkov"){
            G4ThreeVector dir = track->GetMomentumDirection();
            G4ThreeVector parentDir = track->GetVertexMomentumDirection();
            double angle = parentDir.angle(dir); // radians
            double angleDeg = angle * 180.0 / CLHEP::pi;

            G4AnalysisManager *man = G4AnalysisManager::Instance();
            man->FillNtupleDColumn(3, 0, angle);
            man->FillNtupleDColumn(3, 1, angleDeg);
            man->AddNtupleRow(3);
        }
        return; // stop optical photons here
    }

    // 2) Energy deposition for charged particles
    G4LogicalVolume *volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();
    const MyDetectorConstruction *detectorConstruction = 
        static_cast<const MyDetectorConstruction*> (G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    G4LogicalVolume *fScoringVolume = detectorConstruction->GetScoringVolume();

    if(volume != fScoringVolume) return;

    G4double edep = step->GetTotalEnergyDeposit();
    fEventAction->AddEdep(edep);
}
