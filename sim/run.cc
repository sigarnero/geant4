#include "run.hh"

MyRunAction::MyRunAction(){

    G4AnalysisManager *man = G4AnalysisManager::Instance();

    man->CreateH1("Edep", "Energy deposited", 100, 0., 1.1*GeV);

    // These are the photons that reach the detector volume
    man->CreateNtuple("Photons", "Photons");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("fX");
    man->CreateNtupleDColumn("fY");
    man->CreateNtupleDColumn("fZ");
    man->CreateNtupleDColumn("fWavelength");
    man->CreateNtupleDColumn("fTime");
    man->FinishNtuple(0); 

    // These are the photons detected (considering efficiency)
    man->CreateNtuple("Hits", "Hits");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("fX");
    man->CreateNtupleDColumn("fY");
    man->CreateNtupleDColumn("fZ");
    man->FinishNtuple(1);    // Number of tuples, you can create another one with more data and pass 1 etc.

    man->CreateNtuple("Scoring", "Scoring");
    man->CreateNtupleDColumn("fEdep");
    man->FinishNtuple(2);  
    
    man->CreateNtuple("Cherenkov", "Cherenkov photons");
    man->CreateNtupleDColumn("thetaC_rad");
    man->CreateNtupleDColumn("thetaC_deg");
    man->FinishNtuple(3);

    // Number of initially generted Cherenkov photons
    man->CreateNtuple("PhotonCount", "Cherenkov Photons per Event");
    man->CreateNtupleIColumn("nPhotons");
    man->FinishNtuple(4);

    man->CreateH1("CherenkovAngle", "Cherenkov Angle Distribution", 100, 0., 50.0);
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