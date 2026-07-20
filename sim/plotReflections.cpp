#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TPaveText.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TLegend.h"
#include <iostream>

// Uso: root -l 'plot_reflections.cpp("build2/output0_t1.root")'
void plot_reflections(const char* filename = "barraVshaped40deg.root") {

    TFile *f = TFile::Open(filename);
    if (!f || f->IsZombie()) { std::cerr << "Error: cannot open file" << std::endl; return; }
 
    // Crea la cartella di output (ricorsivo, non fallisce se esiste già)
    TString outDir = "plotsNew";
    gSystem->mkdir(outDir, kTRUE);
    TString outPath = TString(outDir) + "/";
 
    TTree *pr = (TTree*)f->Get("PhotonReflections");
    if (!pr) { std::cerr << "Error: cannot find PhotonReflections tree" << std::endl; return; }
 
    // ── BINNING ─────────────────────────────────────────────────────────────
    // Adatta questi range ai tuoi dati reali (es. controlla con
    // pr->GetMinimum("fPathLength") / GetMaximum(...) prima di runnare)
    const int    nbinsRefl = 30;
    const double reflMin   = 0.,   reflMax   = 30.;    // fNReflections
    const int    nbinsPath = 50;
    const double pathMin   = 0.,   pathMax   = 300.;   // fPathLength [mm]
    // ─────────────────────────────────────────────────────────────────────────
 
    gStyle->SetOptStat(0);
    gStyle->SetPalette(kBird);
 
    TH2D *hReached = new TH2D("hReached",
        "Fotoni RACCOLTI;N Riflessioni;Path Length [mm]",
        nbinsRefl, reflMin, reflMax, nbinsPath, pathMin, pathMax);
    hReached->SetDirectory(0);   // scollega dal TFile: altrimenti f->Close() lo cancella
 
    TH2D *hLost = new TH2D("hLost",
        "Fotoni PERSI;N Riflessioni;Path Length [mm]",
        nbinsRefl, reflMin, reflMax, nbinsPath, pathMin, pathMax);
    hLost->SetDirectory(0);      // idem

    const int nbinsWL = 50;
    const double wlMin = 100., wlMax = 900.;   // adatta se serve (es. 350-450 per il bar test monocromatico)

    TH1D *hWLReached = new TH1D("hWLReached",
        "Lunghezza d'onda fotoni RACCOLTI;#lambda [nm];Fotoni",
        nbinsWL, wlMin, wlMax);
    hWLReached->SetDirectory(0);

    TH1D *hWLLost = new TH1D("hWLLost",
        "Lunghezza d'onda fotoni PERSI;#lambda [nm];Fotoni",
        nbinsWL, wlMin, wlMax);
    hWLLost->SetDirectory(0);

    TH2D *hWLvsReflReached = new TH2D("hWLvsReflReached",
        "Fotoni RACCOLTI;N Riflessioni;#lambda [nm]",
        nbinsRefl, reflMin, reflMax, nbinsWL, wlMin, wlMax);
    hWLvsReflReached->SetDirectory(0);

    TH2D *hWLvsReflLost = new TH2D("hWLvsReflLost",
        "Fotoni PERSI;N Riflessioni;#lambda [nm]",
        nbinsRefl, reflMin, reflMax, nbinsWL, wlMin, wlMax);
    hWLvsReflLost->SetDirectory(0);
 
    Int_t fEvent, fTrackID, fNReflections, fDetectorID, fReachedEnd;
    Double_t fPathLength, fWavelength;
 
    pr->SetBranchAddress("fEvent",         &fEvent);
    pr->SetBranchAddress("fTrackID",       &fTrackID);
    pr->SetBranchAddress("fNReflections",  &fNReflections);
    pr->SetBranchAddress("fPathLength",    &fPathLength);
    pr->SetBranchAddress("fDetectorID",    &fDetectorID);
    pr->SetBranchAddress("fWavelength",    &fWavelength);
    pr->SetBranchAddress("fReachedEnd",    &fReachedEnd);
 
    Long64_t nReached = 0, nLost = 0;
 
    for (Long64_t i = 0; i < pr->GetEntries(); i++) {
        pr->GetEntry(i);
        if (fReachedEnd == 1) {
            hReached->Fill(fNReflections, fPathLength);
            hWLReached->Fill(fWavelength);
            hWLvsReflReached->Fill(fNReflections, fWavelength);
            nReached++;
        } else {
            hLost->Fill(fNReflections, fPathLength);
            hWLLost->Fill(fWavelength);
            hWLvsReflLost->Fill(fNReflections, fWavelength);
            nLost++;
        }
    }
 
    TCanvas *c1 = new TCanvas("c1", "PathLength vs NReflections", 1400, 650);
    c1->Divide(2, 1);
 
    c1->cd(1);
    gPad->SetRightMargin(0.15);
    hReached->Draw("COLZ");
    TPaveText *pt1 = new TPaveText(0.35, 0.85, 0.88, 0.92, "NDC");
    pt1->SetFillColor(0);
    pt1->SetBorderSize(1);
    pt1->AddText(Form("Entries: %lld", nReached));
    pt1->Draw();
 
    c1->cd(2);
    gPad->SetRightMargin(0.15);
    hLost->Draw("COLZ");
    TPaveText *pt2 = new TPaveText(0.35, 0.85, 0.88, 0.92, "NDC");
    pt2->SetFillColor(0);
    pt2->SetBorderSize(1);
    pt2->AddText(Form("Entries: %lld", nLost));
    pt2->Draw();
 
    c1->Update();
    c1->SaveAs(outPath + "PathLength_vs_NReflections.png");
 
    // ── Efficienza di raccolta ──────────────────────────────────────────────
    Long64_t nTotal = nReached + nLost;
    if (nTotal > 0) {
        double eff = double(nReached) / double(nTotal);
        double effErr = std::sqrt(eff * (1 - eff) / nTotal);  // errore binomiale
        std::cout << "\n=== Efficienza di raccolta ===" << std::endl;
        std::cout << "Reached: " << nReached << " / " << nTotal
                   << "  =  " << eff << " +/- " << effErr << std::endl;
        std::cout << "Lost:    " << nLost << std::endl;
    }
 
    // ── CANVAS 2: fotoni Cherenkov generati per evento ──────────────────────
    // Ntuple "PhotonCount" (indice 4), colonna "nPhotons" -> numero di fotoni
    // Cherenkov generati in ciascun evento (prima di qualunque trasporto/perdita)
    TTree *pc = (TTree*)f->Get("PhotonCount");
    if (!pc) {
        std::cerr << "Warning: cannot find PhotonCount tree, skipping canvas 2" << std::endl;
    } else {
        Int_t nPhotons;
        pc->SetBranchAddress("nPhotons", &nPhotons);
 
        Long64_t nEvt = pc->GetEntries();
        Int_t maxPhotons = pc->GetMaximum("nPhotons");
 
        TH1D *hGenerated = new TH1D("hGenerated",
            "Fotoni Cherenkov generati per evento;N fotoni generati;Eventi",
            60, 0, maxPhotons * 1.1 + 1);
        hGenerated->SetDirectory(0);
 
        for (Long64_t i = 0; i < nEvt; i++) {
            pc->GetEntry(i);
            hGenerated->Fill(nPhotons);
        }
 
        TCanvas *c2 = new TCanvas("c2", "Fotoni generati per evento", 900, 650);
        gPad->SetLeftMargin(0.12);
        // gPad->SetGrid(1, 0);
 
        hGenerated->SetFillColorAlpha(kAzure+1, 0.65);
        hGenerated->SetLineColor(kAzure+2);
        hGenerated->SetLineWidth(2);
        hGenerated->SetTitleFont(42);
        hGenerated->GetXaxis()->SetTitleFont(42);
        hGenerated->GetYaxis()->SetTitleFont(42);
        hGenerated->GetXaxis()->CenterTitle();
        hGenerated->GetYaxis()->CenterTitle();
        hGenerated->Draw("HIST");
 
        TPaveText *ptGen = new TPaveText(0.62, 0.78, 0.88, 0.88, "NDC");
        ptGen->SetFillColor(0);
        ptGen->SetBorderSize(1);
        ptGen->AddText(Form("Eventi: %lld", nEvt));
        ptGen->AddText(Form("Media: %.1f", hGenerated->GetMean()));
        ptGen->Draw();
 
        c2->Update();
        c2->SaveAs(outPath + "GeneratedPhotons.png");
    }
 
    // ── CANVAS 3: N riflessioni, reached vs lost, sovrapposti ───────────────
    // Proiezione sull'asse X (fNReflections) dei TH2D già riempiti sopra,
    // così non serve rileggere la TTree una seconda volta
    TH1D *hReflReached = (TH1D*)hReached->ProjectionX("hReflReached");
    TH1D *hReflLost    = (TH1D*)hLost->ProjectionX("hReflLost");
    hReflReached->SetDirectory(0);
    hReflLost->SetDirectory(0);
 
    TCanvas *c3 = new TCanvas("c3", "N Riflessioni: raccolti vs persi", 900, 650);
    gPad->SetLeftMargin(0.12);
    // gPad->SetGrid(1, 0);
 
    hReflReached->SetTitle("Numero di riflessioni;N Riflessioni;Fotoni");
    hReflReached->SetLineColor(kOrange+7);
    hReflReached->SetLineWidth(2);
    hReflReached->SetFillColorAlpha(kOrange+7, 0.35);
    hReflReached->SetFillStyle(3004);
    hReflReached->GetXaxis()->CenterTitle();
    hReflReached->GetYaxis()->CenterTitle();
 
    hReflLost->SetLineColor(kAzure+2);
    hReflLost->SetLineWidth(2);
    hReflLost->SetFillColorAlpha(kAzure+2, 0.35);
    hReflLost->SetFillStyle(3005);
 
    // Il massimo dei due determina la scala verticale, altrimenti uno dei due
    // può uscire dal frame se disegnato per secondo
    double ymax = std::max(hReflReached->GetMaximum(), hReflLost->GetMaximum());
    hReflReached->SetMaximum(ymax * 1.2);
 
    hReflReached->Draw("HIST");
    hReflLost->Draw("HIST SAME");
 
    TLegend *leg = new TLegend(0.55, 0.72, 0.88, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->AddEntry(hReflReached, Form("Raggiungono il rivelatore (%lld)", nReached), "f");
    leg->AddEntry(hReflLost,    Form("Persi (%lld)", nLost), "f");
    leg->Draw();
 
    c3->Update();
    c3->SaveAs(outPath + "Reflections_ReachedVsLost.png");

    // ── CANVAS 4: LUNGHEZZA D'ONDA, RACCOLTI VS PERSI ───────────────
    TCanvas *c4 = new TCanvas("c4", "Lunghezza d'onda: raccolti vs persi", 900, 650);
    gPad->SetLeftMargin(0.12);

    hWLReached->SetLineColor(kOrange+7);
    hWLReached->SetLineWidth(2);
    hWLReached->SetFillColorAlpha(kOrange+7, 0.35);
    hWLReached->SetFillStyle(3004);
    hWLReached->GetXaxis()->CenterTitle();
    hWLReached->GetYaxis()->CenterTitle();

    hWLLost->SetLineColor(kAzure+2);
    hWLLost->SetLineWidth(2);
    hWLLost->SetFillColorAlpha(kAzure+2, 0.35);
    hWLLost->SetFillStyle(3005);

    double ymaxWL = std::max(hWLReached->GetMaximum(), hWLLost->GetMaximum());
    hWLReached->SetMaximum(ymaxWL * 1.2);

    hWLReached->Draw("HIST");
    hWLLost->Draw("HIST SAME");

    TLegend *legWL = new TLegend(0.55, 0.72, 0.88, 0.88);
    legWL->SetBorderSize(0);
    legWL->SetFillStyle(0);
    legWL->AddEntry(hWLReached, Form("Raggiungono il rivelatore (%lld)", nReached), "f");
    legWL->AddEntry(hWLLost,    Form("Persi (%lld)", nLost), "f");
    legWL->Draw();

    c4->Update();
    c4->SaveAs(outPath + "Wavelength_ReachedVsLost.png");

    // ── CANVAS 5: LUNGHEZZA D'ONDA VS N RIFLESSIONI ───────────────
    TCanvas *c5 = new TCanvas("c5", "Lunghezza d'onda vs N Riflessioni", 1400, 650);
    c5->Divide(2, 1);

    c5->cd(1);
    gPad->SetRightMargin(0.15);
    hWLvsReflReached->Draw("COLZ");
    TPaveText *pt5a = new TPaveText(0.35, 0.85, 0.88, 0.92, "NDC");
    pt5a->SetFillColor(0);
    pt5a->SetBorderSize(1);
    pt5a->AddText(Form("Entries: %lld", nReached));
    pt5a->Draw();

    c5->cd(2);
    gPad->SetRightMargin(0.15);
    hWLvsReflLost->Draw("COLZ");
    TPaveText *pt5b = new TPaveText(0.35, 0.85, 0.88, 0.92, "NDC");
    pt5b->SetFillColor(0);
    pt5b->SetBorderSize(1);
    pt5b->AddText(Form("Entries: %lld", nLost));
    pt5b->Draw();

    c5->Update();
    c5->SaveAs(outPath + "Wavelength_vs_NReflections.png");
 
    f->Close();
}