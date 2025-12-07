#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4NistManager.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4PVPlacement.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "G4GenericMessenger.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4VisAttributes.hh"
#include "G4Color.hh"

#include "detector.hh"

class MyDetectorConstruction : public G4VUserDetectorConstruction{
    public:
        MyDetectorConstruction();
        ~MyDetectorConstruction();

        virtual G4VPhysicalVolume *Construct();
        G4LogicalVolume *GetScoringVolume() const { return fScoringVolume; }

    private:

        G4Box *solidWorld, *solidRadiator, *solidDetector, *solidScintillator, *solidAtmosphere;
        G4Sphere *solidMirror;
        // G4Tubs *solidScintillator;
        G4LogicalVolume *logicWorld, *logicRadiator, *logicDetector, *logicScintillator, *logicAtmosphere[10], *logicMirror;      // Up to 10 layers of atmosphere
        G4VPhysicalVolume *physWorld, *physRadiator, *physDetector, *physScintillator, *physAtmosphere[10], *physMirror;
        G4Material *SiO2, *H2O, *Aerogel, *worldMat, *NaI, *Air[10];        // Air not constant in atmosphere layers
        G4Element *C, *Na, *I, *N, *O;

        void DefineMaterial();

        void ConstructCherenkov();
        void ConstructScintillator();
        void ConstructTOF();
        void ConstructAtmosphere();

        virtual void ConstructSDandField();

        G4GenericMessenger *fMessenger;

        G4LogicalVolume *fScoringVolume;

        G4int nCols, nRows;

        G4double xWorld, yWorld, zWorld;  
        
        G4bool isCherenkov, isScintillator, isTOF, isAtmosphere;

        G4OpticalSurface *mirrorSurface;

        G4LogicalSkinSurface *skin;

        G4double radiatorThickness;
};

#endif