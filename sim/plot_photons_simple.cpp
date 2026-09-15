#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TPaveText.h"
#include "TBox.h"
#include "TStyle.h"
#include "TROOT.h"
#include <iostream>
#include <cmath>

void plot_photons(const char* filename = "barraDrittaSiPMgrande100000.root") {

    gROOT->SetBatch(kTRUE);   // niente rendering interattivo, molto più leggero
    gStyle->SetOptStat(0);
    gStyle->SetPalette(kBird);

    TFile *f = TFile::Open(filename, "READ");
    if (!f || f->IsZombie()) { std::cerr << "Error: cannot open file" << std::endl; return; }

    TTree *photons = (TTree*)f->Get("Photons");
    if (!photons) { std::cerr << "Error: cannot find Photons tree" << std::endl; return; }

    // ── Geometria del quadrato di conteggio (area attiva detector, 16mm) ────
    const double squareSide = 16.0;      // mm
    const double squareHalf = squareSide / 2.0;
    const double centerZ = 150.0;        // mm
    const double centerY = 0.0;          // mm
    // ─────────────────────────────────────────────────────────────────────────

    const double histHalfRangeY = 12.5;
    const double histHalfRangeZ = 10.0;
    const int    nbinsHist = 50;

    TH2D *h_det1 = new TH2D("h_det1", "SiPM 1 arriving photons;Z [mm];Y [mm]",
        nbinsHist, centerZ - histHalfRangeZ, centerZ + histHalfRangeZ,
        nbinsHist, centerY - histHalfRangeY, centerY + histHalfRangeY);
    TH2D *h_det2 = new TH2D("h_det2", "SiPM 2 arriving photons;Z [mm];Y [mm]",
        nbinsHist, centerZ - histHalfRangeZ, centerZ + histHalfRangeZ,
        nbinsHist, centerY - histHalfRangeY, centerY + histHalfRangeY);

    h_det1->SetDirectory(0);
    h_det2->SetDirectory(0);

    // ── Lettura minimale: solo fDetectorID, fY, fZ ───────────────────────────
    Double_t fY, fZ;
    Int_t    fDetectorID;

    photons->SetBranchStatus("*", 0);
    photons->SetBranchStatus("fDetectorID", 1);
    photons->SetBranchStatus("fY", 1);
    photons->SetBranchStatus("fZ", 1);

    photons->SetBranchAddress("fDetectorID", &fDetectorID);
    photons->SetBranchAddress("fY", &fY);
    photons->SetBranchAddress("fZ", &fZ);

    int n1 = 0, n2 = 0;
    int nInSquare1 = 0, nInSquare2 = 0;

    Long64_t nEntries = photons->GetEntries();
    for (Long64_t i = 0; i < nEntries; i++) {
        photons->GetEntry(i);

        if (fDetectorID != 1 && fDetectorID != 2) continue;

        bool inSquare = (std::abs(fZ - centerZ) < squareHalf) &&
                         (std::abs(fY - centerY) < squareHalf);

        if (fDetectorID == 1) {
            h_det1->Fill(fZ, fY);
            n1++;
            if (inSquare) nInSquare1++;
        } else {
            h_det2->Fill(fZ, fY);
            n2++;
            if (inSquare) nInSquare2++;
        }
    }

    // ── Disegno ───────────────────────────────────────────────────────────
    auto drawSquare = [&]() {
        TBox *box = new TBox(centerZ - squareHalf, centerY - squareHalf,
                              centerZ + squareHalf, centerY + squareHalf);
        box->SetFillStyle(0);
        box->SetLineColor(kBlack);
        box->SetLineWidth(2);
        box->SetLineStyle(2);
        box->Draw("SAME L");
    };

    TCanvas *c2 = new TCanvas("c2", "Photon Detector Hits (binned)", 1200, 600);
    c2->Divide(2, 1);

    c2->cd(1);
    gPad->SetRightMargin(0.15);
    gPad->SetTopMargin(0.1);
    h_det1->Draw("COLZ");
    drawSquare();
    TPaveText *hpt1 = new TPaveText(0.15, 0.82, 0.5, 0.92, "NDC");
    hpt1->SetFillColor(0);
    hpt1->SetBorderSize(1);
    hpt1->AddText(Form("Entries: %d", n1));
    hpt1->AddText(Form("In 16mm square: %d", nInSquare1));
    hpt1->Draw();

    c2->cd(2);
    gPad->SetRightMargin(0.15);
    gPad->SetTopMargin(0.1);
    h_det2->Draw("COLZ");
    drawSquare();
    TPaveText *hpt2 = new TPaveText(0.15, 0.82, 0.5, 0.92, "NDC");
    hpt2->SetFillColor(0);
    hpt2->SetBorderSize(1);
    hpt2->AddText(Form("Entries: %d", n2));
    hpt2->AddText(Form("In 16mm square: %d", nInSquare2));
    hpt2->Draw();

    c2->Update();
    c2->SaveAs("test.png");

    std::cout << "SiPM 1 arriving photons: " << n1 << " (in square: " << nInSquare1 << ")" << std::endl;
    std::cout << "SiPM 2 arriving photons: " << n2 << " (in square: " << nInSquare2 << ")" << std::endl;

    f->Close();
}