// ----------------------------------------------------------------------------
// 'SCorrelatorPlotter.h'
// Derek Anderson
// 05.25.2023
//
// A small class to consolidate plotting routines associated with
// the sPHENIX Cold QCD Energy-Energy Correlator analysis.
// ----------------------------------------------------------------------------

#ifndef SCORRELATORPLOTTER_H
#define SCORRELATORPLOTTER_H

// standard c includes
#include <vector>
#include <cassert>
#include <iostream>
// class declarations
#include <TH1.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>

using namespace std;



// SCorrelatorPlotter definition --------------------------------------------------

// TODO inherit from abstract plotter class
namespace SColdQcdCorrelatorAnalysis {

  class SCorrelatorPlotter {
 
    public:

      // ctor/dtor
      SCorrelatorPlotter();
      ~SCorrelatorPlotter();

      /* TODO plotting methods go here */

    private:

      /* TODO members go here */

      /* TODO helper methods go here */

  };

}  // end SColdQcdCorrelatorAnalysis namespace

#endif

// end ------------------------------------------------------------------------
