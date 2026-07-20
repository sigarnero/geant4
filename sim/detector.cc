#include "detector.hh"

MySensitiveDetector::MySensitiveDetector(G4String name) 
    : G4VSensitiveDetector(name)
{
    quEff = new G4PhysicsOrderedFreeVector();

    std::ifstream datafile;
    datafile.open("eff.dat");

    while(1){
        G4double wlen, queff;
        datafile >> wlen >> queff;

        if(datafile.eof()){
            break;
        }

        G4cout << wlen << " " << queff << G4endl;
        quEff->InsertValues(wlen, queff/100.);
    }
    datafile.close();
}

MySensitiveDetector::~MySensitiveDetector(){
    delete quEff;
}

G4bool MySensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *ROHist){
    G4Track *track = aStep->GetTrack();
    G4StepPoint *preStepPoint = aStep->GetPreStepPoint();

    // Trasforma la posizione globale del preStep in coordinate locali del volume
    const G4VTouchable *touchable = preStepPoint->GetTouchable();
    G4ThreeVector worldPos = preStepPoint->GetPosition();
    G4ThreeVector localPos = touchable->GetHistory()->GetTopTransform().TransformPoint(worldPos);

    // Semilarghezza x della box (deve combaciare con detHalfThicknessX in construction.cc)
    const G4double detHalfThicknessX = 10.0*mm; 
    const G4double tolerance = 1.0e-6*mm;   // tolleranza numerica

    // Verifica se il fotone è entrato dalla faccia frontale (quella a contatto col radiatore)
    // NB: per detector 1 (+105mm) la faccia "frontale" è a x_locale = -detHalfThicknessX
    //     per detector 2 (-105mm) la faccia "frontale" è a x_locale = +detHalfThicknessX
    G4bool enteredFromFrontFace = (std::abs(std::abs(localPos.x()) - detHalfThicknessX) < tolerance);

    if(!enteredFromFrontFace){
        track->SetTrackStatus(fStopAndKill);  // comunque termina il fotone, ma non lo conti
        return false;  // scarta l'hit
    }

    track->SetTrackStatus(fStopAndKill);

    G4StepPoint *postStepPoint = aStep->GetPostStepPoint();

    G4ThreeVector posPhoton = preStepPoint->GetPosition();
    G4ThreeVector momPhoton = preStepPoint->GetMomentum();

    G4double time = preStepPoint->GetGlobalTime();
    G4double wlen = (1.239841939*eV/momPhoton.mag())*1E+03;

    // const G4VTouchable *touchable = aStep->GetPreStepPoint()->GetTouchable();
    G4int copyNumber = touchable->GetCopyNumber();
    
    // NEW: Determine detector ID from copy number
    // copyNumber 0 = detector at +105mm = Detector 1
    // copyNumber 1 = detector at -105mm = Detector 2
    G4int detectorID = copyNumber + 1;

    G4VPhysicalVolume *physVol = touchable->GetVolume();
    G4ThreeVector posDetector = physVol->GetTranslation();

    G4int evt = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

    G4AnalysisManager *man = G4AnalysisManager::Instance();

    // Ntuple 0: All photons reaching the detector (before efficiency cut) 
    man->FillNtupleIColumn(0, 0, evt);
    man->FillNtupleIColumn(0, 1, detectorID);  // NEW COLUMN
    man->FillNtupleDColumn(0, 2, posPhoton[0]);
    man->FillNtupleDColumn(0, 3, posPhoton[1]);
    man->FillNtupleDColumn(0, 4, posPhoton[2]);
    man->FillNtupleDColumn(0, 5, wlen);
    man->FillNtupleDColumn(0, 6, time);
    man->AddNtupleRow(0);

    if(G4UniformRand() < quEff->Value(wlen)){   
        // Ntuple 1: Detected photons 
        man->FillNtupleIColumn(1, 0, evt);
        man->FillNtupleIColumn(1, 1, detectorID);  // NEW COLUMN
        man->FillNtupleDColumn(1, 2, posDetector[0]);
        man->FillNtupleDColumn(1, 3, posDetector[1]);
        man->FillNtupleDColumn(1, 4, posDetector[2]);
        man->FillNtupleDColumn(1, 5, posPhoton[0]);
        man->FillNtupleDColumn(1, 6, posPhoton[1]);
        man->FillNtupleDColumn(1, 7, posPhoton[2]);
        man->FillNtupleDColumn(1, 8, wlen);
        man->FillNtupleDColumn(1, 9, time);
        man->AddNtupleRow(1);

        // NEW: Store time in appropriate vector based on detector
        if(copyNumber == 0) {  // Detector 1 at +105mm
            fDetector1Times.push_back(time);
        } else if(copyNumber == 1) {  // Detector 2 at -105mm
            fDetector2Times.push_back(time);
        }
    }

    return true;
}