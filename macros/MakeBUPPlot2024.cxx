/// ===========================================================================
/*! \file   MakeBUPPlot2024.cxx
 *  \author Derek Anderson
 *  \date   10.22.2024
 *
 *  Short macro to plot several EEC distributions
 *  on top of each other and scale by an approriate
 *  weight to adjust statistics.
 */
/// ===========================================================================

// c++ utilities
#include <iostream>
#include <string>
#include <utility>
#include <vector>
// root libraries
#include <TCanvas.h>
#include <TError.h>
#include <TFile.h>
#include <TH1.h>
#include <TLegend.h>
#include <TLine.h>
#include <TPaveText.h>

// aliases for convenience
using spair = std::pair<std::string, std::string>;
using upair = std::pair<uint32_t, uint32_t>;
using dpair = std::pair<double, double>;
using fpair = std::pair<float, float>;
using style = std::tuple<int32_t, int32_t, int32_t, int32_t>;
using edges = std::array<double, 4>;



// ============================================================================
//! Calculate scale factor
// ============================================================================
double CalculateScaleFactor() {

  // input parameters
  const double  target_lumi  = 8.0e7;   // projected p+Au lumi [mb^-1]
  const double  pythia_xsec  = 0.0363;  // x-section for hard qcd [pthat > 7 GeV/c]
  const double  pythia_nevts = 1.4e7;   // no. of simulated pythia events

  // calculate scale factor
  const double target_nevts = 197 * target_lumi * pythia_xsec;
  const double scale_factor = pythia_nevts / target_nevts;
  return scale_factor;

}



