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
    fParticleGun->SetParticleMomentum(8. * GeV);
    // fParticleGun->SetParticleEnergy(2. * MeV);
    fParticleGun->SetParticleDefinition(particle);
}


// When irradiating photons
// MyPrimaryGenerator::MyPrimaryGenerator(){
//     fParticleGun = new G4ParticleGun(1);

//     G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
//     particleName = "proton";   // <-- importante, vedi nota sotto
//     particle = particleTable->FindParticle(particleName);

//     G4ThreeVector mom(0., 0., 1.);

//     fParticleGun->SetParticleMomentumDirection(mom);
//     // fParticleGun->SetParticleEnergy(3.0996 * eV);   // <-- non SetParticleMomentum (questo per protoni)
//     fParticleGun->SetParticleDefinition(particle);
// }

MyPrimaryGenerator::~MyPrimaryGenerator(){
    delete fParticleGun;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent){    
    // Sigma = 6 mm in both x and y
    G4double sigma = 6.0*mm;
    
    G4double x = G4RandGauss::shoot(0.0, sigma);  // Mean = 0, sigma = 6 mm
    G4double y = G4RandGauss::shoot(0.0, sigma);  // Mean = 0, sigma = 6 mm
    G4double z = -0.5*m;  // Starting position in z (adjust as needed)
    // G4double z = 0.1*m; 

    
    G4ThreeVector position(x, y, z);
    // G4ThreeVector position(0, 0, z);
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

// void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent){    
//     // ============================================================
//     // UNICO PARAMETRO DA CAMBIARE: angolo di emissione rispetto
//     // all'asse X (asse longitudinale della barra), nel piano XY
//     // ============================================================
//     G4double emissionAngle = 44.0*deg;   // <-- cambia solo questo - Questo è l angolo rispetto a z, quello di emissione Cherenkov è 90 - questo

//     G4ThreeVector direction(std::sin(emissionAngle), 0., std::cos(emissionAngle));
//     fParticleGun->SetParticleMomentumDirection(direction);

//     G4ThreeVector polarization = ComputeOrthogonalPolarization(direction);
//     fParticleGun->SetParticlePolarization(polarization);

//     // Posizione fissa: coordinate assolute nel mondo (coerenti con la tua geometria)
//     // Esempio: appena dentro l'estremità -x della barra in ConstructFusedSilicaBarTest
//     G4double barCenterZ = 0.2*m;
//     G4double x0 = 9.0*mm;   // dentro la barra, vicino a un'estremità
//     G4double y0 = 0.0*mm;
//     G4double z0 = barCenterZ-0.095*m;

//     G4ThreeVector position(x0, y0, z0);
//     fParticleGun->SetParticlePosition(position);

//     G4AnalysisManager *man = G4AnalysisManager::Instance();
//     man->FillNtupleIColumn(10, 0, anEvent->GetEventID());
//     man->FillNtupleDColumn(10, 1, x0/mm);
//     man->FillNtupleDColumn(10, 2, y0/mm);
//     man->FillNtupleDColumn(10, 3, z0/mm);
//     man->AddNtupleRow(10);

//     fParticleGun->GeneratePrimaryVertex(anEvent);
// }

// void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent){
//     // Geometria della barra (deve coincidere con construction.cc!)
//     const G4double barRadius  = 5.0*mm;
//     const G4double barHalfLen = 100.0*mm;
//     const G4double barCenterZ = 0.2*m;
//     const G4double z = barCenterZ - barHalfLen;   // faccia d'ingresso, LED a contatto

//     // --- Posizione: uniforme sul disco della faccia d'ingresso ---
//     G4double r   = barRadius * std::sqrt(G4UniformRand());
//     G4double phi = 2.*CLHEP::pi*G4UniformRand();
//     G4double x = r*std::cos(phi);
//     G4double y = r*std::sin(phi);
//     G4ThreeVector position(x, y, z);
//     fParticleGun->SetParticlePosition(position);

//     // --- Direzione: cono di semiapertura 15° attorno a +z ---
//     G4double halfAngle   = 15.0*deg;
//     G4double cosThetaMax = std::cos(halfAngle);
//     G4double u = G4UniformRand();

//     // OPZIONE A - uniforme in angolo solido:
//     // G4double cosTheta = 1.0 - u*(1.0 - cosThetaMax);

//     // OPZIONE B - Lambertiana troncata (default consigliato):
//     G4double cosTheta = std::sqrt(cosThetaMax*cosThetaMax + u*(1.0 - cosThetaMax*cosThetaMax));

//     G4double sinTheta = std::sqrt(1.0 - cosTheta*cosTheta);
//     G4double phiDir = 2.*CLHEP::pi*G4UniformRand();

//     G4ThreeVector direction(sinTheta*std::cos(phiDir), sinTheta*std::sin(phiDir), cosTheta);
//     fParticleGun->SetParticleMomentumDirection(direction);

//     // ntuple 10: ora x,y sono coerenti con la posizione realmente usata
//     G4AnalysisManager *man = G4AnalysisManager::Instance();
//     man->FillNtupleIColumn(10, 0, anEvent->GetEventID());
//     man->FillNtupleDColumn(10, 1, x/mm);
//     man->FillNtupleDColumn(10, 2, y/mm);
//     man->FillNtupleDColumn(10, 3, z/mm);
//     man->AddNtupleRow(10);

//     fParticleGun->GeneratePrimaryVertex(anEvent);
// }

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

G4ThreeVector MyPrimaryGenerator::ComputeOrthogonalPolarization(const G4ThreeVector& dir)
{
    G4ThreeVector d = dir.unit();

    // Riferimento: Z, a meno che dir non sia quasi parallelo a Z (allora uso Y)
    G4ThreeVector ref(0., 0., 1.);
    if(std::abs(d.dot(ref)) > 0.99){
        ref = G4ThreeVector(0., 1., 0.);
    }

    G4ThreeVector polarization = d.cross(ref);
    polarization = polarization.unit();

    return polarization;
}
