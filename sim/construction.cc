#include "construction.hh"

MyDetectorConstruction::MyDetectorConstruction(){
    fMessenger = new G4GenericMessenger(this, "/detector/", "Detector construction");

    fMessenger->DeclareProperty("nCols", nCols,"Number of columns");
    fMessenger->DeclareProperty("nRows", nRows,"Number of rows");
    fMessenger->DeclareProperty("isCherenkov", isCherenkov, "Toggle Cherenkov setup");
    fMessenger->DeclareProperty("isScintillator", isScintillator, "Toggle Scintillator setup");
    fMessenger->DeclareProperty("isTOF", isTOF, "Toggle TOF setup");
    fMessenger->DeclareProperty("isAtmosphere", isAtmosphere, "Toggle Atmosphere setup");
    fMessenger->DeclareProperty("isFusedSilica", isFusedSilica, "Toggle Fused silica bar interal reflection setup");
    fMessenger->DeclareProperty("isFusedSilicaProx", isFusedSilicaProx, "Toggle Fused silica proximity focus setup");
    fMessenger->DeclareProperty("radiatorThickness", radiatorThickness, "Thickness of the Cherenkov radiator");

    // Inizializza tutti i puntatori a nullptr
    solidRadiator = nullptr;
    solidDetector = nullptr;
    solidMirror = nullptr;
    logicRadiator = nullptr;
    logicDetector = nullptr;
    logicMirror = nullptr;

    nCols = 50;
    nRows = 50;

    radiatorThickness = 0.1*mm;

    DefineMaterial();

    xWorld = 0.5*m;         // half length
    yWorld = 0.5*m;
    zWorld = 0.5*m;

    isFusedSilica = true;
    isFusedSilicaProx = false;
    isCherenkov = false;
    isScintillator = false;
    isTOF = false;   
    isAtmosphere = false;
    isGasPM = false;
}

MyDetectorConstruction::~MyDetectorConstruction(){
    delete fMessenger;
}

// This connects our logicDetctor to the sensitive detector concept -> Now the detector is sensitive
void MyDetectorConstruction::ConstructSDandField(){
    MySensitiveDetector *sensDet = new MySensitiveDetector("SensitiveDetector");
    
    if(logicDetector != NULL){
        logicDetector->SetSensitiveDetector(sensDet);
    }
}

void MyDetectorConstruction::ConstructFusedSilicaProx(){
    // IMPORTANTE: Cancella e ricrea il solidRadiator con il nuovo spessore
    if(solidRadiator != nullptr) {
        delete solidRadiator;
    }
    
    G4double baseRadius = 5*mm;
    G4double height = 1.11 * baseRadius;  // full height
    G4double dz = height / 2.0;           // half-height

    solidRadiatorProx = new G4Cons("solidRadiator",
                                    0.,          // inner radius at -z (tip)
                                    baseRadius,          // outer radius at -z (tip)
                                    0.,          // inner radius at +z (base)
                                    0.,  // outer radius at +z (base)
                                    dz,          // half-height
                                    0.,          // start angle
                                    360*deg);    // delta angle
    
    // Ricrea anche il logicRadiator per applicare le modifiche
    if(logicRadiator != nullptr) {
        delete logicRadiator;
    }
    
    logicRadiator = new G4LogicalVolume(solidRadiatorProx, SiO2, "logicRadiator");
    
    physRadiator = new G4PVPlacement(0, G4ThreeVector(0,0,0.25*m), logicRadiator, "physRadiator", logicWorld, false, 0, true);

    // Colore (R, G, B, opacità)
    G4VisAttributes* radiatorVisAtt = new G4VisAttributes(G4Colour(0.0, 0.8, 1.0, 1.0));
    radiatorVisAtt->SetVisibility(true);
    radiatorVisAtt->SetForceSolid(true);   // superficie piena, non wireframe
    logicRadiator->SetVisAttributes(radiatorVisAtt);

    fScoringVolume = logicRadiator;

    if(solidDetector != nullptr){
        delete solidDetector;
    }

    G4double arraySize = 1.0*m;  // Area totale da coprire
    G4double gap = 0.1*mm;

    solidDetector = new G4Box("solidDetector", 
                          (arraySize/nRows)/2.0 - gap, 
                          (arraySize/nCols)/2.0 - gap, 
                          0.01*m);

    if(logicDetector != nullptr){
        delete logicDetector;
    }

    logicDetector = new G4LogicalVolume(solidDetector, worldMat, "logicDetector");
    logicDetector->SetVisAttributes(new G4VisAttributes(G4Colour(1,0,0,0.5)));
    
    for(G4int i = 0; i < nRows; i++){
        for(G4int j = 0; j < nCols; j++){
            physDetector = new G4PVPlacement(0, 
                G4ThreeVector(-arraySize/2.0 + (j+0.5)*arraySize/nCols,
                              -arraySize/2.0 + (i+0.5)*arraySize/nRows,
                              0.49*m - 0.01*m), 
                logicDetector, "physDetector", logicWorld, false, j + i*nCols, true);
        }
    }
}

