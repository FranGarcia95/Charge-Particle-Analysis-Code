#include "DetectorCalibration.h" 

DetectorCalibration::DetectorCalibration(DetectorClass* det, UserInput *u){

    Detector = det;
    userInp = u;

}

DetectorCalibration::~DetectorCalibration(){}

void DetectorCalibration::Inizialization(){
    
    __Sample__ = userInp->Get_Sample();
    __Number_Strips__ = userInp->Get_Number_Strips();
    __Number_Peaks__ = userInp->Get_NumberPeak_Calibration();
    __Peak_Energy__ = userInp->Get_Simulated_Energy();
    __Peak_Energy_Error__ = userInp->Get_Simulated_Energy_Error();
    __Cal_Range__ = userInp->Get_Calibration_Range();
    __Width_Fit__ = userInp->Get_Width_Fit__();

    __amp_histo__ = Detector->Get_Amp_Histogram();

    for(int i = 0; i < __Sample__.size(); i++){
        for(int j = 0; j < __Number_Strips__; j++){
            __amp_histo__[i][j]->Rebin(5);
        }
    }

}

void DetectorCalibration::Processing(){

    TFile *calFile = new TFile("Linear_Calibration.root", "RECREATE");
    TFile *fitFile = new TFile("Peaks_Fitting_Plots.root", "RECREATE");

    for(int i = 0; i < __Number_Strips__; i++){

        for(int j = 0; j < __Cal_Range__.size(); j++){

    		std::cout << "Processing detector " << i + 1 << " for the sample " << __Sample__[__Cal_Range__[j][0]].c_str() << std::endl;

            __Means_Info__.push_back(Fitting_Peak(__Cal_Range__[j][i + 1], __amp_histo__[__Cal_Range__[j][0]][i], __Width_Fit__, fitFile, i + 1, j + 1));
	    
        }

        std::vector<float> means;
        std::vector<float> errors;

        for (const auto &v : __Means_Info__) {
            means.push_back(v[0]);    // fitted mean
            errors.push_back(v[1]);   // error on mean
        }

        linear_calibration(means, errors, i + 1, calFile);
        __Means_Info__.clear();
        means.clear();
        errors.clear();  

    }

    std::fstream file_energy_par;
    file_energy_par.open("Energy_Calibration_Parameters", std::ios::out);

    if(file_energy_par){

        for(int i = 0; i < __Number_Strips__; i++){

            file_energy_par << __Energy_Par__[i][0] << "     "  << __Energy_Par__[i][1] << std::endl;

        }

        file_energy_par.close();

    }else{
        std::cerr << "File have not been created" << std::endl;
    }

    calFile->Write();
    calFile->Close();
    delete calFile;

    std::cout << "The processing finished" << std::endl;

}

std::vector<float> DetectorCalibration::Fitting_Peak(float Mean, TH1F* histo, int width, TFile *outfile, int a, int b){

    float Constant = 0;
    float mean = 0;

    for(int i = 1; i < histo->GetXaxis()->GetNbins(); i++){
        if(i > histo->GetXaxis()->FindBin(Mean - width) && i < histo->GetXaxis()->FindBin(Mean + width)){
            if(Constant < histo->GetBinContent(i)){
                Constant = histo->GetBinContent(i);
                mean = histo->GetXaxis()->GetBinCenter(i);
            }
        }
    }
    //Our histogram is histo
    TF1 *func = new TF1(Form("gaus_strip_%d_%d", a, b), "gaus", mean - width, mean + width);
    // set the parameters to the mean and RMS of the histogram
    func->SetParameters(Constant, mean, 30);

    // give the parameters meaningful names
    func->SetParNames ("Constant","Mean_value","Sigma");
    //func->SetParLimits(1, Mean - Mea, Mean + 10);
    //func->SetParLimits(0, Constant - 50, Constant + 50);

    // call TH1::Fit with the name of the TF1 object
    histo->Fit(func, "R");

    outfile->cd();

    histo->SetName(Form("PeakFit_strip_%d_range_%d", a, b));
    histo->Write();

    return {(float)func->GetParameter(1), (float)func->GetParError(1)};
}

void DetectorCalibration::linear_calibration(std::vector<float> info_peak,
                                             std::vector<float> info_peak_err,
                                             int strip,
                                             TFile *outfile)
{
    // Known energies
    std::vector<float> energy = __Peak_Energy__;

    // Measured peak positions
    std::vector<float> channel = info_peak;

    // Errors from Gaussian fits
    std::vector<float> channelErr = info_peak_err;

    // Energies assumed exact
    std::vector<float> energyErr = __Peak_Energy_Error__;

    // Graph with errors
    TGraphErrors *gr = new TGraphErrors(
        __Number_Peaks__,
        channel.data(),
        energy.data(),
        channelErr.data(),
        energyErr.data()
    );

    gr->SetName(Form("Calibration_strip_%d", strip));
    gr->SetTitle(Form("Strip %d calibration;Channel;Energy [keV]", strip));

    // Draw points
    gr->SetMarkerStyle(20);
    gr->SetMarkerSize(1.2);
    gr->SetMarkerColor(kBlue+1);
    gr->SetLineColor(kBlue+1);

    // Linear function
    TF1 *f1;
    if(__Number_Peaks__ == 1)
        f1 = new TF1(Form("f1_%d", strip), "[1]*x", 0, 5000);
    else
        f1 = new TF1(Form("f1_%d", strip), "[0] + [1]*x", 0, 5000);

    f1->SetParameters(0, 0.001);

    // Fit
    gr->Fit(f1, "RQ");

    // Save parameters
    std::vector<float> energy_par = {
        (float)f1->GetParameter(0),
        (float)f1->GetParameter(1)
    };
    __Energy_Par__.push_back(energy_par);

    // Create canvas so the ROOT file stores the visual result
    TCanvas *c = new TCanvas(Form("cCalib_%d", strip), "", 800, 600);

    gr->Draw("AP");      // A = axes, P = points, errors are automatic
    f1->Draw("SAME");

    outfile->cd();
    gr->Write();
    f1->Write();
    c->Write();
}
