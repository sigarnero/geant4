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
                    
                    // Initialize tracking for this photon
                    fPhotonReflectionCount[secondary->GetTrackID()] = 0;
                    fPhotonPathLength[secondary->GetTrackID()] = 0.0;
                }
            }
        }
    }

    // Track optical photon reflections and path length
    if(track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()){
        G4int trackID = track->GetTrackID();
        
        // Add step length to total path
        G4double stepLength = step->GetStepLength();
        fPhotonPathLength[trackID] += stepLength;
        
        // NEW: Detect reflection by checking if direction changed at a boundary
        G4StepPoint* preStepPoint = step->GetPreStepPoint();
        G4StepPoint* postStepPoint = step->GetPostStepPoint();
        
        // Get volumes
        G4VPhysicalVolume* preVolume = preStepPoint->GetPhysicalVolume();
        G4VPhysicalVolume* postVolume = postStepPoint->GetPhysicalVolume();
        
        // Check if we're at a boundary (different volumes or world boundary)
        G4bool atBoundary = false;
        if(postVolume == nullptr || preVolume != postVolume) {
            atBoundary = true;
        }
        
        // If at boundary, check for direction change (reflection)
        if(atBoundary && stepLength > 0) {
            G4ThreeVector preDirection = preStepPoint->GetMomentumDirection();
            G4ThreeVector postDirection = postStepPoint->GetMomentumDirection();
            
            // Calculate angle between directions
            G4double angle = preDirection.angle(postDirection);
            
            // If direction changed significantly (> 0.1 radians ≈ 5.7 degrees), count as reflection
            if(angle > 0.1) {
                fPhotonReflectionCount[trackID]++;
                
                // DEBUG
                G4String processName = "Unknown";
                const G4VProcess* process = postStepPoint->GetProcessDefinedStep();
                if(process) processName = process->GetProcessName();
                
                // G4cout << "Track " << trackID << " reflected! Process: " << processName 
                //        << " Angle change: " << angle*180/3.14159 << " deg"
                //        << " Total reflections: " << fPhotonReflectionCount[trackID] << G4endl;
            }
        }
        
        // When photon is killed, record data
        if(track->GetTrackStatus() == fStopAndKill){
            G4int nReflections = fPhotonReflectionCount[trackID];
            G4double pathLength = fPhotonPathLength[trackID];
            
            G4int evt = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
            
            // Get detector info
            const G4VTouchable* touchable = preStepPoint->GetTouchable();
            G4int copyNumber = -1;
            
            if(touchable->GetVolume()){
                G4String volumeName = touchable->GetVolume()->GetName();
                if(volumeName == "physDetector"){
                    copyNumber = touchable->GetCopyNumber();
                }
            }
            
            // Record to analysis manager
            G4AnalysisManager *man = G4AnalysisManager::Instance();
            
            man->FillNtupleIColumn(9, 0, evt);
            man->FillNtupleIColumn(9, 1, trackID);
            man->FillNtupleIColumn(9, 2, nReflections);
            man->FillNtupleDColumn(9, 3, pathLength/mm);
            man->FillNtupleIColumn(9, 4, copyNumber + 1);
            man->AddNtupleRow(9);
            
            // Clean up
            fPhotonReflectionCount.erase(trackID);
            fPhotonPathLength.erase(trackID);
        }
        
        return;
    }


    // Energy deposition for charged particles (existing code)
    G4LogicalVolume *volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();
    const MyDetectorConstruction *detectorConstruction = 
        static_cast<const MyDetectorConstruction*> (G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    G4LogicalVolume *fScoringVolume = detectorConstruction->GetScoringVolume();

    if(volume != fScoringVolume) return;

    G4double edep = step->GetTotalEnergyDeposit();
    fEventAction->AddEdep(edep);
}