// ============================================================================
//! Create projection plot of several EECs
// ============================================================================
void MakeBUPPlot2024() {

  // --------------------------------------------------------------------------
  // Parameters
  // --------------------------------------------------------------------------

  // i/o files
  const std::string in_file  = "twoPoint.pa200hijing500bgkd010jet10run6.trksWithOneGeVCstCut_true.d22m10y2024.root";
  const std::string out_file = "bup2024_eec_unscaled.pa200hijing500bkgd010jet10run6.trksWithOneGeVCstCut_true.d22m10y2024.root";

  // input histograms & output names
  const std::vector<spair> in_out_hists = {
    {"hPackageCorrelatorErrorDrAxis_ptJet10", "hEEC_PtJet10"},
    {"hPackageCorrelatorErrorDrAxis_ptJet20", "hEEC_PtJet20"},
    {"hPackageCorrelatorErrorDrAxis_ptJet30", "hEEC_PtJet30"},
    {"hPackageCorrelatorErrorDrAxis_ptJet40", "hEEC_PtJet40"}
  };

  // histogram styles & legend labels
  const std::vector<style> styles = {
    {883, 20, 0, 1},
    {602, 21, 0, 1},
    {863, 33, 0, 1},
    {843, 34, 0, 1}
  };
  const std::vector<std::string> labels = {
    "p_{T}^{jet} = 10 - 20 GeV/c",
    "p_{T}^{jet} = 20 - 30 GeV/c",
    "p_{T}^{jet} = 30 - 40 GeV/c",
    "p_{T}^{jet} > 40 GeV/c"
  };

  // plotting options
  const dpair plot_xrange = {0.03, 1.};
  const dpair plot_yrange = {0.00007, 0.7};

  // common histogram styles
  const std::string hist_title    = "";
  const spair       axis_titles   = {"R_{L}", "Normalized EEC"};
  const fpair       title_offsets = {1.0, 1.6};
  const fpair       title_sizes   = {0.04, 0.04};
  const fpair       label_sizes   = {0.04, 0.04};
  const uint32_t    hist_width    = 1;
  const uint32_t    hist_align    = 22;
  const uint32_t    title_center  = 1;
  const uint32_t    hist_font     = 42;

  // text in label
  const std::vector<std::string> text = {
    "#bf{#it{sPHENIX}} BUP 2024",
    "#bf{PYTHIA 8} #oplus #bf{HIJING}, #it{b} = 0 - 10 fm",
    "80 nb^{-1}, #it{p}+Au",
    "anti-k_{T}, R_{jet} = 0.4",
    "#bf{charged jets}"
  };

  // normalization routine
  auto normalize = [&plot_xrange](TH1D* hist) {
    const int32_t istart   = hist -> FindBin(plot_xrange.first);
    const int32_t istop    = hist -> FindBin(plot_xrange.second);
    const double  integral = hist -> Integral(istart, istop);
    if (integral > 0) {
      hist -> Scale(1. / integral);
    }
    return;
  };

  // scale options
  const bool   do_scale = false;
  const double scale    = CalculateScaleFactor();

  // text & legend options
  const uint32_t text_color  = 0;
  const uint32_t text_fill   = 0;
  const uint32_t text_line   = 0;
  const uint32_t text_font   = 42;
  const uint32_t text_align  = 12;
  const float    text_height = 0.05;
  const edges    text_dim    = {0.1, 0.1, 0.3, 0.1 + (text_height * text.size())};
  const edges    leg_dim     = {0.3, 0.1, 0.5, 0.1 + (text_height * labels.size())};

  // line options
  const uint32_t line_color = 923;
  const uint32_t line_line  = 9;
  const uint32_t line_width = 2;
  const edges    line_dim   = {0.4, plot_yrange.first, 0.4, plot_yrange.second};

  // canvas options
  const upair       canvas_dim    = {950, 950};
  const upair       canvas_log    = {1, 1};
  const upair       canvas_tick   = {1, 1};
  const upair       canvas_grid   = {0, 0};
  const edges       canvas_margin = {0.02, 0.02, 0.15, 0.15};
  const uint32_t    canvas_mode   = 0;
  const uint32_t    canvas_border = 2;
  const std::string canvas_name   = "cBUP2024";
  const std::string canvas_title  = "";

  // --------------------------------------------------------------------------
  // Open files, grab histograms
  // --------------------------------------------------------------------------

  // lower verbosity & announce start
  gErrorIgnoreLevel = kError;
  std::cout << "\n  Beginning BUP 2024 plot making!" << std::endl;

  // open files
  TFile* output = new TFile( out_file.data(), "recreate" );
  TFile* input  = new TFile( in_file.data(), "read" );
  if (!output || !input) {
    std::cerr << "PANIC: couldn't open a file!\n"
              << "       output = " << output << "\n"
              << "       input  = " << input
              << std::endl;
    return;
  }
  std::cout << "    Opened files." << std::endl;

  // grab input histograms
  std::vector<TH1D*> histograms;
  for (const auto& in_out_hist : in_out_hists) {
    histograms.push_back(
      (TH1D*) input -> Get( in_out_hist.first.data() )
    );
    if (!histograms.back()) {
      std::cerr << "PANIC: couldn't grab histogram '" << in_out_hist.first << "!" << std::endl;
      return;
    } else {
      histograms.back() -> SetName( in_out_hist.second.data() );
    }
  }
  std::cout << "    Grabbed input histograms." << std::endl;

  // --------------------------------------------------------------------------
  // Scale, normalize & set styles
  // --------------------------------------------------------------------------

  // scale if need be
  if (do_scale) {
    for (auto hist : histograms) {
      for (std::size_t ibin = 1; ibin <= hist -> GetNbinsX(); ++ibin) {
        const double value   = hist -> GetBinContent(ibin);
        const double error   = hist -> GetBinError(ibin);
        const double new_val = value / scale;
        const double new_err = error / std::sqrt(scale);
        hist -> SetBinContent(ibin, new_val);
        hist -> SetBinError(ibin, new_err);
      }
    }
    std::cout << "    Scaled histograms." << "\n"
              << "      scale = " << scale
              << std::endl;
  }

  // normalize histograms
  for (auto hist : histograms) {
    normalize(hist);
  }
  std::cout << "    Normalized histograms." << std::endl;

  // loop over histograms
  for (std::size_t ihist = 0; ihist < histograms.size(); ++ihist) {
    histograms[ihist] -> SetMarkerColor( get<0>(styles.at(ihist)) );
    histograms[ihist] -> SetMarkerStyle( get<1>(styles.at(ihist)) );
    histograms[ihist] -> SetFillColor( get<0>(styles.at(ihist)) );
    histograms[ihist] -> SetFillStyle( get<2>(styles.at(ihist)) );
    histograms[ihist] -> SetLineColor( get<0>(styles.at(ihist)) );
    histograms[ihist] -> SetLineStyle( get<3>(styles.at(ihist)) );
    histograms[ihist] -> SetLineWidth( hist_width );
    histograms[ihist] -> SetTitle( hist_title.data() );
    histograms[ihist] -> SetTitleFont( hist_font );
    histograms[ihist] -> GetXaxis() -> SetRangeUser( plot_xrange.first, plot_xrange.second );
    histograms[ihist] -> GetXaxis() -> SetTitle( axis_titles.first.data() );
    histograms[ihist] -> GetXaxis() -> SetTitleFont( hist_font );
    histograms[ihist] -> GetXaxis() -> SetTitleSize( title_sizes.first );
    histograms[ihist] -> GetXaxis() -> SetTitleOffset( title_offsets.first );
    histograms[ihist] -> GetXaxis() -> SetLabelFont( hist_font );
    histograms[ihist] -> GetXaxis() -> SetLabelSize( label_sizes.first );
    histograms[ihist] -> GetXaxis() -> CenterTitle( title_center );
    histograms[ihist] -> GetYaxis() -> SetRangeUser( plot_yrange.first, plot_yrange.second );
    histograms[ihist] -> GetYaxis() -> SetTitle( axis_titles.second.data() );
    histograms[ihist] -> GetYaxis() -> SetTitleFont( hist_font  );
    histograms[ihist] -> GetYaxis() -> SetTitleSize( title_sizes.second );
    histograms[ihist] -> GetYaxis() -> SetTitleOffset( title_offsets.second );
    histograms[ihist] -> GetYaxis() -> SetLabelFont( hist_font );
    histograms[ihist] -> GetYaxis() -> SetLabelSize( label_sizes.first );
    histograms[ihist] -> GetYaxis() -> CenterTitle( title_center );
  }
  std::cout << "    Set styles." << std::endl;

  // --------------------------------------------------------------------------
  // Make other objects
  // --------------------------------------------------------------------------

  // make legend
  TLegend* legend = new TLegend( leg_dim[0], leg_dim[1], leg_dim[2], leg_dim[3] );
  legend -> SetFillColor( text_color );
  legend -> SetFillStyle( text_fill );
  legend -> SetLineColor( text_color );
  legend -> SetLineStyle( text_line );
  legend -> SetTextFont( text_font );
  legend -> SetTextAlign( text_align );
  for (std::size_t ihist = 0; ihist < histograms.size(); ++ihist) {
    legend -> AddEntry( histograms[ihist], labels.at(ihist).data(), "pf" );
  }
  std::cout << "    Made legend." << std::endl;

  // make text box
  TPaveText* box = new TPaveText( text_dim[0], text_dim[1], text_dim[2], text_dim[3], "NDC NB" );
  box -> SetFillColor( text_color );
  box -> SetFillStyle( text_fill );
  box -> SetLineColor( text_color );
  box -> SetLineStyle( text_line );
  box -> SetTextFont( text_font );
  box -> SetTextAlign( text_align );
  for (const std::string& line : text) {
    box -> AddText( line.data() );
  }
  std::cout << "    Made text box." << std::endl;

  // make line
  TLine* line = new TLine( line_dim[0], line_dim[1], line_dim[2], line_dim[3] );
  line -> SetLineColor( line_color );
  line -> SetLineStyle( line_line );
  line -> SetLineWidth( line_width );
  std::cout << "    Made line." << std::endl;

  // --------------------------------------------------------------------------
  // Make plot
  // --------------------------------------------------------------------------

  // create canvas
  TCanvas* plot = new TCanvas( canvas_name.data(), canvas_title.data(), canvas_dim.first, canvas_dim.second );
  plot -> SetGrid( canvas_grid.first, canvas_grid.second );
  plot -> SetTicks( canvas_tick.first, canvas_tick.second );
  plot -> SetLogx( canvas_log.first );
  plot -> SetLogy( canvas_log.second );
  plot -> SetBorderMode( canvas_mode );
  plot -> SetBorderSize( canvas_border );
  plot -> SetTopMargin( canvas_margin[0] );
  plot -> SetRightMargin( canvas_margin[1] );
  plot -> SetBottomMargin( canvas_margin[2] );
  plot -> SetLeftMargin( canvas_margin[3] );
  plot -> cd();
  std::cout << "    Made canvas." << std::endl;

  // draw objects on it
  histograms[0] -> Draw();
  for (std::size_t ihist = 1; ihist < histograms.size(); ++ihist) {
    histograms[ihist] -> Draw("same");
  }
  line   -> Draw();
  box    -> Draw();
  legend -> Draw();
  std::cout << "    Drew objects." << std::endl;

  // save and close
  output -> cd();
  plot   -> Write();
  plot   -> Close();
  std::cout << "    Saved canvas." << std::endl;

  // --------------------------------------------------------------------------
  // Save histograms & close files
  // --------------------------------------------------------------------------

  // save histograms
  output -> cd();
  for (const auto hist : histograms) {
    hist -> Write();
  }
  std::cout << "    Saved histograms." << std::endl;

  // close files
  output -> cd();
  output -> Close();
  input  -> cd();
  input  -> Close();

  // announce end & exit
  std::cout << "  Finished making BUP 2024 plot!\n" << std::endl;
  return;

}

// end ========================================================================