void MyDetectorConstruction::ConstructFusedSilica(){
    // IMPORTANTE: Cancella e ricrea il solidRadiator con il nuovo spessore
    if(solidRadiator != nullptr) {
        delete solidRadiator;
    }
    
    solidRadiator = new G4Box("solidRadiator", 100*mm, 2*mm, radiatorThickness);
    
    // Ricrea anche il logicRadiator per applicare le modifiche
    if(logicRadiator != nullptr) {
        delete logicRadiator;
    }
    
    logicRadiator = new G4LogicalVolume(solidRadiator, SiO2, "logicRadiator");
    
    physRadiator = new G4PVPlacement(0, G4ThreeVector(0,0,0.15*m), logicRadiator, "physRadiator", logicWorld, false, 0, true);

    G4VisAttributes *radiatorVisAtt = new G4VisAttributes(G4Color(0.5, 0.5, 0., 1.));
    radiatorVisAtt->SetForceWireframe(true);
    logicRadiator->SetVisAttributes(radiatorVisAtt);

    fScoringVolume = logicRadiator;

    if(solidDetector != nullptr){
        delete solidDetector;
    }

    solidDetector = new G4Box("solidDetector", 10*mm, 5*mm, 5*mm);


    if(logicDetector != nullptr){
        delete logicDetector;
    }

    logicDetector = new G4LogicalVolume(solidDetector, worldMat, "logicDetector");
    logicDetector->SetVisAttributes(new G4VisAttributes(G4Colour(1,0,0,0.5)));
    
    physDetector = new G4PVPlacement(0, G4ThreeVector(105*mm, 0.*m, 0.15*m), logicDetector, "physDetector", logicWorld, false, 0, true);
    physDetector = new G4PVPlacement(0, G4ThreeVector(-105*mm, 0.*m, 0.15*m), logicDetector, "physDetector", logicWorld, false, 1, true);
}


void MyDetectorConstruction::ConstructCherenkov(){

    // IMPORTANTE: Cancella e ricrea il solidRadiator con il nuovo spessore
    if(solidRadiator != nullptr) {
        delete solidRadiator;
    }
    
    solidRadiator = new G4Box("solidRadiator", 0.4*m, 0.4*m, radiatorThickness);
    
    // Ricrea anche il logicRadiator per applicare le modifiche
    if(logicRadiator != nullptr) {
        delete logicRadiator;
    }
    
    logicRadiator = new G4LogicalVolume(solidRadiator, Aerogel, "logicRadiator");
    
    physRadiator = new G4PVPlacement(0, G4ThreeVector(0,0,0.15*m), logicRadiator, "physRadiator", logicWorld, false, 0, true);

    G4VisAttributes *radiatorVisAtt = new G4VisAttributes(G4Color(0.5, 0.5, 0., 1.));
    radiatorVisAtt->SetForceWireframe(true);
    logicRadiator->SetVisAttributes(radiatorVisAtt);

    fScoringVolume = logicRadiator;

    rotX = new G4RotationMatrix();
    rotX->rotateX(-45*degree);

    // Spherical Mirror
    if(solidRadiator != nullptr) {
        solidMirror = new G4Sphere("solidMirror", 0.5*m, 0.51*m, 0*degree, 360*degree, 0*degree, 30*degree);
        logicMirror = new G4LogicalVolume(solidMirror, worldMat, "logicMirror");
        skin = new G4LogicalSkinSurface("skin", logicMirror, mirrorSurface);
        physMirror = new G4PVPlacement(0, G4ThreeVector(0, 0, -0.1*m), logicMirror, "physMirror", logicWorld, false, 0, true);

        G4VisAttributes *mirrorVisAtt = new G4VisAttributes(G4Color(0.8, 0.8, 0.8, 0.3));
        mirrorVisAtt->SetForceWireframe(true);
        logicMirror->SetVisAttributes(mirrorVisAtt);

        solidMirror_1 = new G4Box("solidMirror_1", 0.25*m,0.25*m, 0.005*m);
        logicMirror_1 = new G4LogicalVolume(solidMirror_1, worldMat, "logicMirror_1");
        skin_1 = new G4LogicalSkinSurface("skin_1", logicMirror_1, mirrorSurface);
        physMirror_1 = new G4PVPlacement(rotX, G4ThreeVector(0, 0, -0.2*m), logicMirror_1, "physMirror_1", logicWorld, false, 0, true);

        G4VisAttributes *mirrorVisAtt_1 = new G4VisAttributes(G4Color(0.1, 0.8, 0.9, 0.4));
        mirrorVisAtt_1->SetForceWireframe(true);
        logicMirror_1->SetVisAttributes(mirrorVisAtt_1);
    }

    if(solidDetector != nullptr){
        delete solidDetector;
    }

    solidDetector = new G4Box("solidDetector", 
                          (0.3*m/nRows)/2.0 - 0.1*mm, // Half-length minus gap
                          0.01*m, 
                          (0.3*m/nCols)/2.0 - 0.1*mm);

    if(logicDetector != nullptr){
        delete logicDetector;
    }

    logicDetector = new G4LogicalVolume(solidDetector, worldMat, "logicDetector");
    logicDetector->SetVisAttributes(new G4VisAttributes(G4Colour(1,0,0,0.5)));
    
    for(G4int i = 0; i < nRows; i++){
        for(G4int j = 0; j < nCols; j++){
            physDetector = new G4PVPlacement(0, G4ThreeVector(-0.15*m+(i+0.5)*0.3*m/nRows, -0.49*m, -0.35*m+(j+0.5)*0.3*m/nCols),
            logicDetector, "physDetector", logicWorld, false, j+i*nCols, true);
        }
    }

    // Stampa di debug per confermare il nuovo spessore
    G4cout << "=== RADIATOR THICKNESS: " << radiatorThickness/mm << " mm ===" << G4endl;
}

