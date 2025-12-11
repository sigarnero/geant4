#include "generator.hh"

MyPrimaryGenerator::MyPrimaryGenerator(){
    fParticleGun = new G4ParticleGun(1);    // Number of particles generated per event

    // ParticleTable contains all the particles I can use
    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    particleName = "proton";
    particle = particleTable->FindParticle(particleName);

    G4ThreeVector pos(0., 0., 0.);
    G4ThreeVector mom(0., 0., 1.);

    fParticleGun->SetParticlePosition(pos);
    fParticleGun->SetParticleMomentumDirection(mom);
    fParticleGun->SetParticleMomentum(5. * GeV);
    fParticleGun->SetParticleDefinition(particle);
}

MyPrimaryGenerator::~MyPrimaryGenerator(){
    delete fParticleGun;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent){    
    fParticleGun->GeneratePrimaryVertex(anEvent);
}