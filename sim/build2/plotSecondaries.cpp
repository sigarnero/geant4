#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TString.h"
#include <iostream>
#include <map>
#include <set>
#include <cmath>

// Uso: root -l 'plotSecondaries.cpp("build2/output0_t1.root")'
//
// Analizza la ntuple "Secondaries" prodotta da MySecondariesSD: particelle
// (principalmente secondarie prodotte nel polietilene) che attraversano
// i due pannelli testimone 5x5 cm posti a monte (-Z) dei SiPM, a X≈±150mm.
//
// NOTA: il volume testimone è aria (non perturbativo): fEdep sarà quasi
// sempre ≈ 0. Le colonne informative per lo studio di radiation hardness
// sono fPDG (composizione del flusso) e fKinE (spettro energetico).

// Mappa PDG -> etichetta leggibile, per le particelle più comuni prodotte
// da interazioni adroniche. Tutto il resto finisce in "Other".
TString PdgLabel(Int_t pdg) {
    switch (pdg) {
        case 2112:  return "neutron";
        case 2212:  return "proton";
        case 22:    return "gamma";
        case -22:   return "opticalphoton";
        case 11:    return "e-";
        case -11:   return "e+";
        case 13:    return "mu-";
        case -13:   return "mu+";
        case 211:   return "pi+";
        case -211:  return "pi-";
        case 111:   return "pi0";
        case 321:   return "K+";
        case -321:  return "K-";
        case 2000010020: return "deuteron";      // occasionalmente usato da G4
        default:
            if (pdg > 1000000000) return "ion";  // codici nucleari G4 (10ZZZAAAI)
            return "Other";
    }
}

