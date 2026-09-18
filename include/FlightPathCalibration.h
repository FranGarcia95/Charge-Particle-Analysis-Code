#ifndef FlightPathCalibration_h
#define FlightPathCalibration_h

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <map>
#include <sstream>

#include "TTree.h"
#include <TString.h>
#include "TSystem.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TCutG.h"
#include <TF1.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TLegend.h>
#include <TGraph.h>
#include "TMinuit.h"

#include "DetectorClass.h" 
#include "UserInput.h" 

class FlightPathCalibration{ 
    
    public:

        FlightPathCalibration(DetectorClass* det, UserInput *u);
        ~FlightPathCalibration();
        static FlightPathCalibration* current_instance;

        void Inizialization();
        void Processing();


    private:

        UserInput *userInp;
        DetectorClass *Detector;

        TFile *__flight_path_file__;

        TH1D**  __TOF_histo__; 
        TH1D**  __Energy_histo__; //amp histogram to see the deposited energy bunch by bunch
        TH1F*   __counting_rate__;
        TH1D *tof_fit, *energy_fit;

        TF1 *fit_tp_sim = nullptr, *fit_dip1_sim = nullptr, *fit_dip2_sim = nullptr, *fit_dip3_sim = nullptr, *fit_res_sim = nullptr;
        TF1 *fit_tp = nullptr, *fit_dip1 = nullptr, *fit_dip2 = nullptr, *fit_dip3 = nullptr, *fit_res = nullptr;

        //General variables for the histograms definition
        int numberoffits;
        std::vector<float> __energy__;
        std::vector<double> __energy_bin__;
        std::vector<double> __tof_energy_par__;
        std::vector<double> xp, xpERR, e, eERR;
        std::vector<float> __Exp_Upper_Limit__, __Exp_Lower_Limit__, __Sim_Upper_Limit__ ,__Sim_Lower_Limit__;
        std::vector<double> __Par_Exp_Thermal__, __Par_Exp_Dip1__, __Par_Exp_Dip2__, __Par_Exp_Dip3__, __Par_Exp_Res__;
        std::vector<double> __Par_Sim_Thermal__, __Par_Sim_Dip1__, __Par_Sim_Dip2__, __Par_Sim_Dip3__, __Par_Sim_Res__;

        std::vector<std::string> __Sample__; 
        std::string __Detector__;

        std::vector<bool> Peakstobefitted;
        bool __Readhistofromfile__;

        void Counting_Rate();
        void Counting_Rate_TC();
        TH1F* Rebinning( TH1F* histoin, TH1F* histoout);
        TH1D* Rebinning( TH1D* histoin, TH1D* histoout);
        double first_derivative(double x, double a, double b, double c);
        double second_derivative(double x, double a, double b);
        double solve_quadratic(double a, double b, double c, double x1, double x2);
        static double fitflight(double* v, double* par);
        static void logLikelihood(int &npar, double *grad, double &fval, double *par, int iflag);
        void flight_path_calibration();
        void flight_path_calibration_TC();
        //void flight_path_TC();

};
#endif