#include "stepping.hh"

MySteppingAction::MySteppingAction(MyEventAction *eventAction)
{
    fEventAction = eventAction;
}

MySteppingAction::~MySteppingAction(){}

void MySteppingAction::UserSteppingAction(const G4Step *step){

    const G4Track* track = step->GetTrack();

    // ---- Hadronic Inelastic (Bertini) interaction tracking ----
    // Check the post-step process for hadronic inelastic interactions
    const G4VProcess* postProcess = step->GetPostStepPoint()->GetProcessDefinedStep();
    if(postProcess) {
        G4String procName = postProcess->GetProcessName();
        if(procName.find("Inelastic") != std::string::npos) {
            fEventAction->IncrementBertiniCount();
            fEventAction->SetHadronicInteraction(); 
            // Record only secondaries produced by the hadronic process itself
            // (exclude Cerenkov optical photons that may be created in the same step)
            const std::vector<const G4Track*>* secs = step->GetSecondaryInCurrentStep();
            if(secs) {
                for(const G4Track* sec : *secs) {
                    const G4VProcess* creator = sec->GetCreatorProcess();
                    if(creator && creator->GetProcessName().find("Inelastic") != std::string::npos) {
                        fEventAction->AddBertiniSecondary(sec->GetDefinition()->GetPDGEncoding());
                    }
                }
            }
        }
    }


    // Store parent momentum for Cherenkov photons created in this step
    const std::vector<const G4Track*>* secondaries = step->GetSecondaryInCurrentStep();
    if(secondaries){
        for(const G4Track* secondary : *secondaries){
            if(secondary->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()){
                const G4VProcess* creator = secondary->GetCreatorProcess();
                if(creator && creator->GetProcessName() == "Cerenkov"){
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
        
        // Detect reflection by checking if direction changed at a boundary
        G4StepPoint* preStepPoint = step->GetPreStepPoint();
        G4StepPoint* postStepPoint = step->GetPostStepPoint();
        
        // Get volumes
        G4VPhysicalVolume* preVolume = preStepPoint->GetPhysicalVolume();
        G4VPhysicalVolume* postVolume = postStepPoint->GetPhysicalVolume();
        
        // Check if we're at a boundary
        G4bool atBoundary = false;
        if(postVolume == nullptr || preVolume != postVolume) {
            atBoundary = true;
        }
        
        // If at boundary, check for direction change (reflection)
        if(atBoundary && stepLength > 0) {
            G4ThreeVector preDirection = preStepPoint->GetMomentumDirection();
            G4ThreeVector postDirection = postStepPoint->GetMomentumDirection();
            
            G4double angle = preDirection.angle(postDirection);
            
            if(angle > 0.1) {
                fPhotonReflectionCount[trackID]++;
            }
        }
        
        // When photon is killed, record data
        if(track->GetTrackStatus() == fStopAndKill){
            G4int nReflections = fPhotonReflectionCount[trackID];
            G4double pathLength = fPhotonPathLength[trackID];
            
            G4int evt = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
            
            // NEW: Get wavelength
            G4ThreeVector momentum = track->GetMomentum();
            G4double wavelength = (1.239841939*eV/momentum.mag())*1E+03; // in nm
            
            // Get detector info
            const G4VTouchable* touchable = preStepPoint->GetTouchable();
            G4int copyNumber = -1;
            G4int reachedEnd = 0;  // NEW: flag for reaching detector
            
            if(touchable->GetVolume()){
                G4String volumeName = touchable->GetVolume()->GetName();
                if(volumeName == "physDetector"){
                    copyNumber = touchable->GetCopyNumber();
                    reachedEnd = 1;  // NEW: photon reached the end!
                }
            }
            
            // Record to analysis manager
            G4AnalysisManager *man = G4AnalysisManager::Instance();
            
            man->FillNtupleIColumn(9, 0, evt);
            man->FillNtupleIColumn(9, 1, trackID);
            man->FillNtupleIColumn(9, 2, nReflections);
            man->FillNtupleDColumn(9, 3, pathLength/mm);
            man->FillNtupleIColumn(9, 4, copyNumber + 1);  // 0=lost, 1=det1, 2=det2
            man->FillNtupleDColumn(9, 5, wavelength);       // NEW
            man->FillNtupleIColumn(9, 6, reachedEnd);       // NEW
            man->AddNtupleRow(9);
            
            // Clean up
            fPhotonReflectionCount.erase(trackID);
            fPhotonPathLength.erase(trackID);
        }
        
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