void plotSecondaries(const char* filename = "alluminiumLayer0.5cmAllSecondaries.root") {

    TFile *f = TFile::Open(filename);
    if (!f || f->IsZombie()) { std::cerr << "Error: cannot open file" << std::endl; return; }

    TString outDir = "plotsAllSecondariesAluminium0.5cm";
    gSystem->mkdir(outDir, kTRUE);
    TString outPath = TString(outDir) + "/";

    TTree *wh = (TTree*)f->Get("WitnessHits");
    if (!wh) { std::cerr << "Error: cannot find WitnessHits tree" << std::endl; return; }

    gStyle->SetOptStat(0);
    gStyle->SetPalette(kBird);

    Int_t    fEvent, fSide, fTrackID, fParentID, fPDG;
    Double_t fKinE, fEdep, fX, fY, fZ, fTime, fAngleFromAxis;

    wh->SetBranchAddress("fEvent",    &fEvent);
    wh->SetBranchAddress("fSide",     &fSide);
    wh->SetBranchAddress("fTrackID",  &fTrackID);
    wh->SetBranchAddress("fParentID", &fParentID);
    wh->SetBranchAddress("fPDG",      &fPDG);
    wh->SetBranchAddress("fKinE",     &fKinE);
    wh->SetBranchAddress("fEdep",     &fEdep);
    wh->SetBranchAddress("fX",        &fX);
    wh->SetBranchAddress("fY",        &fY);
    wh->SetBranchAddress("fZ",        &fZ);
    wh->SetBranchAddress("fTime",     &fTime);
    wh->SetBranchAddress("fAngleFromAxis", &fAngleFromAxis);

    Long64_t nEntries = wh->GetEntries();
    if (nEntries == 0) {
        std::cout << "WitnessHits tree is empty: nessuna particella ha raggiunto i pannelli in questo run." << std::endl;
        f->Close();
        return;
    }

    // ── BINNING ─────────────────────────────────────────────────────────────
    // Energia cinetica: range log-friendly. Adatta se vedi hit sottodimensionati
    // o code oltre 1 GeV (protoni primari deviati, raro ma possibile).
    const int    nbinsE = 60;
    const double eMin   = 0.01,  eMax = 500.;    // MeV, scala log
    const int    nbinsT = 60;
    const double tMin   = 0.,    tMax = 30.;     // ns
    const int    nbinsXY = 50;
    // ─────────────────────────────────────────────────────────────────────────

    // Istogramma composizione PDG: bin per specie note + "Other", per lato
    std::vector<TString> species = {"neutron","proton","gamma","e-","e+","mu-","mu+","pi+","pi-","pi0","K+","K-","deuteron","ion","Other"};
    std::map<TString,int> speciesIndex;
    for (size_t i = 0; i < species.size(); i++) speciesIndex[species[i]] = i;

    TH1D *hPdgSide0 = new TH1D("hPdgSide0", "Composizione del flusso;;Particelle", species.size(), 0, species.size());
    TH1D *hPdgSide1 = new TH1D("hPdgSide1", "", species.size(), 0, species.size());
    hPdgSide0->SetDirectory(0);
    hPdgSide1->SetDirectory(0);
    for (size_t i = 0; i < species.size(); i++) {
        hPdgSide0->GetXaxis()->SetBinLabel(i+1, species[i]);
    }

    TH1D *hKinESide0 = new TH1D("hKinESide0", "Spettro energia cinetica;E_{kin} [MeV];Particelle", nbinsE, std::log10(eMin), std::log10(eMax));
    TH1D *hKinESide1 = new TH1D("hKinESide1", "", nbinsE, std::log10(eMin), std::log10(eMax));
    hKinESide0->SetDirectory(0);
    hKinESide1->SetDirectory(0);

    TH2D *hMapSide0 = new TH2D("hMapSide0", "Mappa hit pannello lato +X;X [mm];Y [mm]", nbinsXY, 125., 175., nbinsXY, -25., 25.);
    TH2D *hMapSide1 = new TH2D("hMapSide1", "Mappa hit pannello lato -X;X [mm];Y [mm]", nbinsXY, -175., -125., nbinsXY, -25., 25.);
    hMapSide0->SetDirectory(0);
    hMapSide1->SetDirectory(0);

    // Pannello aggiuntivo dopo lo strato di Al, sull'asse del fascio (side==2)
    TH1D *hPdgSideAl = new TH1D("hPdgSideAl", "Composizione del flusso (dopo Al);;Particelle", species.size(), 0, species.size());
    hPdgSideAl->SetDirectory(0);
    for (size_t i = 0; i < species.size(); i++) hPdgSideAl->GetXaxis()->SetBinLabel(i+1, species[i]);

    // Range centrato su 0: il pannello è sull'asse, non a ±150mm come gli altri
    TH2D *hMapSideAl = new TH2D("hMapSideAl", "Mappa hit pannello dopo Al;X [mm];Y [mm]", nbinsXY, -30., 30., nbinsXY, -30., 30.);
    hMapSideAl->SetDirectory(0);

        // Angolo di emissione rispetto all'asse del fascio (Z), solo pannello dopo Al
    TH1D *hAngleSideAl = new TH1D("hAngleSideAl",
        "Angolo di emissione rispetto all'asse del fascio;#theta [deg];Particelle",
        90, 0., 180.);
    hAngleSideAl->SetDirectory(0);

    TH1D *hTimeSide0 = new TH1D("hTimeSide0", "Tempo di arrivo;Time [ns];Particelle", nbinsT, tMin, tMax);
    TH1D *hTimeSide1 = new TH1D("hTimeSide1", "", nbinsT, tMin, tMax);
    hTimeSide0->SetDirectory(0);
    hTimeSide1->SetDirectory(0);

    // Molteplicità per evento (hit grezzi sul pannello, per lato) e conteggio
    // primarie vs secondarie
    std::map<Int_t,int> multSide0, multSide1;
    Long64_t nPrimary = 0, nSecondary = 0;
    Long64_t nSide0 = 0, nSide1 = 0, nSideAl = 0, nPrimaryExcludedAl = 0;
    double sumKinESide0 = 0., sumKinESide1 = 0.;

    for (Long64_t i = 0; i < nEntries; i++) {
        wh->GetEntry(i);

        TString label = PdgLabel(fPDG);
        int idx = speciesIndex.count(label) ? speciesIndex[label] : speciesIndex["Other"];
        double logE = std::log10(std::max(fKinE, 1e-6));

        if (fSide == 0) {
            hPdgSide0->Fill(idx + 0.5);
            hKinESide0->Fill(logE);
            hMapSide0->Fill(fX, fY);
            hTimeSide0->Fill(fTime);
            multSide0[fEvent]++;
            nSide0++;
            sumKinESide0 += fKinE;
        } else if (fSide == 1) {
            hPdgSide1->Fill(idx + 0.5);
            hKinESide1->Fill(logE);
            hMapSide1->Fill(fX, fY);
            hTimeSide1->Fill(fTime);
            multSide1[fEvent]++;
            nSide1++;
            sumKinESide1 += fKinE;
        } else if (fSide == 2) {
            // Esclude il protone primario non interagente (parentID==0):
            // vogliamo solo le secondarie generate nello strato di Al
            if (fParentID == 0) { nPrimaryExcludedAl++; continue; }
            hPdgSideAl->Fill(idx + 0.5);
            hMapSideAl->Fill(fX, fY);
            hAngleSideAl->Fill(fAngleFromAxis);
            nSideAl++;
        }

        if (fParentID == 0) nPrimary++; else nSecondary++;
    }

    // ── CANVAS 1: Composizione del flusso per specie ─────────────────────────
    TCanvas *c1 = new TCanvas("c1", "Composizione del flusso", 900, 650);
    gPad->SetLeftMargin(0.12);
    gPad->SetBottomMargin(0.12);

    hPdgSide0->SetLineColor(kAzure+2);
    hPdgSide0->SetFillColorAlpha(kAzure+1, 0.5);
    hPdgSide0->GetXaxis()->SetLabelSize(0.045);
    hPdgSide0->GetYaxis()->CenterTitle();
    hPdgSide0->SetStats(0);

    hPdgSide1->SetLineColor(kOrange+7);
    hPdgSide1->SetFillColorAlpha(kOrange+7, 0.5);

    double ymaxPdg = std::max(hPdgSide0->GetMaximum(), hPdgSide1->GetMaximum());
    hPdgSide0->SetMaximum(ymaxPdg * 1.3 + 1);

    hPdgSide0->Draw("HIST");
    hPdgSide1->Draw("HIST SAME");

    TLegend *leg1 = new TLegend(0.6, 0.75, 0.88, 0.88);
    leg1->SetBorderSize(0);
    leg1->SetFillStyle(0);
    leg1->AddEntry(hPdgSide0, Form("Lato +X (%lld hit)", nSide0), "f");
    leg1->AddEntry(hPdgSide1, Form("Lato -X (%lld hit)", nSide1), "f");
    leg1->Draw();

    c1->Update();
    c1->SaveAs(outPath + "PDG_Composition.png");

    // ── CANVAS 2: Spettro energia cinetica (log X, log Y) ────────────────────
    TCanvas *c2 = new TCanvas("c2", "Spettro energia cinetica", 900, 650);
    gPad->SetLeftMargin(0.12);
    gPad->SetLogy();

    hKinESide0->SetLineColor(kAzure+2);
    hKinESide0->SetLineWidth(2);
    hKinESide0->GetXaxis()->CenterTitle();
    hKinESide0->GetYaxis()->CenterTitle();

    hKinESide1->SetLineColor(kOrange+7);
    hKinESide1->SetLineWidth(2);

    // Bin in log10(E): rietichetta l'asse in MeV per leggibilità
    hKinESide0->GetXaxis()->SetTitle("log_{10}(E_{kin} / MeV)");

    double ymaxE = std::max(hKinESide0->GetMaximum(), hKinESide1->GetMaximum());
    hKinESide0->SetMaximum(ymaxE * 3.);
    hKinESide0->SetMinimum(0.5);

    hKinESide0->Draw("HIST");
    hKinESide1->Draw("HIST SAME");

    TLegend *leg2 = new TLegend(0.6, 0.75, 0.88, 0.88);
    leg2->SetBorderSize(0);
    leg2->SetFillStyle(0);
    leg2->AddEntry(hKinESide0, Form("Lato +X, <E> = %.2f MeV", nSide0 ? sumKinESide0/nSide0 : 0.), "l");
    leg2->AddEntry(hKinESide1, Form("Lato -X, <E> = %.2f MeV", nSide1 ? sumKinESide1/nSide1 : 0.), "l");
    leg2->Draw();

    c2->Update();
    c2->SaveAs(outPath + "KineticEnergy_Spectrum.png");

    // ── CANVAS 3: Mappa spaziale hit sui due pannelli ────────────────────────
    TCanvas *c3 = new TCanvas("c3", "Mappa hit pannelli", 1400, 650);
    c3->Divide(2, 1);

    c3->cd(1);
    gPad->SetRightMargin(0.15);
    hMapSide0->Draw("COLZ");
    TPaveText *pt3a = new TPaveText(0.35, 0.85, 0.88, 0.92, "NDC");
    pt3a->SetFillColor(0);
    pt3a->SetBorderSize(1);
    pt3a->AddText(Form("Entries: %lld", nSide0));
    pt3a->Draw();

    c3->cd(2);
    gPad->SetRightMargin(0.15);
    hMapSide1->Draw("COLZ");
    TPaveText *pt3b = new TPaveText(0.35, 0.85, 0.88, 0.92, "NDC");
    pt3b->SetFillColor(0);
    pt3b->SetBorderSize(1);
    pt3b->AddText(Form("Entries: %lld", nSide1));
    pt3b->Draw();

    c3->Update();
    c3->SaveAs(outPath + "HitMap_Panels.png");

    // ── CANVAS 4: Distribuzione temporale di arrivo ──────────────────────────
    TCanvas *c4 = new TCanvas("c4", "Tempo di arrivo sui pannelli", 900, 650);
    gPad->SetLeftMargin(0.12);

    hTimeSide0->SetLineColor(kAzure+2);
    hTimeSide0->SetFillColorAlpha(kAzure+1, 0.35);
    hTimeSide0->SetFillStyle(3004);
    hTimeSide0->GetXaxis()->CenterTitle();
    hTimeSide0->GetYaxis()->CenterTitle();

    hTimeSide1->SetLineColor(kOrange+7);
    hTimeSide1->SetFillColorAlpha(kOrange+7, 0.35);
    hTimeSide1->SetFillStyle(3005);

    double ymaxT = std::max(hTimeSide0->GetMaximum(), hTimeSide1->GetMaximum());
    hTimeSide0->SetMaximum(ymaxT * 1.2 + 1);

    hTimeSide0->Draw("HIST");
    hTimeSide1->Draw("HIST SAME");

    TLegend *leg4 = new TLegend(0.55, 0.72, 0.88, 0.88);
    leg4->SetBorderSize(0);
    leg4->SetFillStyle(0);
    leg4->AddEntry(hTimeSide0, "Lato +X", "f");
    leg4->AddEntry(hTimeSide1, "Lato -X", "f");
    leg4->Draw();

    c4->Update();
    c4->SaveAs(outPath + "ArrivalTime.png");

    // ── CANVAS 5: Molteplicità per evento (per lato) ─────────────────────────
    Int_t maxMult0 = 0, maxMult1 = 0;
    for (auto &kv : multSide0) maxMult0 = std::max(maxMult0, kv.second);
    for (auto &kv : multSide1) maxMult1 = std::max(maxMult1, kv.second);
    Int_t nbinsMult = std::max(maxMult0, maxMult1) + 2;

    TH1D *hMultSide0 = new TH1D("hMultSide0", "Hit per evento sul pannello;N particelle;Eventi", nbinsMult, 0, nbinsMult);
    TH1D *hMultSide1 = new TH1D("hMultSide1", "", nbinsMult, 0, nbinsMult);
    hMultSide0->SetDirectory(0);
    hMultSide1->SetDirectory(0);
    for (auto &kv : multSide0) hMultSide0->Fill(kv.second);
    for (auto &kv : multSide1) hMultSide1->Fill(kv.second);

    TCanvas *c5 = new TCanvas("c5", "Molteplicita per evento", 900, 650);
    gPad->SetLeftMargin(0.12);

    hMultSide0->SetLineColor(kAzure+2);
    hMultSide0->SetFillColorAlpha(kAzure+1, 0.35);
    hMultSide0->SetFillStyle(3004);
    hMultSide0->GetXaxis()->CenterTitle();
    hMultSide0->GetYaxis()->CenterTitle();

    hMultSide1->SetLineColor(kOrange+7);
    hMultSide1->SetFillColorAlpha(kOrange+7, 0.35);
    hMultSide1->SetFillStyle(3005);

    double ymaxM = std::max(hMultSide0->GetMaximum(), hMultSide1->GetMaximum());
    hMultSide0->SetMaximum(ymaxM * 1.2 + 1);

    hMultSide0->Draw("HIST");
    hMultSide1->Draw("HIST SAME");

    TLegend *leg5 = new TLegend(0.55, 0.68, 0.88, 0.88);
    leg5->SetBorderSize(0);
    leg5->SetFillStyle(0);
    leg5->AddEntry(hMultSide0, Form("Lato +X: #mu = %.3f", hMultSide0->GetMean()), "f");
    leg5->AddEntry(hMultSide1, Form("Lato -X: #mu = %.3f", hMultSide1->GetMean()), "f");
    leg5->Draw();

    c5->Update();
    c5->SaveAs(outPath + "Multiplicity_PerEvent.png");

    // ── CANVAS Al-1: Composizione del flusso dopo lo strato di Al ──────────
    TCanvas *cAl1 = new TCanvas("cAl1", "Composizione flusso dopo Al", 900, 650);
    gPad->SetLeftMargin(0.12);
    gPad->SetBottomMargin(0.12);
    hPdgSideAl->SetLineColor(kGreen+2);
    hPdgSideAl->SetFillColorAlpha(kGreen+1, 0.5);
    hPdgSideAl->GetXaxis()->SetLabelSize(0.045);
    hPdgSideAl->GetYaxis()->CenterTitle();
    hPdgSideAl->SetStats(0);
    hPdgSideAl->Draw("HIST");

    TPaveText *ptAl1 = new TPaveText(0.6, 0.8, 0.88, 0.88, "NDC");
    ptAl1->SetFillColor(0);
    ptAl1->SetBorderSize(1);
    ptAl1->AddText(Form("Secondarie: %lld", nSideAl));
    ptAl1->AddText(Form("Primari esclusi: %lld", nPrimaryExcludedAl));
    ptAl1->Draw();

    cAl1->Update();
    cAl1->SaveAs(outPath + "PDG_Composition_AfterAl.png");

    // ── CANVAS Al-2: Mappa spaziale hit dopo lo strato di Al ────────────────
    TCanvas *cAl2 = new TCanvas("cAl2", "Mappa hit dopo Al", 900, 650);
    gPad->SetRightMargin(0.15);
    hMapSideAl->Draw("COLZ");
    TPaveText *ptAl2 = new TPaveText(0.35, 0.85, 0.88, 0.92, "NDC");
    ptAl2->SetFillColor(0);
    ptAl2->SetBorderSize(1);
    ptAl2->AddText(Form("Entries: %lld", nSideAl));
    ptAl2->Draw();
    cAl2->Update();
    cAl2->SaveAs(outPath + "HitMap_AfterAl.png");

    // ── CANVAS Al-3: Angolo di emissione, pannello dopo Al ─────────────────
    TCanvas *cAl3 = new TCanvas("cAl3", "Angolo di emissione dopo Al", 900, 650);
    gPad->SetLeftMargin(0.12);

    hAngleSideAl->SetLineColor(kGreen+2);
    hAngleSideAl->SetFillColorAlpha(kGreen+1, 0.35);
    hAngleSideAl->SetFillStyle(3004);
    hAngleSideAl->GetXaxis()->CenterTitle();
    hAngleSideAl->GetYaxis()->CenterTitle();
    hAngleSideAl->SetStats(0);
    hAngleSideAl->Draw("HIST");

    TPaveText *ptAl3 = new TPaveText(0.55, 0.75, 0.88, 0.88, "NDC");
    ptAl3->SetFillColor(0);
    ptAl3->SetBorderSize(1);
    ptAl3->AddText(Form("<#theta> = %.1f deg", hAngleSideAl->GetMean()));
    ptAl3->AddText(Form("RMS = %.1f deg", hAngleSideAl->GetRMS()));
    ptAl3->Draw();

    cAl3->Update();
    cAl3->SaveAs(outPath + "EmissionAngle_AfterAl.png");

    // ── Riepilogo testuale ────────────────────────────────────────────────────
    Long64_t nEvt = wh->GetMaximum("fEvent") - wh->GetMinimum("fEvent") + 1;  // stima grezza
    std::cout << "\n=== Riepilogo WitnessHits ===" << std::endl;
    std::cout << "Hit totali:            " << nEntries << std::endl;
    std::cout << "  Lato +X:             " << nSide0 << "  (<E_kin> = " << (nSide0 ? sumKinESide0/nSide0 : 0.) << " MeV)" << std::endl;
    std::cout << "  Lato -X:             " << nSide1 << "  (<E_kin> = " << (nSide1 ? sumKinESide1/nSide1 : 0.) << " MeV)" << std::endl;
    std::cout << "Da particella primaria (parentID==0): " << nPrimary << std::endl;
    std::cout << "Da secondarie (parentID>0):           " << nSecondary << std::endl;
    std::cout << "Molteplicita media lato +X: " << hMultSide0->GetMean() << std::endl;
    std::cout << "Molteplicita media lato -X: " << hMultSide1->GetMean() << std::endl;
    std::cout << "\n=== Riepilogo pannello dopo Al (side==2) ===" << std::endl;
    std::cout << "Secondarie:      " << nSideAl << std::endl;
    std::cout << "Primari esclusi: " << nPrimaryExcludedAl << std::endl;

    f->Close();
}