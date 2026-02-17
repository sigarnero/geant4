#include "physics.hh"

MyPhysicsList::MyPhysicsList(){
    // Define physics processes here
    RegisterPhysics(new G4EmStandardPhysics());
    RegisterPhysics(new G4OpticalPhysics());
    // RegisterPhysics(new G4HadronPhysicsFTFP_BERT());  // Inelastic hadronic
    // RegisterPhysics(new G4HadronElasticPhysics());
    // RegisterPhysics(new G4StoppingPhysics()); 
    // RegisterPhysics(new G4DecayPhysics());
    // RegisterPhysics(new G4RadioactiveDecayPhysics());
}

MyPhysicsList::~MyPhysicsList(){}