void MyDetectorConstruction::ConstructScintillator(){

    // solidScintillator = new G4Tubs("solidScintillator", 10*cm, 20*cm, 30*cm, 0.*deg, 360.*deg);
    solidScintillator = new G4Box("solidScintillator", 5*cm, 5*cm, 6*cm);

    logicScintillator = new G4LogicalVolume(solidScintillator, NaI, "logicScintillator");

    skin = new G4LogicalSkinSurface("skin", logicWorld, mirrorSurface);

    // fScoringVolume = logicScintillator;

    solidDetector = new G4Box("solidDetector", 1*cm, 5*cm, 6*cm);

    logicDetector  = new G4LogicalVolume(solidDetector, worldMat, "logicDetector");

    for(G4int i = 0; i < 6; i++){
        for(G4int j = 0; j < 16; j++){

            G4Rotate3D rotZ(j*22.5*deg, G4ThreeVector(0,0,1));
            G4Translate3D transXScint(G4ThreeVector(5./tan(22.5/2*deg)*cm + 5.*cm, 0*cm, -40*cm + i*15*cm));
            G4Translate3D transXDet(G4ThreeVector(5./tan(22.5/2*deg)*cm + 6.*cm + 5*cm, 0*cm, -40*cm + i*15*cm));

            G4Transform3D transformScint = (rotZ)*(transXScint);
            G4Transform3D transformDet = (rotZ)*(transXDet);

            physScintillator = new G4PVPlacement(transformScint, logicScintillator, "physScintillator", logicWorld, false, 0, true);
            physDetector = new G4PVPlacement(transformDet, logicDetector, "physDetector", logicWorld, false, 0, true);
        }
    }
}

void MyDetectorConstruction::ConstructTOF(){
    solidDetector = new G4Box("solidDetector", 1*m, 1*m, 0.1*m);
    logicDetector = new G4LogicalVolume(solidDetector, worldMat, "logicDetector");
    physDetector = new G4PVPlacement(0, G4ThreeVector(0.*m, 0.*m, -4*m), logicDetector, "physDetector", logicWorld, false, 0, true);
    physDetector = new G4PVPlacement(0, G4ThreeVector(0.*m, 0.*m, -3*m), logicDetector, "physDetector", logicWorld, false, 1, true);
}

void MyDetectorConstruction::ConstructAtmosphere(){
    solidAtmosphere = new G4Box("solidAtmosphere", xWorld, yWorld, zWorld/10.);

    for (G4int i =0; i < 10; i++){
        logicAtmosphere[i] = new G4LogicalVolume(solidAtmosphere, Air[i], "logicAtmosphere");
        physAtmosphere[i] = new G4PVPlacement(0, G4ThreeVector(0, 0, zWorld/10.*2*i - zWorld + zWorld/10.), logicAtmosphere[i], "physAtmosphere", logicWorld, false, i, true);
    }
}

