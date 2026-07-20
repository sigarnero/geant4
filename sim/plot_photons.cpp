#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TPaveText.h"
#include <iostream>

void plot_photons(const char* filename = "barraDrittaSiPMgrande.root") {
    TFile *f = TFile::Open(filename);
    if (!f || f->IsZombie()) { std::cerr << "Error: cannot open file" << std::endl; return; }

    TTree *photons = (TTree*)f->Get("Photons");
    if (!photons) { std::cerr << "Error: cannot find Photons tree" << std::endl; return; }

    TTree *hits = (TTree*)f->Get("Hits");
    if (!hits) { std::cerr << "Error: cannot find Hits tree" << std::endl; return; }

    Double_t fX, fY, fZ, fWavelength, fTime;
    Double_t fX_photon, fY_photon, fZ_photon, fWavelength_hits, fTime_hits;
    Int_t fEvent, fDetectorID, fEvent_hits, fDetID_hits;

    photons->SetBranchAddress("fEvent",      &fEvent);
    photons->SetBranchAddress("fDetectorID", &fDetectorID);
    photons->SetBranchAddress("fX",          &fX);
    photons->SetBranchAddress("fY",          &fY);
    photons->SetBranchAddress("fZ",          &fZ);
    photons->SetBranchAddress("fWavelength", &fWavelength);
    photons->SetBranchAddress("fTime",       &fTime);

    hits->SetBranchAddress("fEvent",      &fEvent_hits);
    hits->SetBranchAddress("fDetectorID", &fDetID_hits);
    hits->SetBranchAddress("fX_photon",          &fX_photon);
    hits->SetBranchAddress("fY_photon",          &fY_photon);
    hits->SetBranchAddress("fZ_photon",          &fZ_photon);
    hits->SetBranchAddress("fWavelength", &fWavelength_hits);
    hits->SetBranchAddress("fTime",       &fTime_hits);

    TGraph *g_det1 = new TGraph();
    TGraph *g_det2 = new TGraph();
    TGraph *g_det3 = new TGraph();
    TGraph *g_det4 = new TGraph();
    int n1 = 0, n2 = 0, n3 = 0, n4 = 0;

    for (Long64_t i = 0; i < photons->GetEntries(); i++) {
        photons->GetEntry(i);
        if      (fDetectorID == 1) g_det1->SetPoint(n1++, fZ, fY);
        else if (fDetectorID == 2) g_det2->SetPoint(n2++, fZ, fY);
    }

    for (Long64_t i = 0; i < hits->GetEntries(); i++) {
        hits->GetEntry(i);
        if      (fDetID_hits == 1) g_det3->SetPoint(n3++, fZ_photon, fY_photon);
        else if (fDetID_hits == 2) g_det4->SetPoint(n4++, fZ_photon, fY_photon);
    }

    // ── MARKER STYLE ─────────────────────────────────────────────────────────
    // Size:  0.1–0.5 for dense clouds, 1.0–2.0 for sparse data
    // Style: 6 = pixel dot, 20 = filled circle, 21 = filled square, 34 = filled cross
    // Color: kBlue, kRed, kBlack, kGreen+2, or any ROOT color index
    float markerSize  = 0.5;   // <── change this
    int   markerStyle = 8;     // <── change this
    // ─────────────────────────────────────────────────────────────────────────

    g_det1->SetMarkerSize(markerSize);
    g_det1->SetMarkerStyle(markerStyle);
    g_det1->SetMarkerColor(kBlue);
    g_det1->SetTitle("SiPM 1 arriving photons;Z [mm];Y [mm]");

    g_det2->SetMarkerSize(markerSize);
    g_det2->SetMarkerStyle(markerStyle);
    g_det2->SetMarkerColor(kRed);
    g_det2->SetTitle("SiPM 2 arriving photons;Z [mm];Y [mm]");

    g_det3->SetMarkerSize(markerSize);
    g_det3->SetMarkerStyle(markerStyle);
    g_det3->SetMarkerColor(kBlue);
    g_det3->SetTitle("SiPM 1 detected photons;Z [mm];Y [mm]");

    g_det4->SetMarkerSize(markerSize);
    g_det4->SetMarkerStyle(markerStyle);
    g_det4->SetMarkerColor(kRed);
    g_det4->SetTitle("SiPM 2 detected photons;Z [mm];Y [mm]");

    TCanvas *c1 = new TCanvas("c1", "Photon Detector Hits", 1200, 600);
    c1->Divide(2, 2);

    c1->cd(1);
    gPad->SetRightMargin(0.05);
    gPad->SetTopMargin(0.1);
    g_det1->Draw("AP");

    // Legend for pad 1
    TPaveText *pt1 = new TPaveText(0.15, 0.85, 0.45, 0.92, "NDC");
    pt1->SetFillColor(0);
    pt1->SetBorderSize(1);
    pt1->AddText(Form("Entries: %d", n1));
    pt1->Draw();

    c1->cd(2);
    gPad->SetRightMargin(0.05);
    gPad->SetTopMargin(0.1);
    g_det2->Draw("AP");

    // Legend for pad 2
    TPaveText *pt2 = new TPaveText(0.15, 0.85, 0.45, 0.92, "NDC");
    pt2->SetFillColor(0);
    pt2->SetBorderSize(1);
    pt2->AddText(Form("Entries: %d", n2));
    pt2->Draw();

    c1->cd(3);
    gPad->SetRightMargin(0.05);
    gPad->SetTopMargin(0.1);
    g_det3->Draw("AP");

    // Legend for pad 3
    TPaveText *pt3 = new TPaveText(0.15, 0.85, 0.45, 0.92, "NDC");
    pt3->SetFillColor(0);
    pt3->SetBorderSize(1);
    pt3->AddText(Form("Entries: %d", n3));
    pt3->Draw();

    c1->cd(4);
    gPad->SetRightMargin(0.05);
    gPad->SetTopMargin(0.1);
    g_det4->Draw("AP");

    // Legend for pad 4
    TPaveText *pt4 = new TPaveText(0.15, 0.85, 0.45, 0.92, "NDC");
    pt4->SetFillColor(0);
    pt4->SetBorderSize(1);
    pt4->AddText(Form("Entries: %d", n4));
    pt4->Draw();

    c1->Update();
    c1->SaveAs("PhotonDistribution_singleProton.png");

    std::cout << "SiPM 1 arriving photons: " << n1 << std::endl;
    std::cout << "SiPM 2 arriving photons: " << n2 << std::endl;
    std::cout << "SiPM 1 detected photons: " << n3 << std::endl;
    std::cout << "SiPM 2 detected photons: " << n4 << std::endl;

    f->Close();
}