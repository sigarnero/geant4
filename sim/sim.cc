#include <iostream>
#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "G4HadronicParameters.hh"
#include "QGSP_BERT.hh"

#include "construction.hh"
#include "physics.hh"
#include "action.hh"
#include "detector.hh"
#include "run.hh"
#include "generator.hh"


int main(int argc, char** argv) {
    
    G4UIExecutive *ui = 0;
    G4RunManager *runManager = new G4RunManager();
    
    runManager->SetUserInitialization(new MyDetectorConstruction());
    runManager->SetUserInitialization(new MyPhysicsList());
    runManager->SetUserInitialization(new MyActionInitialization());
    
    // G4VModularPhysicsList *physics = new QGSP_BERT;
    // physics->RegisterPhysics(new G4DecayPhysics());
    // runManager->SetUserInitialization(physics);

    // G4HadronicParameters::Instance()->SetTimeThresholdForRadioactiveDecay( 1.0e+60*CLHEP::year );

    runManager->Initialize();
    
    if(argc == 1){
        ui = new G4UIExecutive(argc, argv);
    }

    auto *vismanager = new G4VisExecutive();
    vismanager->Initialize();

    G4UImanager *UImanager = G4UImanager::GetUIpointer();

    if(ui){
        UImanager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();
    }
    else{
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    }

    return 0;
}