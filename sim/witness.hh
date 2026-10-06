#ifndef WITNESS_HH
#define WITNESS_HH

#include "G4VSensitiveDetector.hh"
#include "G4OpticalPhoton.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Step.hh"

class MyWitnessSD : public G4VSensitiveDetector
{
public:
    MyWitnessSD(G4String name);
    virtual ~MyWitnessSD();

    virtual G4bool ProcessHits(G4Step *aStep, G4TouchableHistory *ROHist);
};

#endif