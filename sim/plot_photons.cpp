#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TPaveText.h"
#include "TBox.h"
#include "TStyle.h"
#include <iostream>

void plot_photons(const char* filename = "barraDrittaSiPMgrande100000.root") {
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

    // ── Geometria del quadrato di conteggio ─────────────────────────────────
    // Area attiva reale del detector (vedi ConstructFusedSilica: detHalfY=detHalfZ=8mm)
    const double squareSide = 16.0;      // mm
    const double squareHalf = squareSide / 2.0;
    const double centerZ = 150.0;        // mm, posizione Z del detector (0.15*m)
    const double centerY = 0.0;          // mm, centrato in Y
    // ─────────────────────────────────────────────────────────────────────────

    TGraph *g_det1 = new TGraph();
    TGraph *g_det2 = new TGraph();
    TGraph *g_det3 = new TGraph();
    TGraph *g_det4 = new TGraph();
    int n1 = 0, n2 = 0, n3 = 0, n4 = 0;
    int nInSquare1 = 0, nInSquare2 = 0, nInSquare3 = 0, nInSquare4 = 0;

    // ── Istogrammi 2D (stesso range dello scatter plot, binnati) ────────────
    const double histHalfRangeY = 12.5;   // mm, un po' oltre il quadrato 16mm per vedere il bordo
    const double histHalfRangeZ = 10.0; 
    const int    nbinsHist = 100;         // 0.5mm/bin su 20mm di range

    TH2D *h_det1 = new TH2D("h_det1", "SiPM 1 arriving photons;Z [mm];Y [mm]",
        nbinsHist, centerZ - histHalfRangeZ, centerZ + histHalfRangeZ,
        nbinsHist, centerY - histHalfRangeY, centerY + histHalfRangeY);
    TH2D *h_det2 = new TH2D("h_det2", "SiPM 2 arriving photons;Z [mm];Y [mm]",
        nbinsHist, centerZ - histHalfRangeZ, centerZ + histHalfRangeZ,
        nbinsHist, centerY - histHalfRangeY, centerY + histHalfRangeY);
    TH2D *h_det3 = new TH2D("h_det3", "SiPM 1 detected photons;Z [mm];Y [mm]",
        nbinsHist, centerZ - histHalfRangeZ, centerZ + histHalfRangeZ,
        nbinsHist, centerY - histHalfRangeY, centerY + histHalfRangeY);
    TH2D *h_det4 = new TH2D("h_det4", "SiPM 2 detected photons;Z [mm];Y [mm]",
        nbinsHist, centerZ - histHalfRangeZ, centerZ + histHalfRangeZ,
        nbinsHist, centerY - histHalfRangeY, centerY + histHalfRangeY);

    h_det1->SetDirectory(0); h_det2->SetDirectory(0);
    h_det3->SetDirectory(0); h_det4->SetDirectory(0);

    for (Long64_t i = 0; i < photons->GetEntries(); i++) {
        photons->GetEntry(i);
        bool inSquare = (std::abs(fZ - centerZ) < squareHalf) && (std::abs(fY - centerY) < squareHalf);
        if (fDetectorID == 1) {
            g_det1->SetPoint(n1++, fZ, fY);
            h_det1->Fill(fZ, fY);
            if (inSquare) nInSquare1++;
        } else if (fDetectorID == 2) {
            g_det2->SetPoint(n2++, fZ, fY);
            h_det2->Fill(fZ, fY);
            if (inSquare) nInSquare2++;
        }
    }

    for (Long64_t i = 0; i < hits->GetEntries(); i++) {
        hits->GetEntry(i);
        bool inSquare = (std::abs(fZ_photon - centerZ) < squareHalf) && (std::abs(fY_photon - centerY) < squareHalf);
        if (fDetID_hits == 1) {
            g_det3->SetPoint(n3++, fZ_photon, fY_photon);
            h_det3->Fill(fZ_photon, fY_photon);
            if (inSquare) nInSquare3++;
        } else if (fDetID_hits == 2) {
            g_det4->SetPoint(n4++, fZ_photon, fY_photon);
            h_det4->Fill(fZ_photon, fY_photon);
            if (inSquare) nInSquare4++;
        }
    }

    // ── MARKER STYLE ─────────────────────────────────────────────────────────
    float markerSize  = 0.5;
    int   markerStyle = 8;
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
    c1->Divide(2, 1);

    // Helper lambda per disegnare il quadrato (stesso su tutti e 4 i pad)
    auto drawSquare = [&]() {
        TBox *box = new TBox(centerZ - squareHalf, centerY - squareHalf,
                              centerZ + squareHalf, centerY + squareHalf);
        box->SetFillStyle(0);        // solo contorno, non riempito
        box->SetLineColor(kBlack);
        box->SetLineWidth(2);
        box->SetLineStyle(2);        // tratteggiato, per distinguerlo dai punti
        box->Draw("SAME L");
    };

    c1->cd(1);
    gPad->SetRightMargin(0.05);
    gPad->SetTopMargin(0.1);
    g_det1->Draw("AP");
    drawSquare();
    TPaveText *pt1 = new TPaveText(0.15, 0.82, 0.45, 0.92, "NDC");
    pt1->SetFillColor(0);
    pt1->SetBorderSize(1);
    pt1->AddText(Form("Entries: %d", n1));
    pt1->AddText(Form("In 16mm square: %d", nInSquare1));
    pt1->Draw();

    c1->cd(2);
    gPad->SetRightMargin(0.05);
    gPad->SetTopMargin(0.1);
    g_det2->Draw("AP");
    drawSquare();
    TPaveText *pt2 = new TPaveText(0.15, 0.82, 0.45, 0.92, "NDC");
    pt2->SetFillColor(0);
    pt2->SetBorderSize(1);
    pt2->AddText(Form("Entries: %d", n2));
    pt2->AddText(Form("In 16mm square: %d", nInSquare2));
    pt2->Draw();

    // c1->cd(3);
    // gPad->SetRightMargin(0.05);
    // gPad->SetTopMargin(0.1);
    // g_det3->Draw("AP");
    // drawSquare();
    // TPaveText *pt3 = new TPaveText(0.15, 0.82, 0.45, 0.92, "NDC");
    // pt3->SetFillColor(0);
    // pt3->SetBorderSize(1);
    // pt3->AddText(Form("Entries: %d", n3));
    // pt3->AddText(Form("In 16mm square: %d", nInSquare3));
    // pt3->Draw();

    // c1->cd(4);
    // gPad->SetRightMargin(0.05);
    // gPad->SetTopMargin(0.1);
    // g_det4->Draw("AP");
    // drawSquare();
    // TPaveText *pt4 = new TPaveText(0.15, 0.82, 0.45, 0.92, "NDC");
    // pt4->SetFillColor(0);
    // pt4->SetBorderSize(1);
    // pt4->AddText(Form("Entries: %d", n4));
    // pt4->AddText(Form("In 16mm square: %d", nInSquare4));
    // pt4->Draw();

    c1->Update();
    // c1->SaveAs("PhotonDistribution_singleProton.png");

    // ── CANVAS 2: Istogrammi 2D binnati (stessa grandezza dello scatter, con densità) ──
    gStyle->SetOptStat(0);
    gStyle->SetPalette(kBird);

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

    // c2->cd(3);
    // gPad->SetRightMargin(0.15);
    // gPad->SetTopMargin(0.1);
    // h_det3->Draw("COLZ");
    // drawSquare();
    // TPaveText *hpt3 = new TPaveText(0.15, 0.82, 0.5, 0.92, "NDC");
    // hpt3->SetFillColor(0);
    // hpt3->SetBorderSize(1);
    // hpt3->AddText(Form("Entries: %d", n3));
    // hpt3->AddText(Form("In 16mm square: %d", nInSquare3));
    // hpt3->Draw();

    // c2->cd(4);
    // gPad->SetRightMargin(0.15);
    // gPad->SetTopMargin(0.1);
    // h_det4->Draw("COLZ");
    // drawSquare();
    // TPaveText *hpt4 = new TPaveText(0.15, 0.82, 0.5, 0.92, "NDC");
    // hpt4->SetFillColor(0);
    // hpt4->SetBorderSize(1);
    // hpt4->AddText(Form("Entries: %d", n4));
    // hpt4->AddText(Form("In 16mm square: %d", nInSquare4));
    // hpt4->Draw();

    c2->Update();
    c2->SaveAs("PhotonDistribution2Dbinned100000.png");

    std::cout << "SiPM 1 arriving photons: " << n1 << " (in square: " << nInSquare1 << ")" << std::endl;
    std::cout << "SiPM 2 arriving photons: " << n2 << " (in square: " << nInSquare2 << ")" << std::endl;
    std::cout << "SiPM 1 detected photons: " << n3 << " (in square: " << nInSquare3 << ")" << std::endl;
    std::cout << "SiPM 2 detected photons: " << n4 << " (in square: " << nInSquare4 << ")" << std::endl;

    f->Close();
    }