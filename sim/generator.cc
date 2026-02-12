#include "generator.hh"

MyPrimaryGenerator::MyPrimaryGenerator(){
    fParticleGun = new G4ParticleGun(1);    // Number of particles generated per event

    // ParticleTable contains all the particles I can use
    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    particleName = "proton";
    particle = particleTable->FindParticle(particleName);

    // G4ThreeVector pos(0., 0., 0.);
    G4ThreeVector mom(0., 0., 1.);

    // fParticleGun->SetParticlePosition(pos);
    fParticleGun->SetParticleMomentumDirection(mom);
    fParticleGun->SetParticleMomentum(5. * GeV);
    fParticleGun->SetParticleDefinition(particle);
}

MyPrimaryGenerator::~MyPrimaryGenerator(){
    delete fParticleGun;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent){    
    // Sigma = 6 mm in both x and y
    G4double sigma = 6.0*mm;
    
    G4double x = G4RandGauss::shoot(0.0, sigma);  // Mean = 0, sigma = 6 mm
    G4double y = G4RandGauss::shoot(0.0, sigma);  // Mean = 0, sigma = 6 mm
    G4double z = -0.5*m;  // Starting position in z (adjust as needed)
    
    G4ThreeVector position(x, y, z);
    fParticleGun->SetParticlePosition(position);

    G4AnalysisManager *man = G4AnalysisManager::Instance();
    man->FillNtupleIColumn(10, 0, anEvent->GetEventID());
    man->FillNtupleDColumn(10, 1, x/mm);
    man->FillNtupleDColumn(10, 2, y/mm);
    man->FillNtupleDColumn(10, 3, z/mm);
    man->AddNtupleRow(10);
    
    // Generate the primary vertex
    fParticleGun->GeneratePrimaryVertex(anEvent);
}

// MyPrimaryGenerator::MyPrimaryGenerator()
// {
//     fGPS = new G4GeneralParticleSource();  // No arguments!
// }

// MyPrimaryGenerator::~MyPrimaryGenerator()
// {
//     delete fGPS;
// }

// void MyPrimaryGenerator::GeneratePrimaries(G4Event* anEvent)
// {
//     fGPS->GeneratePrimaryVertex(anEvent);
// }
