#include "stepping.hh"

MySteppingAction::MySteppingAction(MyEventAction *eventAction)
{
    fEventAction = eventAction;
    fBoundaryProcess = nullptr;
}

MySteppingAction::~MySteppingAction(){}

void MySteppingAction::FindBoundaryProcess()
{
    G4ProcessManager* opManager = G4OpticalPhoton::OpticalPhoton()->GetProcessManager();
    if(!opManager) return;

    G4ProcessVector* postStepDoItVector = opManager->GetPostStepProcessVector(typeDoIt);
    G4int nProcesses = postStepDoItVector->entries();

    for(G4int i = 0; i < nProcesses; i++){
        G4VProcess* proc = (*postStepDoItVector)[i];
        G4OpBoundaryProcess* opProc = dynamic_cast<G4OpBoundaryProcess*>(proc);
        if(opProc){
            fBoundaryProcess = opProc;
            return;
        }
    }

    // Diagnostica: se arriviamo qui, il processo non è stato trovato
    G4cerr << "WARNING: G4OpBoundaryProcess not found in process list!" << G4endl;
}

void MySteppingAction::UserSteppingAction(const G4Step *step){

    const G4Track* track = step->GetTrack();
    if(!fBoundaryProcess) FindBoundaryProcess();

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

        // ---- NEW: registra i veri incontri con le facce terminali (piano YZ) ----
        G4String preName  = preVolume  ? preVolume->GetName()  : "OutOfWorld";
        G4String postName = postVolume ? postVolume->GetName() : "OutOfWorld";

        // ---- NEW: log the very first optical boundary encounter per photon ----
        if(atBoundary && fBoundaryProcess && stepLength > 0 && preName == "physRadiator" && fFirstBoundaryLogged.find(trackID) == fFirstBoundaryLogged.end()){
            fFirstBoundaryLogged.insert(trackID);

            G4ThreeVector exitPos = postStepPoint->GetPosition();
            G4ThreeVector exitDir = preStepPoint->GetMomentumDirection();

            const G4double barHalfLength = 150.0*mm;
            const G4double barHalfY      = 15.0*mm;
            const G4double barHalfZ      = 5.0*mm;          // = radiatorThickness for this run
            const G4double posTolerance  = 1.0*um;

            const G4ThreeVector radiatorPivot(0, 0, 150.0*mm);  // must match construction.cc's pivot! If change tilt angle it breaks!!

            G4ThreeVector localPos = exitPos - radiatorPivot;   // NEW: convert to radiator-local frame

            G4int face = -1;
            G4ThreeVector normal;

            if(std::abs(std::abs(localPos.x()) - barHalfLength) < posTolerance){
                face = 2; normal = G4ThreeVector(1,0,0);
            } else if(std::abs(std::abs(localPos.y()) - barHalfY) < posTolerance){
                face = 0; normal = G4ThreeVector(0,1,0);
            } else if(std::abs(std::abs(localPos.z()) - barHalfZ) < posTolerance){
                face = 1; normal = G4ThreeVector(0,0,1);
            }

            if(face >= 0){
                G4double cosTheta = std::abs(exitDir.dot(normal));
                if(cosTheta > 1.0) cosTheta = 1.0;
                G4double angleIncidence = std::acos(cosTheta) * 180.0/CLHEP::pi;

                G4OpBoundaryProcessStatus status = fBoundaryProcess->GetStatus();
                G4int transmitted = (status == FresnelRefraction) ? 1 : 0;

                G4double wavelength = (1.239841939*eV/track->GetMomentum().mag())*1E+03;
                G4int evt = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

                G4AnalysisManager *man = G4AnalysisManager::Instance();
                man->FillNtupleIColumn(14, 0, evt);
                man->FillNtupleIColumn(14, 1, trackID);
                man->FillNtupleIColumn(14, 2, face);
                man->FillNtupleDColumn(14, 3, angleIncidence);
                man->FillNtupleIColumn(14, 4, transmitted);
                man->FillNtupleDColumn(14, 5, wavelength);
                man->AddNtupleRow(14);
            } else {
                static G4int nFaceMatchFail = 0;
                nFaceMatchFail++;
                if(nFaceMatchFail % 100 == 1){  // don't flood stdout
                    G4cerr << "FirstBounce: face match failed at pos ("
                           << exitPos.x()/mm << ", " << exitPos.y()/mm << ", " << exitPos.z()/mm
                           << ") mm" << G4endl;
                }
            }
        }

        if(atBoundary && fBoundaryProcess && preName == "physRadiator" && postName != "physRadiator"){

            G4ThreeVector exitPos = postStepPoint->GetPosition();

            const G4double barHalfLength = 150.0*mm;
            const G4double posTolerance  = 1.0*um;

            G4bool isEndFace = (std::abs(std::abs(exitPos.x()) - barHalfLength) < posTolerance);    // isFusedSilica = true;
            // G4bool isEndFace = (std::abs(std::abs(exitPos.z()) - 3*barHalfLength) < posTolerance);    // isFusedSilicaBarTest = true;

            if(isEndFace){
                const G4double minMeaningfulStep = 1.0e-9*mm;  // soglia, regolabile
                if(stepLength < minMeaningfulStep){
                // non è un nuovo bounce, è l'oscillazione numerica dello stesso punto --> non fa nulla
                } else {
                    G4ThreeVector exitDir = preStepPoint->GetMomentumDirection();

                    G4double angleFromAxis = exitDir.angle(G4ThreeVector(1,0,0)) * 180.0/CLHEP::pi;
                    if(angleFromAxis > 90.0) angleFromAxis = 180.0 - angleFromAxis;

                    G4int evt = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
                    G4int nReflSoFar   = fPhotonReflectionCount.count(trackID) ? fPhotonReflectionCount[trackID] : 0;
                    G4double pathSoFar = fPhotonPathLength.count(trackID) ? fPhotonPathLength[trackID] : 0.0;
                    G4double wavelength = (1.239841939*eV/track->GetMomentum().mag())*1E+03;
                    G4int wentToDetector = (postName == "physDetector") ? 1 : 0;

                    G4OpBoundaryProcessStatus status = fBoundaryProcess->GetStatus();
                    G4int transmitted = (status == FresnelRefraction) ? 1 : 0;

                    G4AnalysisManager *man = G4AnalysisManager::Instance();
                    man->FillNtupleIColumn(13, 0, evt);
                    man->FillNtupleIColumn(13, 1, trackID);
                    man->FillNtupleDColumn(13, 2, exitPos.x()/mm);
                    man->FillNtupleDColumn(13, 3, exitPos.y()/mm);
                    man->FillNtupleDColumn(13, 4, exitPos.z()/mm);
                    man->FillNtupleDColumn(13, 5, angleFromAxis);
                    man->FillNtupleIColumn(13, 6, nReflSoFar);
                    man->FillNtupleDColumn(13, 7, pathSoFar/mm);
                    man->FillNtupleDColumn(13, 8, wavelength);
                    man->FillNtupleIColumn(13, 9, wentToDetector);
                    man->FillNtupleIColumn(13, 10, transmitted);
                    man->AddNtupleRow(13);
                }
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
            fFirstBoundaryLogged.erase(trackID);
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