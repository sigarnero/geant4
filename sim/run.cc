#include "run.hh"

MyRunAction::MyRunAction(){

    G4AnalysisManager *man = G4AnalysisManager::Instance();

    man->CreateH1("Edep", "Energy deposited", 100, 0., 1.1*GeV);

    // Ntuple 0: MODIFIED - added detector ID column
    man->CreateNtuple("Photons", "Photons");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleIColumn("fDetectorID");  // NEW
    man->CreateNtupleDColumn("fX");
    man->CreateNtupleDColumn("fY");
    man->CreateNtupleDColumn("fZ");
    man->CreateNtupleDColumn("fWavelength");
    man->CreateNtupleDColumn("fTime");
    man->FinishNtuple(0); 

    // Ntuple 1: MODIFIED - added detector ID column
    man->CreateNtuple("Hits", "Hits");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleIColumn("fDetectorID");  // NEW
    man->CreateNtupleDColumn("fX");
    man->CreateNtupleDColumn("fY");
    man->CreateNtupleDColumn("fZ");
    man->CreateNtupleDColumn("fWavelength");
    man->CreateNtupleDColumn("fTime");
    man->FinishNtuple(1);

    // Ntuple 2: UNCHANGED
    man->CreateNtuple("Scoring", "Scoring");
    man->CreateNtupleDColumn("fEdep");
    man->FinishNtuple(2);  
    
    // Ntuple 3: UNCHANGED
    man->CreateNtuple("Cherenkov", "Cherenkov photons");
    man->CreateNtupleDColumn("thetaC_rad");
    man->CreateNtupleDColumn("thetaC_deg");
    man->FinishNtuple(3);

    // Ntuple 4: UNCHANGED
    man->CreateNtuple("PhotonCount", "Cherenkov Photons per Event");
    man->CreateNtupleIColumn("nPhotons");
    man->FinishNtuple(4);

    man->CreateH1("CherenkovAngle", "Cherenkov Angle Distribution", 100, 0., 50.0);

    // NEW Ntuples for coincidence detection start at index 5

    // Ntuple 5: Detector 1 time distribution
    man->CreateNtuple("Detector1Times", "Detector 1 Time Distribution");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleIColumn("fDetectorID");
    man->CreateNtupleDColumn("fTime");
    man->FinishNtuple(5);

    // Ntuple 6: Detector 2 time distribution
    man->CreateNtuple("Detector2Times", "Detector 2 Time Distribution");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleIColumn("fDetectorID");
    man->CreateNtupleDColumn("fTime");
    man->FinishNtuple(6);

    // Ntuple 7: Coincidences
    man->CreateNtuple("Coincidences", "Coincidence Events");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("fTime1");
    man->CreateNtupleDColumn("fTime2");
    man->CreateNtupleDColumn("fTimeDiff");
    man->CreateNtupleDColumn("fTimeAvg");
    man->FinishNtuple(7);

    // Ntuple 8: Event summary
    man->CreateNtuple("EventSummary", "Event Summary");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleIColumn("fNDet1");
    man->CreateNtupleIColumn("fNDet2");
    man->CreateNtupleIColumn("fNCoincidences");
    man->FinishNtuple(8);
}

MyRunAction::~MyRunAction(){}

void MyRunAction::BeginOfRunAction(const G4Run* run){

    G4AnalysisManager *man = G4AnalysisManager::Instance();

    G4int runNumber = run->GetRunID();
    std::stringstream strRunID;
    strRunID << runNumber;

    man->OpenFile("output"+strRunID.str()+".root");
}

void MyRunAction::EndOfRunAction(const G4Run*){

    G4AnalysisManager *man = G4AnalysisManager::Instance();
    
    man->Write();
    man->CloseFile();

}