void MyDetectorConstruction::ConstructGasPM(){
    solidWindow = new G4Box("solidWindow", 0.1*m, 0.1*m, 0.005*m);
    logicWindow = new G4LogicalVolume(solidWindow, MgF2, "logicWindow");
    physWindow = new G4PVPlacement(0, G4ThreeVector(0.*m, 0.1*m, 0.25*m), logicWindow, "physWindow", logicWorld, false, 0, true);

    G4VisAttributes *radiatorVisAtt = new G4VisAttributes(G4Color(1., 0.5, 0., 1.));
    radiatorVisAtt->SetForceWireframe(true);
    logicWindow->SetVisAttributes(radiatorVisAtt);
}

G4VPhysicalVolume *MyDetectorConstruction::Construct(){
    
    // Create the solid
    solidWorld = new G4Box("solidWorld", xWorld, yWorld, zWorld); // half lengths

    // Logical volume -> insert the material into the solid
    logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

    // To make the world invisible
    G4VisAttributes* worldVis = new G4VisAttributes();
    worldVis->SetVisibility(false);
    logicWorld->SetVisAttributes(worldVis);

    // Physical volume -> place the logical volume into the physical world [This is mother volume]
    physWorld = new G4PVPlacement(0, G4ThreeVector(0,0,0), logicWorld, "physWorld", 0, false, 0, true);


    if(isFusedSilica){
        ConstructFusedSilica();
        G4cout << "=== USING FUSED SILICA AS RADIATOR MATERIAL ===" << G4endl;
    }   

    if(isFusedSilicaProx){
        ConstructFusedSilicaProx();
        G4cout << "=== USING FUSED SILICA IN PROXIMITY FOCUS GEOMETRY ===" << G4endl;
    }   

    if(isCherenkov){
        ConstructCherenkov();
        G4cout << "=== CHERENKOV GEOMETRY CONSTRUCTED ===" << G4endl;
    }

    if(isScintillator){
        ConstructScintillator();
    }

    if(isTOF){
        ConstructTOF();
    }
    
    if(isAtmosphere){
        ConstructAtmosphere();
    }

    if(isGasPM){
        ConstructGasPM();
    }

    return physWorld;
}

