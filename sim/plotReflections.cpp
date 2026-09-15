#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TPaveText.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TLegend.h"
#include "TProfile2D.h"
#include "TLine.h"
#include "TGraph.h"
#include "TMath.h"
#include <iostream>

// Change line 510 for the halfZ of the detector for the plotting of the 2D profile of the photon hits on the SiPMs. It should match the halfZ of the detector in construction.cc and stepping.cc

// Uso: root -l 'plot_reflections.cpp("build2/output0_t1.root")'
void plot_reflections(const char* filename = "SiPM4x4mmBarra1x3x30.root") {

    TFile *f = TFile::Open(filename);
    if (!f || f->IsZombie()) { std::cerr << "Error: cannot open file" << std::endl; return; }
 
    // Crea la cartella di output (ricorsivo, non fallisce se esiste già)
    TString outDir = "plotsSiPM4x4mmBarra1x3x30";
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
    const double pathMin   = 0.,   pathMax   = 1000.;   // fPathLength [mm]
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

    TH1D *hPathLength = new TH1D("hPathLength", "Distanza percorsa dai fotoni; Length [mm]; Events", 100, 0, 1000);
    hPathLength->SetDirectory(0);
 
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
        hPathLength->Fill(fPathLength);
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
    double eff = 0., effErr = 0.; 
    if (nTotal > 0) {
        eff = double(nReached) / double(nTotal);
        effErr = std::sqrt(eff * (1 - eff) / nTotal);  // errore binomiale
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
    leg->AddEntry(hReflReached, Form("Raggiungono il rivelatore (%.1f%%)", eff * 100.0), "f");
    leg->AddEntry(hReflLost,    Form("Persi (%.1f%%)", (1.0 - eff) * 100.0), "f");

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

    // ── CANVAS 6: Fotoni rivelati per detector (post-efficienza) ────────────
    TTree *hits = (TTree*)f->Get("Hits");
    if (!hits) {
        std::cerr << "Warning: cannot find Hits tree, skipping canvas 4" << std::endl;
    } else {
        Int_t fDetectorID_hits;
        hits->SetBranchAddress("fDetectorID", &fDetectorID_hits);

        Long64_t nDet1 = 0, nDet2 = 0;
        Long64_t nHitsTotal = hits->GetEntries();

        for (Long64_t i = 0; i < nHitsTotal; i++) {
            hits->GetEntry(i);
            if      (fDetectorID_hits == 1) nDet1++;
            else if (fDetectorID_hits == 2) nDet2++;
        }
 
        TH1D *hDetCounts = new TH1D("hDetCounts",
                "Fotoni rivelati per SiPM (post-efficienza);;Fotoni rivelati",
                2, 0, 2);
        hDetCounts->SetDirectory(0);
        hDetCounts->SetBinContent(1, nDet1);
        hDetCounts->SetBinContent(2, nDet2);
        hDetCounts->SetBinError(1, std::sqrt((double)nDet1));   // errore Poisson
        hDetCounts->SetBinError(2, std::sqrt((double)nDet2));
        hDetCounts->GetXaxis()->SetBinLabel(1, "Detector 1 (+105mm)");
        hDetCounts->GetXaxis()->SetBinLabel(2, "Detector 2 (-105mm)");
        hDetCounts->GetXaxis()->SetLabelSize(0.05);
        hDetCounts->SetFillColorAlpha(kAzure+1, 0.7);
        hDetCounts->SetLineColor(kAzure+2);
        hDetCounts->SetLineWidth(2);
        hDetCounts->SetStats(0);
        hDetCounts->GetYaxis()->CenterTitle();

        TCanvas *c6 = new TCanvas("c6", "Fotoni rivelati per detector", 800, 650);
        gPad->SetLeftMargin(0.12);
        gPad->SetBottomMargin(0.1);
        hDetCounts->SetMaximum(std::max(nDet1, nDet2) * 1.3 + 1);
        hDetCounts->Draw("HIST E1");   // barre + barre d'errore verticali
        // Asimmetria: A = (N1 - N2)/(N1 + N2), errore propagato per Poisson indip.
        Long64_t nSum = nDet1 + nDet2;
        double asym = 0., asymErr = 0.;
        if (nSum > 0) {
            asym = double(nDet1 - nDet2) / double(nSum);
            asymErr = std::sqrt(4.0 * nDet1 * nDet2 / std::pow((double)nSum, 3));
        }
        TPaveText *pt6 = new TPaveText(0.55, 0.72, 0.88, 0.88, "NDC");
        pt6->SetFillColor(0);
        pt6->SetBorderSize(1);
        pt6->AddText(Form("Det 1: %lld", nDet1));
        pt6->AddText(Form("Det 2: %lld", nDet2));
        pt6->AddText(Form("Asimmetria: %.3f #pm %.3f", asym, asymErr));
        pt6->Draw();
        c6->Update();
        c6->SaveAs(outPath + "DetectorCounts.png");
        std::cout << "\n=== Conteggi per detector (Hits, post-efficienza) ===" << std::endl;
        std::cout << "Detector 1 (+105mm): " << nDet1 << std::endl;
        std::cout << "Detector 2 (-105mm): " << nDet2 << std::endl;
        if (nSum > 0)
            std::cout << "Asimmetria (N1-N2)/(N1+N2): " << asym << " +/- " << asymErr << std::endl;    
    }

    // ── CANVAS 7: Fotoni rivelati per evento, media per detector ────────────
    TTree *summary = (TTree*)f->Get("EventSummary");
    if (!summary) {
        std::cerr << "Warning: cannot find EventSummary tree, skipping canvas 7" << std::endl;
    } else {
        Int_t fEvt_s, fNDet1, fNDet2, fNCoinc;
        summary->SetBranchAddress("fEvent",         &fEvt_s);
        summary->SetBranchAddress("fNDet1",         &fNDet1);
        summary->SetBranchAddress("fNDet2",         &fNDet2);
        summary->SetBranchAddress("fNCoincidences", &fNCoinc);

        Long64_t nEvtSum = summary->GetEntries();

        // Range dinamico: prendi il massimo tra i due detector per fissare i bin
        Int_t maxN1 = (Int_t)summary->GetMaximum("fNDet1");
        Int_t maxN2 = (Int_t)summary->GetMaximum("fNDet2");
        Int_t maxN  = std::max(maxN1, maxN2);
        Int_t nbinsPerEvt = maxN + 2;   // un bin per ogni valore intero, piu' margine

        TH1D *hPerEvtDet1 = new TH1D("hPerEvtDet1",
            "Fotoni rivelati per evento;N fotoni rivelati;Eventi",
            nbinsPerEvt, 0, nbinsPerEvt);
        TH1D *hPerEvtDet2 = new TH1D("hPerEvtDet2", "", nbinsPerEvt, 0, nbinsPerEvt);
        hPerEvtDet1->SetDirectory(0);
        hPerEvtDet2->SetDirectory(0);

        for (Long64_t i = 0; i < nEvtSum; i++) {
            summary->GetEntry(i);
            hPerEvtDet1->Fill(fNDet1);
            hPerEvtDet2->Fill(fNDet2);
        }

        // ── 7a: distribuzioni sovrapposte (forma della distribuzione, non solo media) ──
        TCanvas *c7a = new TCanvas("c7a", "Fotoni per evento: Det1 vs Det2", 900, 650);
        gPad->SetLeftMargin(0.12);

        hPerEvtDet1->SetLineColor(kAzure+2);
        hPerEvtDet1->SetFillColorAlpha(kAzure+1, 0.35);
        hPerEvtDet1->SetFillStyle(3004);
        hPerEvtDet1->GetXaxis()->CenterTitle();
        hPerEvtDet1->GetYaxis()->CenterTitle();

        hPerEvtDet2->SetLineColor(kOrange+7);
        hPerEvtDet2->SetFillColorAlpha(kOrange+7, 0.35);
        hPerEvtDet2->SetFillStyle(3005);

        double ymaxPerEvt = std::max(hPerEvtDet1->GetMaximum(), hPerEvtDet2->GetMaximum());
        hPerEvtDet1->SetMaximum(ymaxPerEvt * 1.2);

        hPerEvtDet1->Draw("HIST");
        hPerEvtDet2->Draw("HIST SAME");

        double mean1 = hPerEvtDet1->GetMean(), rms1 = hPerEvtDet1->GetRMS();
        double mean2 = hPerEvtDet2->GetMean(), rms2 = hPerEvtDet2->GetRMS();
        double sem1  = rms1 / std::sqrt((double)nEvtSum);   // errore sulla media
        double sem2  = rms2 / std::sqrt((double)nEvtSum);

        TLegend *leg7 = new TLegend(0.55, 0.68, 0.88, 0.88);
        leg7->SetBorderSize(0);
        leg7->SetFillStyle(0);
        leg7->AddEntry(hPerEvtDet1, Form("Det 1: #mu = %.2f #pm %.2f", mean1, sem1), "f");
        leg7->AddEntry(hPerEvtDet2, Form("Det 2: #mu = %.2f #pm %.2f", mean2, sem2), "f");
        leg7->Draw();

        c7a->Update();
        c7a->SaveAs(outPath + "PhotonsPerEvent_Det1vsDet2.png");

        std::cout << "\n=== Fotoni rivelati per evento (media su " << nEvtSum << " eventi) ===" << std::endl;
        std::cout << "Detector 1: " << mean1 << " +/- " << sem1 << " (RMS = " << rms1 << ")" << std::endl;
        std::cout << "Detector 2: " << mean2 << " +/- " << sem2 << " (RMS = " << rms2 << ")" << std::endl;
    }

    TTree *coincidence = (TTree*)f->Get("Coincidences");
    if (!coincidence) {
        std::cerr << "Warning: cannot find Coincidences tree, skipping canvas 8" << std::endl;
    } else {
        Double_t fTime1, fTime2, fTimeDiff, fTimeAvg;
        coincidence->SetBranchAddress("fTime1", &fTime1);
        coincidence->SetBranchAddress("fTime2", &fTime2);
        coincidence->SetBranchAddress("fTimeDiff", &fTimeDiff); 
        coincidence->SetBranchAddress("fTimeAvg", &fTimeAvg);

        Long64_t nCoinc = coincidence->GetEntries();
        TH1D *hTime1 = new TH1D("hTime1", "Tempo di arrivo SiPM 1; Time [ns]; Events", 100, 0, 30);
        TH1D *hTime2 = new TH1D("hTime2", "Tempo di arrivo SiPM 2; Time [ns], Events", 100, 0, 30);
        hTime1->SetDirectory(0);
        hTime2->SetDirectory(0);

        for(Long64_t i =0; i < nCoinc; i++) {
            coincidence->GetEntry(i);
            hTime1->Fill(fTime1);
            hTime2->Fill(fTime2);
        }
        
        // ── 8: distribuzioni temporali di arrivo ──
        TCanvas *c8 = new TCanvas("c8", "Distribuizioni temporali di arrivo fotoni", 900, 650);
        gPad->SetLeftMargin(0.12);

        hTime1->SetLineColor(kAzure+2);
        hTime1->SetFillColorAlpha(kAzure+1, 0.35);
        hTime1->SetFillStyle(3004);
        hTime1->GetXaxis()->CenterTitle();
        hTime1->GetYaxis()->CenterTitle();

        hTime2->SetLineColor(kOrange+7);
        hTime2->SetFillColorAlpha(kOrange+7, 0.35);
        hTime2->SetFillStyle(3005);

        hTime1->Draw("HIST");
        hTime2->Draw("HIST SAME");

        TLegend *leg8 = new TLegend(0.55, 0.68, 0.88, 0.88);
        leg8->SetBorderSize(0);
        leg8->SetFillStyle(0);
        leg8->AddEntry(hTime1, "SiPM 1", "f");
        leg8->AddEntry(hTime2, "SiPM 2", "f");
        leg8->Draw();

        c8->Update();
        c8->SaveAs(outPath + "TemporalDistributions.png");
    }    

    // ── CANVAS 9: Lunghezza di percorso dei fotoni ───────────────
    TCanvas *c9 = new TCanvas("c9", "Percorso fotoni", 900, 650);
    gPad->SetLeftMargin(0.12);

    hPathLength->SetLineColor(kBlue);
    hPathLength->SetFillColor(kBlue);
    hPathLength->SetFillStyle(3002);
    hPathLength->GetXaxis()->CenterTitle();
    hPathLength->GetYaxis()->CenterTitle();
    hPathLength->Draw("HIST");

    c9->Update();
    c9->SaveAs(outPath + "PhotonPathLength.png");

    // ── CANVAS 10: Angolo medio vs posizione sulla faccia del radiatore ───────────────

    TTree *re = (TTree*)f->Get("RadiatorExit");

    if (!re) { 
        std::cerr << "Error: cannot find RadiatorExit tree" << std::endl; 
        return; }  else {
        Int_t fEvent_re, fTrackID_re, fWentToDetector, fTransmitted, fNRefl_re;
        Double_t fExitY, fExitZ, fAngleFromAxis, fPathLength_re, fWavelength_re, fExitX;

        re->SetBranchAddress("fExitX",          &fExitX);
        re->SetBranchAddress("fExitY",          &fExitY);
        re->SetBranchAddress("fExitZ",          &fExitZ);
        re->SetBranchAddress("fAngleFromAxis",  &fAngleFromAxis);
        re->SetBranchAddress("fTransmitted",    &fTransmitted);

        const int nbinsYZ = 50;
        const double halfY = 15.0;
        const double halfZ = 5.0;
        const double zCenter = 150.0;

        // Profilo 2D
        TProfile2D *pAngleVsPos = new TProfile2D("pAngleVsPos","Angolo medio vs posizione sulla faccia;Z [mm];Y [mm];Angolo [deg]",
            nbinsYZ, zCenter-halfZ, zCenter+halfZ, nbinsYZ, -halfY, halfY);
        pAngleVsPos->SetDirectory(0);

        // Istogramma 1D angolo
        TH1D *hAngle = new TH1D("hAngle", "Distribuzione angolo di incidenza;Angolo [deg];Entries", 100, 0, 100);
        hAngle->SetDirectory(0);

        for (Long64_t i = 0; i < re->GetEntries(); i++) {
            re->GetEntry(i);
            pAngleVsPos->Fill(fExitZ, fExitY, fAngleFromAxis);
            hAngle->Fill(fAngleFromAxis);
        }

        // Canvas divisa in due
        TCanvas *cAngle = new TCanvas("cAngle", "Distribuzione angolo e mappa", 1400, 650);
        cAngle->Divide(2,1);

        cAngle->cd(1);
        hAngle->Draw("HIST");

        cAngle->cd(2);
        gPad->SetRightMargin(0.15);
        pAngleVsPos->Draw("COLZ");

        cAngle->Update();
        cAngle->SaveAs(outPath + "AngleDistributionAndPosition.png");
    }

    // ── CANVAS 9: Angolo di incidenza al primo bordo, fotoni con N<=1 riflessioni ──
    TTree *fb = (TTree*)f->Get("FirstBounce");
    if (!fb) {
        std::cerr << "Warning: cannot find FirstBounce tree, skipping canvas 10" << std::endl;
    } else {
        // Costruisci una mappa (evento,trackID) -> fNReflections da PhotonReflections
        // per poter filtrare i fotoni "persi al primo bordo"
        std::map<std::pair<Int_t,Int_t>, Int_t> reflMap;
        {
            Int_t ev_pr, trk_pr, nrefl_pr;
            pr->SetBranchAddress("fEvent",        &ev_pr);
            pr->SetBranchAddress("fTrackID",      &trk_pr);
            pr->SetBranchAddress("fNReflections", &nrefl_pr);
            Long64_t nPR = pr->GetEntries();
            for (Long64_t i = 0; i < nPR; i++) {
                pr->GetEntry(i);
                reflMap[{ev_pr, trk_pr}] = nrefl_pr;
            }
            // Ripristina i branch address originali usati più sopra nel file
            pr->SetBranchAddress("fEvent",         &fEvent);
            pr->SetBranchAddress("fTrackID",       &fTrackID);
            pr->SetBranchAddress("fNReflections",  &fNReflections);
        }

        Int_t fEvent_fb, fTrackID_fb, fFace_fb, fTransmitted_fb;
        Double_t fAngleIncidence_fb, fWavelength_fb;
        fb->SetBranchAddress("fEvent",          &fEvent_fb);
        fb->SetBranchAddress("fTrackID",        &fTrackID_fb);
        fb->SetBranchAddress("fFace",           &fFace_fb);
        fb->SetBranchAddress("fAngleIncidence", &fAngleIncidence_fb);
        fb->SetBranchAddress("fTransmitted",    &fTransmitted_fb);
        fb->SetBranchAddress("fWavelength",     &fWavelength_fb);

        TH1D *hAngleTransmitted = new TH1D("hAngleTransmitted",
            "Angolo di incidenza al primo bordo (N riflessioni <= 1);Angolo [deg];Fotoni",
            90, 0., 90.);
        TH1D *hAngleTIR = new TH1D("hAngleTIR", "", 90, 0., 90.);
        hAngleTransmitted->SetDirectory(0);
        hAngleTIR->SetDirectory(0);

        Long64_t nFB = fb->GetEntries();
        Long64_t nUsed = 0, nSkippedFace2 = 0, nSkippedNotFound = 0, nSkippedMoreRefl = 0;

        for (Long64_t i = 0; i < nFB; i++) {
            fb->GetEntry(i);

            // Solo pareti laterali (y o z), non le facce terminali
            if (fFace_fb == 2) { nSkippedFace2++; continue; }

            auto it = reflMap.find({fEvent_fb, fTrackID_fb});
            if (it == reflMap.end()) { nSkippedNotFound++; continue; }

            if (it->second > 1) { 
                nSkippedMoreRefl++; continue; 
            }       // This one fill the histo only with photons that have NReflections <=1, massimo 1 riflessione totale, poi si perdono comunque

            if (fTransmitted_fb == 1) hAngleTransmitted->Fill(fAngleIncidence_fb);
            else                       hAngleTIR->Fill(fAngleIncidence_fb);
            nUsed++;
        }

        // Reference curve: theoretical critical angle vs wavelength, using the
        // actual dispersion table from construction.cc (RINDEX vs energy)
        const int nDisp = 22;
        double energy_eV[nDisp] = {
            1.239841939/1.01398, 1.239841939/0.85211, 1.239841939/0.70652, 1.239841939/0.65627,
            1.239841939/0.64385, 1.239841939/0.63280, 1.239841939/0.58929, 1.239841939/0.58756,
            1.239841939/0.54607, 1.239841939/0.53200, 1.239841939/0.48613, 1.239841939/0.47999,
            1.239841939/0.43583, 1.239841939/0.40466, 1.239841939/0.36501, 1.239841939/0.35500,
            1.239841939/0.33415, 1.239841939/0.31257, 1.239841939/0.29673, 1.239841939/0.28000,
            1.239841939/0.24840, 1.239841939/0.19340};
        double rindex[nDisp] = {
            1.5602, 1.5084, 1.4941, 1.4888, 1.4845, 1.4798, 1.4761, 1.4746, 1.4696, 1.4667,
            1.4635, 1.4631, 1.4607, 1.4601, 1.4585, 1.4584, 1.4570, 1.4567, 1.4564, 1.4552,
            1.4525, 1.4502};

        TGraph *gCritAngle = new TGraph(nDisp);
        for (int i = 0; i < nDisp; i++) {
            double wl_nm = 1239.841939 / energy_eV[i];   // E[eV] = 1239.84/lambda[nm]
            double thetaCrit_deg = std::asin(1.0/rindex[i]) * 180.0/TMath::Pi();
            gCritAngle->SetPoint(i, wl_nm, thetaCrit_deg);
        }
        gCritAngle->Sort();  // energy array isn't monotonic in wavelength, so re-sort
        gCritAngle->SetLineColor(kBlack);
        gCritAngle->SetLineStyle(2);
        gCritAngle->SetLineWidth(2);
        gCritAngle->SetMarkerStyle(20);
        gCritAngle->SetMarkerSize(0.6);

        // 2D: incidence angle vs wavelength, same filtered population as hAngleTIR/hAngleTransmitted
        TH2D *hAngleVsWL = new TH2D("hAngleVsWL",
            "Angolo di incidenza vs lunghezza d'onda (N riflessioni <= 1);#lambda [nm];Angolo [deg]",
            50, wlMin, wlMax, 90, 0., 90.);
        hAngleVsWL->SetDirectory(0);

        for (Long64_t i = 0; i < nFB; i++) {
            fb->GetEntry(i);
            if (fFace_fb == 2) continue;
            auto it = reflMap.find({fEvent_fb, fTrackID_fb});
            if (it == reflMap.end() ) continue;     // add this inside condition if you want only photons with max 1 reflection "|| it->second > 1"
            hAngleVsWL->Fill(fWavelength_fb, fAngleIncidence_fb);
        }

        TCanvas *c10 = new TCanvas("c10", "Angolo di incidenza al primo bordo", 1400, 650);
        c10->Divide(2, 1);

        c10->cd(1);
        gPad->SetLeftMargin(0.12);
        hAngleTIR->SetLineColor(kAzure+2);
        hAngleTIR->SetFillColorAlpha(kAzure+1, 0.35);
        hAngleTIR->SetFillStyle(3005);
        hAngleTIR->GetXaxis()->CenterTitle();
        hAngleTIR->GetYaxis()->CenterTitle();

        hAngleTransmitted->SetLineColor(kOrange+7);
        hAngleTransmitted->SetFillColorAlpha(kOrange+7, 0.35);
        hAngleTransmitted->SetFillStyle(3004);

        double ymax9 = std::max(hAngleTransmitted->GetMaximum(), hAngleTIR->GetMaximum());
        hAngleTIR->SetMaximum(ymax9 * 1.2);

        hAngleTIR->Draw("HIST");
        hAngleTransmitted->Draw("HIST SAME");

        // Linea verticale all'angolo critico teorico (~43.2 deg per SiO2/aria, n~1.46)
        TLine *lineCrit = new TLine(43.2, 0, 43.2, ymax9 * 1.2);
        lineCrit->SetLineColor(kBlack);
        lineCrit->SetLineStyle(2);
        lineCrit->SetLineWidth(2);
        lineCrit->Draw("SAME");

        TLegend *leg10 = new TLegend(0.15, 0.72, 0.5, 0.88);
        leg10->SetBorderSize(0);
        leg10->SetFillStyle(0);
        leg10->AddEntry(hAngleTIR,         Form("TIR / riflesso (%.0f)", hAngleTIR->GetEntries()), "f");
        leg10->AddEntry(hAngleTransmitted, Form("Trasmesso / perso (%.0f)", hAngleTransmitted->GetEntries()), "f");
        leg10->AddEntry(lineCrit, "Angolo critico @ 400nm (43.2#circ)", "l");
        leg10->Draw();

        c10->cd(2);
        gPad->SetRightMargin(0.15);
        hAngleVsWL->Draw("COLZ");
        gCritAngle->Draw("L SAME");

        TLegend *leg10b = new TLegend(0.5, 0.78, 0.88, 0.88);
        leg10b->SetBorderSize(0);
        leg10b->SetFillStyle(0);
        leg10b->AddEntry(gCritAngle, "Angolo critico teorico (dispersione)", "l");
        leg10b->Draw();

        c10->Update();
        c10->SaveAs(outPath + "FirstBounceAngle_N1orLess.png");

        std::cout << "\n=== FirstBounce ntuple diagnostics ===" << std::endl;
        std::cout << "Entries totali in FirstBounce: " << nFB << std::endl;
        std::cout << "Usati (face laterale, N<=1 rifless.): " << nUsed << std::endl;
        std::cout << "Scartati (fFace==2, faccia terminale): " << nSkippedFace2 << std::endl;
        std::cout << "Scartati (non trovati in PhotonReflections): " << nSkippedNotFound << std::endl;
        std::cout << "Scartati (N riflessioni > 1): " << nSkippedMoreRefl << std::endl;
    }

    f->Close();
}