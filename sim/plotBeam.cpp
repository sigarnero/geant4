#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TLatex.h"
#include "TStyle.h"
#include <iostream>

void beamProfileY()
{
    TFile *file = TFile::Open("radiatorHeightScan1.6cm1000evt.root");

    TTree *tree = (TTree*)file->Get("BeamProfile");

    Double_t fY0;

    tree->SetBranchAddress("fY0", &fY0);

    // Radiator half-size in y
    // esempio: 12.5 mm --> altezza totale 25 mm
    double radiatorY = 12.5; // mm

    TH1D *hY = new TH1D("hY",
                        "Beam profile Y;Y position [mm];Entries",
                        100,
                        -30,
                        30);
    hY->SetLineColor(kAzure+2);
    hY->SetLineWidth(2);
    hY->SetFillColorAlpha(kAzure+2, 0.35);
    hY->SetFillStyle(3005);

    Long64_t nEntries = tree->GetEntries();

    int outside = 0;

    for(Long64_t i=0; i<nEntries; i++)
    {
        tree->GetEntry(i);

        hY->Fill(fY0);

        if(fabs(fY0) > radiatorY)
            outside++;
    }


    double percentage = 100.0 * outside / nEntries;


    std::cout << "Total protons = " << nEntries << std::endl;
    std::cout << "Outside radiator = " << outside << std::endl;
    std::cout << "Lost percentage = "
              << percentage << " %" << std::endl;



    TCanvas *c1 = new TCanvas("c1","Beam profile Y",800,600);

    hY->Draw();


    // Draw radiator borders
    TLine *left = new TLine(-radiatorY,
                            0,
                            -radiatorY,
                            hY->GetMaximum());

    TLine *right = new TLine(radiatorY,
                             0,
                             radiatorY,
                             hY->GetMaximum());


    left->SetLineColor(kRed);
    right->SetLineColor(kRed);

    left->SetLineWidth(2);
    right->SetLineWidth(2);

    left->Draw("same");
    right->Draw("same");


    TLatex latex;
    latex.SetNDC();

    latex.DrawLatex(0.15,0.85, Form("Outside = %.2f %%", percentage));

    c1->Update();
    c1->SaveAs("beamProfileY_height1.6cm1000evt.png");

}