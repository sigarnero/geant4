#include "run.hh"

MyRunAction::MyRunAction(){

    G4AnalysisManager *man = G4AnalysisManager::Instance();

    man->CreateH1("Edep", "Energy deposited", 100, 0., 1.1*GeV);

    man->CreateNtuple("Photons", "Photons");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleIColumn("fDetectorID");  
    man->CreateNtupleDColumn("fX");
    man->CreateNtupleDColumn("fY");
    man->CreateNtupleDColumn("fZ");
    man->CreateNtupleDColumn("fWavelength");
    man->CreateNtupleDColumn("fTime");
    man->FinishNtuple(0); 

    man->CreateNtuple("Hits", "Hits");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleIColumn("fDetectorID");  
    man->CreateNtupleDColumn("fX");
    man->CreateNtupleDColumn("fY");
    man->CreateNtupleDColumn("fZ");
    man->CreateNtupleDColumn("fWavelength");
    man->CreateNtupleDColumn("fTime");
    man->FinishNtuple(1);

    man->CreateNtuple("Scoring", "Scoring");
    man->CreateNtupleDColumn("fEdep");
    man->FinishNtuple(2);  
    
    man->CreateNtuple("Cherenkov", "Cherenkov photons");
    man->CreateNtupleDColumn("thetaC_rad");
    man->CreateNtupleDColumn("thetaC_deg");
    man->FinishNtuple(3);

    man->CreateNtuple("PhotonCount", "Cherenkov Photons per Event");
    man->CreateNtupleIColumn("nPhotons");
    man->FinishNtuple(4);

    man->CreateH1("CherenkovAngle", "Cherenkov Angle Distribution", 100, 0., 50.0);

    // Detector 1 time distribution
    man->CreateNtuple("Detector1Times", "Detector 1 Time Distribution");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleIColumn("fDetectorID");
    man->CreateNtupleDColumn("fTime");
    man->FinishNtuple(5);

    // Detector 2 time distribution
    man->CreateNtuple("Detector2Times", "Detector 2 Time Distribution");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleIColumn("fDetectorID");
    man->CreateNtupleDColumn("fTime");
    man->FinishNtuple(6);

    // Coincidences
    man->CreateNtuple("Coincidences", "Coincidence Events");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("fTime1");
    man->CreateNtupleDColumn("fTime2");
    man->CreateNtupleDColumn("fTimeDiff");
    man->CreateNtupleDColumn("fTimeAvg");
    man->FinishNtuple(7);

    // Event summary
    man->CreateNtuple("EventSummary", "Event Summary");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleIColumn("fNDet1");
    man->CreateNtupleIColumn("fNDet2");
    man->CreateNtupleIColumn("fNCoincidences");
    man->FinishNtuple(8);

    // Photon reflection and path statistics
    man->CreateNtuple("PhotonReflections", "Photon Reflections and Path");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleIColumn("fTrackID");
    man->CreateNtupleIColumn("fNReflections");
    man->CreateNtupleDColumn("fPathLength");      // in mm
    man->CreateNtupleIColumn("fDetectorID");      // 0=not detected, 1=det1, 2=det2
    man->CreateNtupleDColumn("fWavelength");      // wavelength in nm
    man->CreateNtupleIColumn("fReachedEnd");      // 1 if reached end, 0 if lost
    man->FinishNtuple(9);

    // Beam profile verification
    man->CreateNtuple("BeamProfile", "Initial Proton Positions");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("fX0");  // Initial x position
    man->CreateNtupleDColumn("fY0");  // Initial y position
    man->CreateNtupleDColumn("fZ0");  // Initial z position
    man->FinishNtuple(10);
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