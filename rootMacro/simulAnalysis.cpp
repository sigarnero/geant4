// ============================================================
//  analisi_ntupla.C  –  scheletro per una TTree/ntupla classica
//  Uso:  root -l 'analisi_ntupla.C("mio_file.root")'
// ============================================================
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TStyle.h"
#include "TROOT.h"

void analisi_ntupla(const char* filename = "ntupla.root",
                    const char* treename = "events")
{
    // ── 0. Stile globale ─────────────────────────────────────
    gROOT->SetBatch(kFALSE);          // kTRUE per girare senza schermo
    gStyle->SetOptStat(1111);         // mostra media, RMS, ecc.
    gStyle->SetOptFit(1111);          // mostra parametri fit
    gStyle->SetPalette(kRainBow);     // palette per TH2

    // ── 1. Apertura file e TTree ──────────────────────────────
    TFile* f = TFile::Open(filename, "READ");
    if (!f || f->IsZombie()) {
        ::Error("analisi_ntupla", "File non trovato: %s", filename);
        return;
    }

    TTree* tree = nullptr;
    f->GetObject(treename, tree);
    if (!tree) {
        ::Error("analisi_ntupla", "TTree '%s' non trovato", treename);
        f->Close();
        return;
    }

    Long64_t nEntries = tree->GetEntries();
    ::Info("analisi_ntupla", "Entries: %lld", nEntries);

    // ── 2. Variabili collegate ai branch ──────────────────────
    // Adatta i tipi e i nomi ai branch del tuo file
    Float_t  px, py, pz, energy;
    Float_t  mass;
    Int_t    charge;
    Int_t    nTracks;

    tree->SetBranchAddress("px",      &px);
    tree->SetBranchAddress("py",      &py);
    tree->SetBranchAddress("pz",      &pz);
    tree->SetBranchAddress("energy",  &energy);
    tree->SetBranchAddress("mass",    &mass);
    tree->SetBranchAddress("charge",  &charge);
    tree->SetBranchAddress("nTracks", &nTracks);

    // ── 3. Booking istogrammi ─────────────────────────────────
    // TH1F(nome, titolo; label assi, nBin, xMin, xMax)
    TH1F* hPt     = new TH1F("hPt",    "p_{T};p_{T} [GeV/c];Entries",   100, 0,  50);
    TH1F* hEta    = new TH1F("hEta",   "#eta;#eta;Entries",               60, -3,   3);
    TH1F* hMass   = new TH1F("hMass",  "Massa invariante;M [GeV/c^{2}];Entries",
                              200, 0, 200);
    TH1F* hEnergy = new TH1F("hEnergy","Energia;E [GeV];Entries",        100, 0, 500);
    TH1F* hNtrk   = new TH1F("hNtrk",  "N tracce;N_{tracks};Entries",    20,  0,  20);

    // Correlazione 2D
    TH2F* hPxPy   = new TH2F("hPxPy",  "p_{x} vs p_{y};p_{x} [GeV/c];p_{y} [GeV/c]",
                              100,-100,100, 100,-100,100);
    TH2F* hEtaPhi = new TH2F("hEtaPhi","#eta vs #phi;#eta;#phi [rad]",
                               60,-3,3, 64,-TMath::Pi(),TMath::Pi());

    // Profile: <E> in funzione di pT
    TProfile* hEvsP = new TProfile("hEvsP","<E> vs p_{T};p_{T} [GeV/c];<E> [GeV]",
                                   50, 0, 50);

    // ── 4. Loop sugli eventi ──────────────────────────────────
    for (Long64_t i = 0; i < nEntries; ++i) {

        tree->GetEntry(i);

        // — calcoli cinematici —
        TLorentzVector p4;
        p4.SetPxPyPzE(px, py, pz, energy);

        Double_t pt  = p4.Pt();
        Double_t eta = p4.Eta();
        Double_t phi = p4.Phi();

        // — tagli di selezione (adatta ai tuoi valori) —
        if (pt < 1.0)         continue;   // soglia minima pT
        if (TMath::Abs(eta) > 2.5) continue;  // acceptance
        if (charge == 0)      continue;   // solo cariche

        // — fill —
        hPt    ->Fill(pt);
        hEta   ->Fill(eta);
        hMass  ->Fill(mass);
        hEnergy->Fill(energy);
        hNtrk  ->Fill(nTracks);
        hPxPy  ->Fill(px, py);
        hEtaPhi->Fill(eta, phi);
        hEvsP  ->Fill(pt, energy);

        // — progresso ogni 10% —
        if (i % (nEntries/10 + 1) == 0)
            ::Info("loop", "%.0f%%", 100.*i/nEntries);
    }

    // ── 5. Draw e decorazioni ─────────────────────────────────
    // Palette uniforme per TH1
    hPt    ->SetLineColor(kAzure+1);   hPt    ->SetLineWidth(2);
    hEta   ->SetLineColor(kRed+1);     hEta   ->SetLineWidth(2);
    hMass  ->SetLineColor(kGreen+2);   hMass  ->SetLineWidth(2);
    hEnergy->SetLineColor(kOrange+1);  hEnergy->SetLineWidth(2);

    // Canvas 1: distribuzioni 1D (divisa in pad)
    TCanvas* c1 = new TCanvas("c1","Distribuzioni 1D",1200,900);
    c1->Divide(2,2);

    c1->cd(1); hPt    ->Draw("HIST"); // HIST = solo linea, no barre errore
    c1->cd(2); hEta   ->Draw("HIST");
    c1->cd(3); hMass  ->Draw("HIST");
    c1->cd(4); hEnergy->Draw("HIST");

    // Canvas 2: correlazioni 2D
    TCanvas* c2 = new TCanvas("c2","Correlazioni 2D",1200,500);
    c2->Divide(2,1);

    c2->cd(1);
    hPxPy->Draw("COLZ");    // COLZ = palette colori
    // opzione alternativa: "CONT4Z" per isocontour

    c2->cd(2);
    hEtaPhi->Draw("COLZ");

    // Canvas 3: Profile
    TCanvas* c3 = new TCanvas("c3","Profile",600,500);
    hEvsP->SetLineColor(kViolet+1);
    hEvsP->SetMarkerStyle(20);
    hEvsP->Draw("E1");       // E1 = barre d'errore

    // Esempio fit gaussiano sulla massa
    TCanvas* c4 = new TCanvas("c4","Fit massa",600,500);
    hMass->Fit("gaus","","", 85, 95);  // fit gaussiano in [85,95] GeV (es. Z)
    hMass->Draw("E1 SAME");

    // ── 6. Salvataggio ───────────────────────────────────────
    c1->SaveAs("distribuzioni_1D.pdf");
    c2->SaveAs("correlazioni_2D.pdf");
    c3->SaveAs("profile_E_vs_pT.pdf");
    c4->SaveAs("fit_massa.pdf");

    // Salva istogrammi su file ROOT
    TFile* fout = TFile::Open("output.root","RECREATE");
    hPt->Write(); hEta->Write(); hMass->Write();
    hPxPy->Write(); hEtaPhi->Write(); hEvsP->Write();
    fout->Close();

    f->Close();
    ::Info("analisi_ntupla", "Fatto.");
}