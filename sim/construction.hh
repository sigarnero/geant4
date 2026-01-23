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
#include "G4Cons.hh"
#include "G4PVPlacement.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
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

        G4Box *solidWorld, *solidRadiator, *solidDetector, *solidScintillator, *solidAtmosphere, *solidMirror_1, *solidWindow;
        G4Sphere *solidMirror;
        // G4Tubs *solidScintillator;
        G4Cons *solidRadiatorProx;
        G4LogicalVolume *logicWorld, *logicRadiator, *logicDetector, *logicScintillator, *logicAtmosphere[10], *logicMirror, *logicMirror_1, *logicWindow;      // Up to 10 layers of atmosphere
        G4VPhysicalVolume *physWorld, *physRadiator, *physDetector, *physScintillator, *physAtmosphere[10], *physMirror, *physMirror_1, *physWindow;
        G4Material *SiO2, *H2O, *Aerogel, *worldMat, *NaI, *Air[10], *MgF2;        // Air not constant in atmosphere layers
        G4Element *C, *Na, *I, *N, *O, *Mg, *F;

        void DefineMaterial();

        void ConstructCherenkov();
        void ConstructScintillator();
        void ConstructTOF();
        void ConstructAtmosphere();
        void ConstructGasPM();
        void ConstructFusedSilica();
        void ConstructFusedSilicaProx();

        virtual void ConstructSDandField();

        G4GenericMessenger *fMessenger;

        G4LogicalVolume *fScoringVolume;

        G4int nCols, nRows;

        G4double xWorld, yWorld, zWorld;  
        
        G4bool isFusedSilica, isFusedSilicaProx, isCherenkov, isScintillator, isTOF, isAtmosphere, isGasPM;

        G4OpticalSurface *mirrorSurface;

        G4LogicalSkinSurface *skin, *skin_1;

        G4double radiatorThickness;

        G4RotationMatrix *rotX;
};

#endif