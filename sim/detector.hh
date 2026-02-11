#ifndef DETECTOR_HH
#define DETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicsOrderedFreeVector.hh"
#include <fstream>
#include <vector>

class MySensitiveDetector : public G4VSensitiveDetector
{
public:
    MySensitiveDetector(G4String name);  // Keep your original constructor
    virtual ~MySensitiveDetector();

    virtual G4bool ProcessHits(G4Step *aStep, G4TouchableHistory *ROHist);
    
    // NEW: Methods for coincidence detection
    const std::vector<G4double>& GetDetector1Times() const { return fDetector1Times; }
    const std::vector<G4double>& GetDetector2Times() const { return fDetector2Times; }
    void ClearEvent() { 
        fDetector1Times.clear(); 
        fDetector2Times.clear();
    }

private:
    G4PhysicsOrderedFreeVector *quEff;
    std::vector<G4double> fDetector1Times;  // NEW: Times for detector at +105mm (copy 0)
    std::vector<G4double> fDetector2Times;  // NEW: Times for detector at -105mm (copy 1)
};

#endif