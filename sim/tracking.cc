#include "tracking.hh"

MyTrackingAction::MyTrackingAction(MyEventAction* eventAction)
    : fEventAction(eventAction)
{}

MyTrackingAction::~MyTrackingAction() {}

void MyTrackingAction::PreUserTrackingAction(const G4Track* track){
    if(track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()){
        const G4VProcess* creator = track->GetCreatorProcess();
        if(creator && creator->GetProcessName() == "Cerenkov"){
            // Increment the photon counter
            fEventAction->IncrementCherenkovPhotonCount();
            
            G4ThreeVector photonDir = track->GetMomentumDirection();
            G4ThreeVector parentDir = fEventAction->GetParentMomentum(track->GetTrackID()); 
            double angle = parentDir.angle(photonDir);
            double angleDeg = angle * 180.0 / CLHEP::pi;

            // G4cout << "========== Cherenkov Photon Info ==========" << G4endl;
            // G4cout << "Recording Cherenkov angle. Track ID: " << track->GetTrackID() 
            //    << " Step number: " << track->GetCurrentStepNumber()
            //    << " Angle: " << (parentDir.angle(photonDir) * 180.0 / CLHEP::pi) << " deg" << G4endl;
            // G4cout << "=========================================" << G4endl;

            G4AnalysisManager *man = G4AnalysisManager::Instance();
            man->FillH1(1, angleDeg);
            man->FillNtupleDColumn(3, 0, angle);
            man->FillNtupleDColumn(3, 1, angleDeg);
            man->AddNtupleRow(3);
        }
    }
}
