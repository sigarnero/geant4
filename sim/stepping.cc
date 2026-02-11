#include "stepping.hh"

MySteppingAction::MySteppingAction(MyEventAction *eventAction)
{
    fEventAction = eventAction;
}

MySteppingAction::~MySteppingAction(){}

void MySteppingAction::UserSteppingAction(const G4Step *step){

    const G4Track* track = step->GetTrack();

    // Store parent momentum for Cherenkov photons created in this step
    const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();
    if(secondaries){
        for(const G4Track* secondary : *secondaries){
            if(secondary->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()){
                const G4VProcess* creator = secondary->GetCreatorProcess();
                if(creator && creator->GetProcessName() == "Cerenkov"){
                    // Store the parent's momentum direction
                    G4ThreeVector parentMomentum = track->GetMomentumDirection();
                    fEventAction->StoreParentMomentum(secondary->GetTrackID(), parentMomentum);
                }
            }
        }
    }

    // Stop opt cal photon tracking 
    if(track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()){
        return;
    }

    // Energy deposition for charged particles
    G4LogicalVolume *volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();
    const MyDetectorConstruction *detectorConstruction = 
        static_cast<const MyDetectorConstruction*> (G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    G4LogicalVolume *fScoringVolume = detectorConstruction->GetScoringVolume();

    if(volume != fScoringVolume) return;

    G4double edep = step->GetTotalEnergyDeposit();
    fEventAction->AddEdep(edep);
}
