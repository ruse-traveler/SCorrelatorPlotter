// ----------------------------------------------------------------------------
// 'DoSubeventRatioChecks.cxx'
// Derek Anderson
// 10.17.2021
//
// Use this to do a couple of quick checks on
// the relative contribution of different
// subevents to the overall ENC.
// ----------------------------------------------------------------------------

#include <array>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <TH1.h>
#include <TPad.h>
#include <TFile.h>
#include <TLine.h>
#include <TError.h>
#include <TString.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TPaveText.h>

using namespace std;

// global constants
static const size_t NInput = 3;
static const size_t NCalc  = 4;
static const size_t NVtx   = 4;
static const size_t NPad   = 2;


void DoSubeventRatioChecks() {

  // lower verbosity
  gErrorIgnoreLevel = kError;
  cout << "\n  Beginning ratio subevent check..." << endl;

  // accessors
  enum Input {
    Bkgd,
    Sig,
    Tot
  };
  enum Calc {
    BkgdRatio,
    SigRatio,
    BkgdSigSum,
    SumRatio
  };
  enum Pad {
    Small,
    Big
  };

  // options ------------------------------------------------------------------

  // io parameters
  const array<string, NInput> arrInFileName = {
    "input/alex_for_subevent_checks/pa200hijing50bkd010run6jet10.true_sub2_modifiedConstit.d29m9y2023.root",
    "input/alex_for_subevent_checks/pa200hijing50bkd010run6jet10.true_sub1_modifiedConstit.d29m9y2023.root",
    "input/alex_for_subevent_checks/pa200hijing50bkd010run6jet10.true_sub0_modifiedConstit.d29m9y2023.root"
  };
  const array<string, NInput> arrInHistName = {
    "hCorrelatorVarianceDrAxis_ptJet10",
    "hCorrelatorVarianceDrAxis_ptJet10",
    "hCorrelatorVarianceDrAxis_ptJet10"
  };
  const array<string, NCalc> arrOutCalcName = {
    "hBkgdTotalRatio",
    "hSignalTotalRatio",
    "hBkgdSignalSum",
    "hSumRatio"
  };
  const array<string, NInput> arrOutHistName = {
    "hBackground",
    "hSignal",
    "hTotal"
  };
  const string sOutput("subeventRatioChecks_twoPoint_ptJet10.pa200hijing50bkgd010run6jet10.d17m10y2023.root");

  // general style parameters
  const string sTitle("");
  const string sTitleX("R_{L}");
  const string sTitleY("EEC");

  // histogram-specific parameters
  const array<uint32_t, NInput> fColInput  = {899, 859, 923};
  const array<uint32_t, NCalc>  fColCalc   = {899, 859, 879, 879};
  const array<uint32_t, NInput> fMarInput  = {26,  32,  20};
  const array<uint32_t, NCalc>  fMarCalc   = {26,  32,  24,  24};
  const array<float,    NInput> fWeights   = {1.,  1.,  1.};
  const array<string,   NCalc>  sTitleCalc = {
    "subevent / total",
    "subevent / total",
    "EEC",
    "sum / total"
  };

  // text parameters
  const array<string, NInput> arrInLabels = {
    "bkgd.",
    "signal",
    "total"
  };
  const array<string, NCalc> arrCalcLabels = {
    "bkgd. / total",
    "signal / total",
    "bkgd. + signal",
    "(bkgd. + signal) / total"
  };
  const vector<string> vecTxt = {
    "#bf{#it{sPHENIX}} Simulation [Run 6]",
    "p+Au, JS 10 GeV jet sample",
    "500 kHz, b = 0 - 10 fm",
    "#bf{charged jets}"
   };
  const string sHeader("#bf{p_{T}^{jet} #in (10, 15) GeV/c}");

  // plot range
  const pair<float, float> xPlotRange = {0.0005, 1.};

  // io operations ------------------------------------------------------------

  // open files
  TFile* fOutput = new TFile(sOutput.data(), "recreate");
  if (!fOutput) {
    cerr << "PANIC: couldn't open output file!\n" << endl;
    return;
  }

  array<TFile*, NInput> arrInFile;
  for (size_t iHist = 0; iHist < NInput; iHist++) {
    arrInFile[iHist] = new TFile(arrInFileName[iHist].data(), "read");
    if (!arrInFile[iHist]) {
      cerr << "PANIC: couldn't open input file " << iHist << "!\n" << endl;
      return;
    }
  }
  cout << "    Opened files." << endl;

  // grab input histograms
  array<TH1D*, NInput> arrInHist;
  for (size_t iHist = 0; iHist < NInput; iHist++) {
    arrInHist[iHist] = (TH1D*) arrInFile[iHist] -> Get(arrInHistName[iHist].data());
    if (!arrInHist[iHist]) {
      cerr << "PANIC: couldn't grab input histogram " << iHist << "!\n" << endl;
      return;
    } else {
      arrInHist[iHist] -> SetName(arrOutHistName[iHist].data());
    }
  }
  cout << "    Grabbed input histograms." << endl;

  // calculations -------------------------------------------------------------

  // create histograms for calculations
  array<TH1D*, NCalc> arrCalcHist;
  for (size_t iCalc = 0; iCalc < NCalc; iCalc++) {
    arrCalcHist[iCalc] = (TH1D*) arrInHist[0] -> Clone();
    arrCalcHist[iCalc] -> SetName(arrOutCalcName[iCalc].data());
    arrCalcHist[iCalc] -> Reset("ICES");
  }

  // sum signal and background
  arrCalcHist[Calc::BkgdSigSum] -> Add(arrInHist[Input::Bkgd], arrInHist[Input::Sig], fWeights[Input::Bkgd], fWeights[Input::Sig]);

  // take ratios
  arrCalcHist[Calc::BkgdRatio] -> Divide(arrInHist[Input::Bkgd],        arrInHist[Input::Tot], fWeights[Input::Bkgd], fWeights[Input::Tot]);
  arrCalcHist[Calc::SigRatio]  -> Divide(arrInHist[Input::Sig],         arrInHist[Input::Tot], fWeights[Input::Sig],  fWeights[Input::Tot]);
  arrCalcHist[Calc::SumRatio]  -> Divide(arrCalcHist[Calc::BkgdSigSum], arrInHist[Input::Tot], fWeights[Input::Tot],  fWeights[Input::Tot]);
  cout << "    Finished calculations." << endl;

  // set histogram styles and make text ---------------------------------------

  // general style parameters
  const uint32_t fAln(12);
  const uint32_t fFil(0);
  const uint32_t fLin(1);
  const uint32_t fWid(1);
  const uint32_t fTxt(42);
  const uint32_t fCnt(1);

  // pad-specific style parameters
  const array<float, NPad> fLab  = {0.074, 0.04};
  const array<float, NPad> fTit  = {0.074, 0.04};
  const array<float, NPad> fOffX = {1.1,   1.0};
  const array<float, NPad> fOffY = {0.7,   1.3};

  // set styles
  size_t iInput = 0;
  for (TH1D* hInput : arrInHist) {
    hInput -> SetMarkerColor(fColInput[iInput]);
    hInput -> SetMarkerStyle(fMarInput[iInput]);
    hInput -> SetFillColor(fColInput[iInput]);
    hInput -> SetFillStyle(fFil);
    hInput -> SetLineColor(fColInput[iInput]);
    hInput -> SetLineStyle(fLin);
    hInput -> SetLineWidth(fWid);
    hInput -> SetTitle(sTitle.data());
    hInput -> SetTitleFont(fTxt);
    hInput -> GetXaxis() -> SetRangeUser(xPlotRange.first, xPlotRange.second);
    hInput -> GetXaxis() -> SetTitle(sTitleX.data());
    hInput -> GetXaxis() -> SetTitleFont(fTxt);
    hInput -> GetXaxis() -> SetTitleSize(fTit[Pad::Big]);
    hInput -> GetXaxis() -> SetTitleOffset(fOffX[Pad::Big]);
    hInput -> GetXaxis() -> SetLabelFont(fTxt);
    hInput -> GetXaxis() -> SetLabelSize(fLab[Pad::Big]);
    hInput -> GetXaxis() -> CenterTitle(fCnt);
    hInput -> GetYaxis() -> SetTitle(sTitleY.data());
    hInput -> GetYaxis() -> SetTitleFont(fTxt);
    hInput -> GetYaxis() -> SetTitleSize(fTit[Pad::Big]);
    hInput -> GetYaxis() -> SetTitleOffset(fOffY[Pad::Big]);
    hInput -> GetYaxis() -> SetLabelFont(fTxt);
    hInput -> GetYaxis() -> SetLabelSize(fLab[Pad::Big]);
    hInput -> GetYaxis() -> CenterTitle(fCnt);
    ++iInput;
  }

  size_t iCalc = 0;
  for (TH1D* hCalc : arrCalcHist) {

    // set general styles
    hCalc -> SetMarkerColor(fColCalc[iCalc]);
    hCalc -> SetMarkerStyle(fMarCalc[iCalc]);
    hCalc -> SetFillColor(fColCalc[iCalc]);
    hCalc -> SetFillStyle(fFil);
    hCalc -> SetLineColor(fColCalc[iCalc]);
    hCalc -> SetLineStyle(fLin);
    hCalc -> SetLineWidth(fWid);
    hCalc -> SetTitle(sTitle.data());
    hCalc -> SetTitleFont(fTxt);
    hCalc -> GetXaxis() -> SetRangeUser(xPlotRange.first, xPlotRange.second);
    hCalc -> GetXaxis() -> SetTitle(sTitleX.data());
    hCalc -> GetXaxis() -> SetTitleFont(fTxt);
    hCalc -> GetXaxis() -> SetLabelFont(fTxt);
    hCalc -> GetXaxis() -> CenterTitle(fCnt);
    hCalc -> GetYaxis() -> SetTitle(sTitleCalc[iCalc].data());
    hCalc -> GetYaxis() -> SetTitleFont(fTxt);
    hCalc -> GetYaxis() -> SetLabelFont(fTxt);
    hCalc -> GetYaxis() -> CenterTitle(fCnt);
 
    // set calc-specific styles
    switch (iCalc) {
      case Calc::BkgdSigSum:
        hCalc -> GetXaxis() -> SetTitleSize(fTit[Pad::Big]);
        hCalc -> GetXaxis() -> SetTitleOffset(fOffX[Pad::Big]);
        hCalc -> GetXaxis() -> SetLabelSize(fLab[Pad::Big]);
        hCalc -> GetYaxis() -> SetTitleSize(fTit[Pad::Big]);
        hCalc -> GetYaxis() -> SetTitleOffset(fOffY[Pad::Big]);
        hCalc -> GetYaxis() -> SetLabelSize(fLab[Pad::Big]);
        break;
      default:
        hCalc -> GetXaxis() -> SetTitleSize(fTit[Pad::Small]);
        hCalc -> GetXaxis() -> SetTitleOffset(fOffX[Pad::Small]);
        hCalc -> GetXaxis() -> SetLabelSize(fLab[Pad::Small]);
        hCalc -> GetYaxis() -> SetTitleSize(fTit[Pad::Small]);
        hCalc -> GetYaxis() -> SetTitleOffset(fOffY[Pad::Big]);
        hCalc -> GetYaxis() -> SetLabelSize(fLab[Pad::Big]);
        break;
    }
    ++iCalc;
  }
  cout << "    Set styles." << endl;

  // make legend
  const uint32_t fColLeg(0);
  const uint32_t fFilLeg(0);
  const uint32_t fLinLeg(0);

  // legend dimensions
  const uint32_t           nRatio     = 2;
  const uint32_t           nSum       = 2;
  const float              yLegInput  = 0.15 + (NInput * 0.05);
  const float              yLegRatio  = 0.1  + (nRatio * 0.05);
  const float              yLegSum    = 0.15 + (nSum   * 0.05);
  const array<float, NVtx> xyLegInput = {0.1, 0.1, 0.3, yLegInput};
  const array<float, NVtx> xyLegRatio = {0.1, 0.1, 0.3, yLegRatio};
  const array<float, NVtx> xyLegSum   = {0.1, 0.1, 0.3, yLegSum};

  TLegend* legInput = new TLegend(xyLegInput[0], xyLegInput[1], xyLegInput[2], xyLegInput[3], sHeader.data());
  legInput -> SetFillColor(fColLeg);
  legInput -> SetFillStyle(fFilLeg);
  legInput -> SetLineColor(fColLeg);
  legInput -> SetLineStyle(fLinLeg);
  legInput -> SetTextFont(fTxt);
  legInput -> SetTextAlign(fAln);
  for (size_t iInput = 0; iInput < NInput; iInput++) {
    legInput -> AddEntry(arrInHist[iInput], arrInLabels[iInput].data(), "pf");
  }

  TLegend* legRatio = new TLegend(xyLegRatio[0], xyLegRatio[1], xyLegRatio[2], xyLegRatio[3]);
  legRatio -> SetFillColor(fColLeg);
  legRatio -> SetFillStyle(fFilLeg);
  legRatio -> SetLineColor(fColLeg);
  legRatio -> SetLineStyle(fLinLeg);
  legRatio -> SetTextFont(fTxt);
  legRatio -> SetTextAlign(fAln);
  legRatio -> AddEntry(arrCalcHist[Calc::BkgdRatio], arrCalcLabels[Calc::BkgdRatio].data(), "pf");
  legRatio -> AddEntry(arrCalcHist[Calc::SigRatio],  arrCalcLabels[Calc::SigRatio].data(),  "pf");

  TLegend* legSum = new TLegend(xyLegRatio[0], xyLegRatio[1], xyLegRatio[2], xyLegRatio[3], sHeader.data());
  legSum -> SetFillColor(fColLeg);
  legSum -> SetFillStyle(fFilLeg);
  legSum -> SetLineColor(fColLeg);
  legSum -> SetLineStyle(fLinLeg);
  legSum -> SetTextFont(fTxt);
  legSum -> SetTextAlign(fAln);
  legSum -> AddEntry(arrInHist[Input::Tot],         arrInLabels[Input::Tot].data(),         "pf");
  legSum -> AddEntry(arrCalcHist[Calc::BkgdSigSum], arrCalcLabels[Calc::BkgdSigSum].data(), "pf");
  cout << "    Made legends." << endl;

  // text box options
  const uint32_t fColTxt(0);
  const uint32_t fFilTxt(0);
  const uint32_t fLinTxt(0);

  // text box dimensions
  const float              yTxt  = 0.1 + (vecTxt.size() * 0.05);
  const array<float, NVtx> xyTxt = {0.3, 0.1, 0.5, yTxt};

  // make text box
  TPaveText* txt = new TPaveText(xyTxt[0], xyTxt[1], xyTxt[2], xyTxt[3], "NDC NB");
  txt -> SetFillColor(fColTxt);
  txt -> SetFillStyle(fFilTxt);
  txt -> SetLineColor(fColTxt);
  txt -> SetLineStyle(fLinTxt);
  txt -> SetTextFont(fTxt);
  txt -> SetTextAlign(fAln);
  for (const string sTxt : vecTxt) {
    txt -> AddText(sTxt.data());
  }
  cout << "    Made text box." << endl;

  // make plots ---------------------------------------------------------------

  const uint32_t fColLin(1);
  const uint32_t fLinLin(9);
  const uint32_t fWidLin(1);

  // line dimensions
  const array<float, NVtx> xyLin = {xPlotRange.first, 1., xPlotRange.second, 1.};

  // make line
  TLine* line = new TLine(xyLin[0], xyLin[1], xyLin[2], xyLin[3]);
  line -> SetLineColor(fColLin);
  line -> SetLineStyle(fLinLin);
  line -> SetLineWidth(fWidLin);
  cout << "    Made line." << endl;

  // plot options
  const uint32_t width(750);
  const uint32_t height(950);
  const uint32_t fMode(0);
  const uint32_t fBord(2);
  const uint32_t fGrid(0);
  const uint32_t fTick(1);
  const uint32_t fLogX(1);
  const uint32_t fLogY(1);
  const uint32_t fFrame(0);

  // plot margins
  const array<float, NVtx> topPadMargin    = {0.02,  0.02, 0.005, 0.15};
  const array<float, NVtx> bottomPadMargin = {0.005, 0.02, 0.15,  0.15};

  // pad dimensions
  const array<float, NVtx> xyTopPad    = {0., 0.35, 1., 1.};
  const array<float, NVtx> xyBottomPad = {0., 0.,   1., 0.35};

  // make plots
  TCanvas* cAll     = new TCanvas("cAllVsRatios", "", width, height);
  TPad*    pPadAllB = new TPad("pPadRatios",  "", xyBottomPad[0], xyBottomPad[1], xyBottomPad[2], xyBottomPad[3]);
  TPad*    pPadAllT = new TPad("pPadSpectra", "", xyTopPad[0],    xyTopPad[1],   xyTopPad[2],     xyTopPad[3]);
  cAll     -> SetGrid(fGrid, fGrid);
  cAll     -> SetTicks(fTick, fTick);
  cAll     -> SetBorderMode(fMode);
  cAll     -> SetBorderSize(fBord);
  pPadAllB -> SetGrid(fGrid, fGrid);
  pPadAllB -> SetTicks(fTick, fTick);
  pPadAllB -> SetLogx(fLogX);
  pPadAllB -> SetLogy(fLogY);
  pPadAllB -> SetBorderMode(fMode);
  pPadAllB -> SetBorderSize(fBord);
  pPadAllB -> SetFrameBorderMode(fFrame);
  pPadAllB -> SetTopMargin(bottomPadMargin[0]);
  pPadAllB -> SetRightMargin(bottomPadMargin[1]);
  pPadAllB -> SetBottomMargin(bottomPadMargin[2]);
  pPadAllB -> SetLeftMargin(bottomPadMargin[3]);
  pPadAllT -> SetGrid(fGrid, fGrid);
  pPadAllT -> SetTicks(fTick, fTick);
  pPadAllT -> SetLogx(fLogX);
  pPadAllT -> SetLogy(fLogY);
  pPadAllT -> SetFrameBorderMode(fFrame);
  pPadAllT -> SetTopMargin(topPadMargin[0]);
  pPadAllT -> SetRightMargin(topPadMargin[1]);
  pPadAllT -> SetBottomMargin(topPadMargin[2]);
  pPadAllT -> SetLeftMargin(topPadMargin[3]);
  cAll     -> cd();
  pPadAllB -> Draw();
  pPadAllT -> Draw();
  pPadAllB -> cd();
  for (size_t iCalc = 0; iCalc < NCalc; iCalc++) {
    switch (iCalc) {
      case Calc::BkgdRatio:
        arrCalcHist[Calc::BkgdRatio] -> Draw();
        break;
      case Calc::SigRatio:
        arrCalcHist[Calc::SigRatio] -> Draw("same");
        break;
    }
  }
  line     -> Draw();
  legRatio -> Draw();
  pPadAllT -> cd();
  for (size_t iInput = 0; iInput < NInput; iInput++) {
    if (iInput == 0) {
      arrInHist[iInput] -> Draw();
    } else {
      arrInHist[iInput] -> Draw("same");
    }
  }
  legInput -> Draw();
  txt      -> Draw();
  fOutput  -> cd();
  cAll     -> Write();
  cAll     -> Close();

  // make plots
  TCanvas* cSum     = new TCanvas("cSumVsRatios", "", width, height);
  TPad*    pPadSumB = new TPad("pPadRatios",  "", xyBottomPad[0], xyBottomPad[1], xyBottomPad[2], xyBottomPad[3]);
  TPad*    pPadSumT = new TPad("pPadSpectra", "", xyTopPad[0],    xyTopPad[1],   xyTopPad[2],     xyTopPad[3]);
  cSum                          -> SetGrid(fGrid, fGrid);
  cSum                          -> SetTicks(fTick, fTick);
  cSum                          -> SetBorderMode(fMode);
  cSum                          -> SetBorderSize(fBord);
  pPadSumB                      -> SetGrid(fGrid, fGrid);
  pPadSumB                      -> SetTicks(fTick, fTick);
  pPadSumB                      -> SetLogx(fLogX);
  pPadSumB                      -> SetLogy(fLogY);
  pPadSumB                      -> SetBorderMode(fMode);
  pPadSumB                      -> SetBorderSize(fBord);
  pPadSumB                      -> SetFrameBorderMode(fFrame);
  pPadSumB                      -> SetTopMargin(bottomPadMargin[0]);
  pPadSumB                      -> SetRightMargin(bottomPadMargin[1]);
  pPadSumB                      -> SetBottomMargin(bottomPadMargin[2]);
  pPadSumB                      -> SetLeftMargin(bottomPadMargin[3]);
  pPadSumT                      -> SetGrid(fGrid, fGrid);
  pPadSumT                      -> SetTicks(fTick, fTick);
  pPadSumT                      -> SetLogx(fLogX);
  pPadSumT                      -> SetLogy(fLogY);
  pPadSumT                      -> SetFrameBorderMode(fFrame);
  pPadSumT                      -> SetTopMargin(topPadMargin[0]);
  pPadSumT                      -> SetRightMargin(topPadMargin[1]);
  pPadSumT                      -> SetBottomMargin(topPadMargin[2]);
  pPadSumT                      -> SetLeftMargin(topPadMargin[3]);
  cSum                          -> cd();
  pPadSumB                      -> Draw();
  pPadSumT                      -> Draw();
  pPadSumB                      -> cd();
  arrCalcHist[Calc::SumRatio]   -> Draw();
  line                          -> Draw();
  pPadSumT                      -> cd();
  arrInHist[Input::Tot]         -> Draw();
  arrCalcHist[Calc::BkgdSigSum] -> Draw("same");
  legSum                        -> Draw();
  txt                           -> Draw();
  fOutput                       -> cd();
  cSum                          -> Write();
  cSum                          -> Close();
  cout << "    Made plots." << endl;

  // save and close -----------------------------------------------------------

  // save histograms
  fOutput -> cd();
  for (TH1D* hInput : arrInHist) {
    hInput -> Write();
  }
  for (TH1D* hCalc : arrCalcHist) {
    hCalc -> Write();
  }
  cout << "    Saved histograms." << endl;

  // close files
  for (TFile* fInput : arrInFile) {
    fInput -> cd();
    fInput -> Close();
  }
  fOutput -> cd();
  fOutput -> Close();
  cout << "  Done with ratio subevent check!\n" << endl;

}

// end ------------------------------------------------------------------------