void MyDetectorConstruction::DefineMaterial(){
    G4NistManager *nist = G4NistManager::Instance();

    // Create a volume -> It needs 3 parts (solid, logical(including the material), physical(placement))
    // Create a material -> function takes name, density and components
    // FUSED SILICA
    SiO2 = new G4Material("SiO2", 2.201*g/cm3, 2);
    SiO2->AddElement(nist->FindOrBuildElement("Si"), 1);
    SiO2->AddElement(nist->FindOrBuildElement("O"), 2);
    
    H2O = new G4Material("H2O", 1.000*g/cm3, 2);
    H2O->AddElement(nist->FindOrBuildElement("H"), 2);
    H2O->AddElement(nist->FindOrBuildElement("O"), 1);

    C = nist->FindOrBuildElement("C"); 

    Aerogel = new G4Material("Areogel", 0.200*g/cm3, 3);
    Aerogel->AddMaterial(SiO2, 62.5*perCent);
    Aerogel->AddMaterial(H2O, 37.4*perCent);
    Aerogel->AddElement(C, 0.1*perCent);

    worldMat = nist->FindOrBuildMaterial("G4_AIR");

    Na = nist->FindOrBuildElement("Na");
    I = nist->FindOrBuildElement("I");
    Mg = nist->FindOrBuildElement("Mg");
    F = nist->FindOrBuildElement("F");

    NaI = new G4Material("NaI", 3.67*g/cm3, 2);
    NaI->AddElement(Na, 1);
    NaI->AddElement(I, 1);

    MgF2 = new G4Material("MgF2", 3.13*g/cm3, 2);
    MgF2->AddElement(Mg, 1);
    MgF2->AddElement(F, 2);

    G4int nEntries = 15;
    G4double energy[nEntries] = {1.239841939*eV/0.9, 1.239841939*eV/0.636, 1.239841939*eV/0.554, 1.239841939*eV/0.517, 1.239841939*eV/0.466, 
        1.239841939*eV/0.401, 1.239841939*eV/0.378, 1.239841939*eV/0.341, 1.239841939*eV/0.318, 1.239841939*eV/0.287, 1.239841939*eV/0.268, 
        1.239841939*eV/0.250, 1.239841939*eV/0.233,1.239841939*eV/0.225, 1.239841939*eV/0.21}; // 0.2um - 0.9um,depends on the wavelength
    G4double rindexAerogel[2] = {1.1, 1.1}; // range of refractive index
    G4double rindexAir[2] = {1.0, 1.0}; 
    G4double rindexNaI[2] = {1.78, 1.78}; 
    G4double rindexMgF2[2] = {1.38, 1.42}; 
    G4double rindexSiO2[nEntries] = {1.451324, 1.456926, 1.459756, 1.461445, 1.464436, 1.469529, 1.472705, 1.478468, 1.483150, 1.491821, 1.499000, 1.507610, 1.518042, 1.524079, 1.538358};
    G4double rindexH2O[2] = {1.33, 1.33}; 
    G4double fraction[nEntries] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};   // Fraction of light emitted in the fast component
    G4double reflectivity[nEntries] = {0.95, 0.95};   // Fraction of reflected photons

    G4MaterialPropertiesTable *mptAerogel = new G4MaterialPropertiesTable();
    mptAerogel->AddProperty("RINDEX", energy, rindexAerogel, 2);    

    G4MaterialPropertiesTable *mptWorld = new G4MaterialPropertiesTable();
    mptWorld->AddProperty("RINDEX", energy, rindexAir, 2);

    G4MaterialPropertiesTable *mptH2O = new G4MaterialPropertiesTable();
    mptH2O->AddProperty("RINDEX", energy, rindexH2O, 2);

    // G4MaterialPropertiesTable *mptMgF2 = new G4MaterialPropertiesTable();
    // mptMgF2->AddProperty("RINDEX", energy, rindexMgF2, 2);

    G4MaterialPropertiesTable *mptSiO2 = new G4MaterialPropertiesTable();
    mptSiO2->AddProperty("RINDEX", energy, rindexSiO2, nEntries);

    // These information you can usually find online
    G4MaterialPropertiesTable *mptNaI = new G4MaterialPropertiesTable();
    mptNaI->AddProperty("RINDEX", energy, rindexNaI, 2);
    mptNaI->AddProperty("SCINTILLATIONCOMPONENT1", energy, fraction, 2);  // ✓ Use predefined key
    mptNaI->AddConstProperty("SCINTILLATIONYIELD", 38./keV);
    mptNaI->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 250.*ns);  // ✓ Renamed from FASTTIMECONSTANT
    mptNaI->AddConstProperty("RESOLUTIONSCALE", 1.0);
    
    NaI->SetMaterialPropertiesTable(mptNaI);  
    Aerogel->SetMaterialPropertiesTable(mptAerogel);
    worldMat->SetMaterialPropertiesTable(mptWorld);  
    H2O->SetMaterialPropertiesTable(mptH2O);
    SiO2->SetMaterialPropertiesTable(mptSiO2);
    // MgF2->SetMaterialPropertiesTable(mptMgF2);
    
    mirrorSurface = new G4OpticalSurface("mirrorSurface");
    // Just default staff but important for the reflective skin
    mirrorSurface->SetType(dielectric_metal);
    mirrorSurface->SetFinish(polished);
    mirrorSurface->SetModel(unified);

    G4MaterialPropertiesTable *mptMirror = new G4MaterialPropertiesTable();
    mptMirror->AddProperty("REFLECTIVITY", energy, reflectivity, 2);
    mirrorSurface->SetMaterialPropertiesTable(mptMirror);  

    G4double density0 = 1.29*kg/m3;
    G4double aN = 14.01*g/mole;     // Nitrogen mole mass
    G4double aO = 16.00*g/mole;     // Oxygen mole mass

    N = new G4Element("Nitrogen", "N", 7., aN);
    O = new G4Element("Oxygen", "O", 8., aO);

    G4double f = 3;
    G4double R = 8.3144621;
    G4double g0 = 9.81;
    G4double T = 293.15; 
    G4double kappa = (f+2)/f;
    G4double M = (0.3*aO +0.7*aN)/1000.;

    for (G4int i = 0; i  < 10; i++){
        std::stringstream stri;
        stri << i;      // i conveerted to a string
        G4double h = 40e3/10. * i;
        G4double density = density0 * pow((1-(kappa)/kappa*M*g0*h/(R*T)), (1/(kappa-1)));
        Air[i] = new G4Material("G4_AIR_"+stri.str(), density, 2);
        Air[i]->AddElement(N, 0.7);
        Air[i]->AddElement(O, 0.3);             
    }
}