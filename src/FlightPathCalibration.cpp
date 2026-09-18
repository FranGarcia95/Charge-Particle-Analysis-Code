#include "FlightPathCalibration.h" 

FlightPathCalibration* FlightPathCalibration::current_instance = nullptr;

FlightPathCalibration::FlightPathCalibration(DetectorClass* det, UserInput *u){

    Detector = det;
    userInp = u;

}

FlightPathCalibration::~FlightPathCalibration(){

    delete __flight_path_file__;

}

void FlightPathCalibration::Inizialization(){

    Peakstobefitted = userInp->Get_Peak_Be_Fitted__();
    numberoffits = userInp->Get_Number_Fits();
    __Readhistofromfile__ = userInp->Get_Histo_From_File();
    __Sample__ = userInp->Get_Sample();
    __Detector__ = userInp->Get_One_Detector();

    if(__Readhistofromfile__){

        std::string file = userInp->Get_Histo_File();
        TFile *fflux = new TFile(file.c_str(), "read");
        if(userInp->Get_Apply_Pile_Up()){
            tof_fit = (TH1D*)fflux->Get(Form("Pile up corrected TOF spectra for detetector Front & sample %s. %s & Dedicated", __Sample__[0].c_str(), __Detector__.c_str()));
            energy_fit = (TH1D*)fflux->Get(Form("Pile up corrected Energy spectra for detetector Front & sample %s. %s & Dedicated", __Sample__[0].c_str(), __Detector__.c_str()));
        }else{
            tof_fit = (TH1D*)fflux->Get(Form("TOF spectra for detetector Front & sample %s. %s & Dedicated", __Sample__[0].c_str(), __Detector__.c_str()));
            energy_fit = (TH1D*)fflux->Get(Form("Energy spectra for detetector Front & sample %s. %s & Dedicated", __Sample__[0].c_str(), __Detector__.c_str()));
        }
        tof_fit->Scale(7e12 * 1000);
        energy_fit->Scale(7e12 * 1000);

    }else{

        __TOF_histo__ = Detector->Get_TOF_Histogram();
        __Energy_histo__ = Detector->Get_Energy_Histogram();
        tof_fit = (TH1D*)__TOF_histo__[0]->Clone();
        energy_fit = (TH1D*)__Energy_histo__[0]->Clone();

    }

    __energy__ = Detector->Get_Energy();
    __energy_bin__ = Detector->Get_Energy_Bin();

    userInp->Get_FlightPath_Calibration_Range(__Sim_Lower_Limit__, __Sim_Upper_Limit__, __Exp_Lower_Limit__, __Exp_Upper_Limit__);
    userInp->Get_FlightPath_Experimental_Parameters(__Par_Exp_Thermal__, __Par_Exp_Dip1__, __Par_Exp_Dip2__, __Par_Exp_Dip3__, __Par_Exp_Res__);
    userInp->Get_FlightPath_Simulation_Parameters(__Par_Sim_Thermal__, __Par_Sim_Dip1__, __Par_Sim_Dip2__, __Par_Sim_Dip3__, __Par_Sim_Res__);

}

void FlightPathCalibration::Processing(){

    if(userInp->Get_TC_FlightPath()){
        Counting_Rate_TC();
        __flight_path_file__ = new TFile("Flight_Path_Calibration.root","RECREATE");
        flight_path_calibration_TC();
    }else{
        Counting_Rate();
        __flight_path_file__ = new TFile("Flight_Path_Calibration.root","RECREATE");
        flight_path_calibration();
    }
    
    // Make sure everything is written
    __flight_path_file__->Write();
    __flight_path_file__->Close();

}

void FlightPathCalibration::Counting_Rate(){

    // Read the flux
    TFile *fflux = new TFile(userInp->Get_Evaluated_Flux().c_str(), "read");
    TH1F *eval = (TH1F*)fflux->Get("h_flux_ear2");
    
    //Read the cross section
    TH1F* hcross;

    std::ifstream rfile(userInp->Get_Cross_Section().c_str());
    if(!rfile.good()){
        std::cerr << "Cross Section file not found!" << std::endl;
        //exit();
    }

    std::string fline, col1, col2;
    std::vector<double> energycs;
    std::vector<double> csvalue;
    int bins = 0;

    while(getline(rfile, fline)){
        std::istringstream ss(fline);
        ss >> col1 >> col2;
        energycs.push_back(stod(col1));
        csvalue.push_back(stod(col2));
        bins++;
    }

    hcross = new TH1F("Lithium cross section", "Lithium cross section",
                                       bins - 1, energycs.data());
    hcross->SetDirectory(__flight_path_file__);
    hcross->Sumw2();
    hcross->GetXaxis()->SetTitle("Energy [keV]");
    hcross->GetYaxis()->SetTitle(Form("barn"));

    for(int i = 1; i < bins; i++){

        hcross->SetBinContent(i, csvalue[i - 1]);

    }
    
    // create count histogram with logarithmic binning for x axis, commonly used for neutron energy histograms
    
    __counting_rate__ = new TH1F("Counting rate for the Lithium sample", "Counting rate for the Lithium sample",
                                       __energy__[2], __energy_bin__.data());
    __counting_rate__->SetDirectory(__flight_path_file__);
    __counting_rate__->Sumw2();
    __counting_rate__->GetXaxis()->SetTitle("Energy [keV]");
    __counting_rate__->GetYaxis()->SetTitle(Form("Counts"));

    // function to rebin the cross section histogram to the same binning as the counts histogram
    TH1F* rebin;
    rebin = Rebinning(hcross, __counting_rate__);
    rebin->Sumw2();
    rebin->GetXaxis()->SetTitle("Energy [keV]");
    rebin->GetYaxis()->SetTitle(Form("barn"));

    Int_t auxbin1;
    Float_t scaler1;
    
    // multiply by the n_TOF neutron flux (binning is in units of ExdPhi/dE, so independent of binning)

    for(int i=1; i<=__counting_rate__->GetNbinsX(); i++){
        auxbin1 = eval->FindBin(__counting_rate__->GetBinCenter(i));
        scaler1 = TMath::Log(__counting_rate__->GetBinLowEdge(i+1)/__counting_rate__->GetBinLowEdge(i));

        if(rebin->GetBinContent(i)>0 && eval->GetBinContent(auxbin1)>0){
            __counting_rate__->SetBinContent(i, rebin->GetBinContent(i)*eval->GetBinContent(auxbin1)*scaler1);
        }


    }
    
    for(int i=1;i<=__counting_rate__->GetNbinsX();i++){
        if(__counting_rate__->GetBinContent(i)==0)__counting_rate__->SetBinContent(i,1);
        __counting_rate__->SetBinError(i,0);
    }
 
    fflux->Close();
    rfile.close();

}

void FlightPathCalibration::Counting_Rate_TC(){

    //Read yield
    TH1F* hyield;

    std::ifstream rfile(userInp->Get_Counting_Rate_TC().c_str());
    if(!rfile.good()){
        std::cerr << "Yield file not found!" << std::endl;
        //exit();
    }

    std::string fline, col1, col2, col3, col4;
    std::vector<double> energyyield;
    std::vector<double> yieldvalue;
    int bins = 0;

    while(getline(rfile, fline)){

        if(fline.empty() || fline[0]=='#')
            continue;

        std::istringstream ss(fline);

        ss >> col1 >> col2 >> col3 >> col4;

        energyyield.push_back(pow(10, stod(col1)) * 1.0e9);
        yieldvalue.push_back(stod(col3)*7e12);
        bins++;

    }

    // add final edge
    energyyield.push_back(pow(10, stod(col2))*1e9);

    hyield = new TH1F("Lithium cross section", "Lithium cross section",
                                       bins - 1, energyyield.data());
    hyield->SetDirectory(__flight_path_file__);
    hyield->Sumw2();
    hyield->GetXaxis()->SetTitle("TOF [ns]");
    hyield->GetYaxis()->SetTitle(Form("barn"));

    for(int i = 1; i < bins; i++){

        hyield->SetBinContent(i, yieldvalue[i - 1]);

    }

    __counting_rate__ = new TH1F("Counting rate for the Lithium sample", "Counting rate for the Lithium sample",
                                       __energy__[2], __energy_bin__.data());
    __counting_rate__->SetDirectory(__flight_path_file__);
    __counting_rate__->Sumw2();
    __counting_rate__->GetXaxis()->SetTitle("Energy [keV]");
    __counting_rate__->GetYaxis()->SetTitle(Form("Counts"));

    // function to rebin the cross section histogram to the same binning as the counts histogram
    TH1F* rebin;
    rebin = Rebinning(hyield, __counting_rate__);
    rebin->Sumw2();
    rebin->GetXaxis()->SetTitle("Energy [keV]");
    rebin->GetYaxis()->SetTitle(Form("barn"));

    __counting_rate__ = rebin;

    rfile.close();

}

TH1F* FlightPathCalibration::Rebinning( TH1F* histoin, TH1F* histoout){
    /*********************************************************************************
     ***                          NEWTON INTERPOLATION                              ***
    **********************************************************************************/
    /**  The idea is to apply the Newton interpolation by taking two consecutives  *** 
     ***point of the histogram, then evaluating the function between each two points***
    ***between the point of the histogram generated. Therefore, we take two points,***
    ***and then we fill the new histogram with these points bin to bin regarding the**
    ***interpolation function*********************************************************
    ***********************************************************************/
    //We define this vector to compare the histogram bin by bin, in this way
    //we will know how many bins are related to both histograms, and then
    //we choose the number of bins to maje the proper rebinning 

    //Here we take the number of bins from the original histogram
     if (!histoin) {
        std::cerr << "Original histogram is null!" << std::endl;
        return nullptr;
    }

    // Extract original bin edges and contents
    int nbins = histoin->GetNbinsX();
    std::vector<double> original_bins(nbins + 1);
    std::vector<double> original_content(nbins);

    for (int i = 1; i <= nbins; i++) {
        original_bins[i - 1] = histoin->GetXaxis()->GetBinLowEdge(i);
        original_content[i - 1] = histoin->GetBinContent(i);
    }
    original_bins[nbins] = histoin->GetXaxis()->GetBinUpEdge(nbins);

    // Use binning from histo2 or keep original binning
    std::vector<double> new_bins;
    if (histoout) {
        int new_nbins = histoout->GetNbinsX();
        for (int i = 1; i <= new_nbins; i++) {
            new_bins.push_back(histoout->GetXaxis()->GetBinLowEdge(i));
        }
        new_bins.push_back(histoout->GetXaxis()->GetBinUpEdge(new_nbins));
    } else {
        std::cerr << "No histogram provided for rebinning!" << std::endl;
        return nullptr;
    }

    // Create new histogram
    TH1F* HRebin = new TH1F("HRebin", "Rebinned Histogram", new_bins.size() - 1, new_bins.data());
    HRebin->SetDirectory(0);
    // Fill new histogram with Newton interpolation
    for (std::size_t i = 0; i < new_bins.size() - 1; i++) {
        double x = (new_bins[i] + new_bins[i + 1]) / 2; // Midpoint of the bin
        double y = 0.0;

        // Newton interpolation
        for (std::size_t j = 0; j < original_bins.size() - 1; j++) {
            if (x >= original_bins[j] && x < original_bins[j + 1] ) {
                double x1 = original_bins[j];
                double x2 = original_bins[j + 1];
                double y1 = original_content[j];
                double y2 = original_content[j + 1];
                y = y1 + (x - x1) * (y2 - y1) / (x2 - x1); // Linear interpolation
                //break;
            }
        }

        HRebin->SetBinContent(i + 1, y);
    }
 	   		
  return HRebin;
}

TH1D* FlightPathCalibration::Rebinning( TH1D* histoin, TH1D* histoout){
    /*********************************************************************************
     ***                          NEWTON INTERPOLATION                              ***
    **********************************************************************************/
    /**  The idea is to apply the Newton interpolation by taking two consecutives  *** 
     ***point of the histogram, then evaluating the function between each two points***
    ***between the point of the histogram generated. Therefore, we take two points,***
    ***and then we fill the new histogram with these points bin to bin regarding the**
    ***interpolation function*********************************************************
    ***********************************************************************/
    //We define this vector to compare the histogram bin by bin, in this way
    //we will know how many bins are related to both histograms, and then
    //we choose the number of bins to maje the proper rebinning 

    //Here we take the number of bins from the original histogram
     if (!histoin) {
        std::cerr << "Original histogram is null!" << std::endl;
        return nullptr;
    }

    // Extract original bin edges and contents
    int nbins = histoin->GetNbinsX();
    std::vector<double> original_bins(nbins + 1);
    std::vector<double> original_content(nbins);

    for (int i = 1; i <= nbins; i++) {
        original_bins[i - 1] = histoin->GetXaxis()->GetBinLowEdge(i);
        original_content[i - 1] = histoin->GetBinContent(i);
    }
    original_bins[nbins] = histoin->GetXaxis()->GetBinUpEdge(nbins);

    // Use binning from histo2 or keep original binning
    std::vector<double> new_bins;
    if (histoout) {
        int new_nbins = histoout->GetNbinsX();
        for (int i = 1; i <= new_nbins; i++) {
            new_bins.push_back(histoout->GetXaxis()->GetBinLowEdge(i));
        }
        new_bins.push_back(histoout->GetXaxis()->GetBinUpEdge(new_nbins));
    } else {
        std::cerr << "No histogram provided for rebinning!" << std::endl;
        return nullptr;
    }

    // Create new histogram
    TH1D* HRebin = new TH1D("HRebin", "Rebinned Histogram", new_bins.size() - 1, new_bins.data());
    HRebin->SetDirectory(0);
    // Fill new histogram with Newton interpolation
    for (std::size_t i = 0; i < new_bins.size() - 1; i++) {
        double x = (new_bins[i] + new_bins[i + 1]) / 2; // Midpoint of the bin
        double y = 0.0;

        // Newton interpolation
        for (std::size_t j = 0; j < original_bins.size() - 1; j++) {
            if (x >= original_bins[j] && x < original_bins[j + 1] ) {
                double x1 = original_bins[j];
                double x2 = original_bins[j + 1];
                double y1 = original_content[j];
                double y2 = original_content[j + 1];
                y = y1 + (x - x1) * (y2 - y1) / (x2 - x1); // Linear interpolation
                //break;
            }
        }

        HRebin->SetBinContent(i + 1, y);
    }
 	   		
  return HRebin;
}

double FlightPathCalibration::first_derivative(double x, double a, double b, double c) {
    return 3*a*x*x + 2*b*x + c;
}

double FlightPathCalibration::second_derivative(double x, double a, double b) {
    return 6*a*x + 2*b;
}

double FlightPathCalibration::solve_quadratic(double a, double b, double c, double x1, double x2) {
    double discriminant = b*b - 4*a*c;
    if (discriminant >= 0) {
        x1 = (-b + sqrt(discriminant)) / (2*a);
        x2 = (-b - sqrt(discriminant)) / (2*a);
    } else {
        std::cout << "no roots" << std::endl;
    }
    return x2;
}

double FlightPathCalibration::fitflight(double* v, double* par){

const double m=939565560.81;            //Neutron Mass ev/c2
const double c0=29.972458;    //cm/ns
Double_t fitval=m/2/c0/c0*pow(par[0]/(v[0]-par[1]),2);
return fitval;

}

void FlightPathCalibration::flight_path_calibration(){

    tof_fit->Rebin(2);
    tof_fit->Scale(1.0);
    
    TH1F *sim_fit = (TH1F*)__counting_rate__->Clone();
    sim_fit->SetDirectory(0);
    
    //tof_fit is related to the tof spectra
    //sim_fit is the simulated energy from lithium

    int peak = 0;

    //The first fitting is the thermal peak. Using a third degree polinom
    if(Peakstobefitted[0]){
        fit_tp = new TF1("par", "[0]*x*x*x + [1]*x*x + [2]*x + [3]", __Exp_Lower_Limit__[peak], __Exp_Upper_Limit__[peak]);
        fit_tp->SetParameters(__Par_Exp_Thermal__[0], __Par_Exp_Thermal__[1], __Par_Exp_Thermal__[2], __Par_Exp_Thermal__[3]);
        tof_fit->Fit(fit_tp, "R");

        double a = fit_tp->GetParameter(0);
        double b = fit_tp->GetParameter(1);
        double c = fit_tp->GetParameter(2);
        double d = fit_tp->GetParameter(3);

        double x1, x2;
        x2 = solve_quadratic(3*a, 2*b, c, x1, x2);

        // Check which root corresponds to a peak (maximum or minimum)
        double second_deriv_at_x1 = second_derivative(x1, a, b);
        double second_deriv_at_x2 = second_derivative(x2, a, b);

        xp.push_back(x2);
        xpERR.push_back(0.001*x2);

        peak++;
    }

    if(Peakstobefitted[1]){
        fit_dip1 = new TF1("dip1", "[0] + [4] * x - [1]*exp(-0.5*((x-[2])/[3])**2)",__Exp_Lower_Limit__[peak],__Exp_Upper_Limit__[peak]);
        fit_dip1->SetParameters(__Par_Exp_Dip1__[0], __Par_Exp_Dip1__[1], __Par_Exp_Dip1__[2], __Par_Exp_Dip1__[3], __Par_Exp_Dip1__[4]); //bg,amp,mean,sigma
        tof_fit->Fit(fit_dip1, "RL");
        xp.push_back(fit_dip1->GetParameter(2));
        xpERR.push_back(4*fit_dip1->GetParError(2));

        peak++;
    }

    if(Peakstobefitted[2]){
        fit_dip2 = new TF1("dip2", "[0] - [1]*exp(-0.5*((x-[2])/[3])**2)",__Exp_Lower_Limit__[peak],__Exp_Upper_Limit__[peak]);
        fit_dip2->SetParameters(__Par_Exp_Dip2__[0], __Par_Exp_Dip2__[1], __Par_Exp_Dip2__[2], __Par_Exp_Dip2__[3]); //bg,amp,mean,sigma
        tof_fit->Fit(fit_dip2, "R");  
        
        xp.push_back(fit_dip2->GetParameter(2));
        xpERR.push_back(4*fit_dip2->GetParError(2));

        peak++;
    }

    if(Peakstobefitted[3]){
        fit_dip3 = new TF1("dip3", "[0] - [1]*exp(-0.5*((x-[2])/[3])**2)",__Exp_Lower_Limit__[peak],__Exp_Upper_Limit__[peak]);
        fit_dip3->SetParameters(__Par_Exp_Dip3__[0], __Par_Exp_Dip3__[1], __Par_Exp_Dip3__[2], __Par_Exp_Dip3__[3]); //bg,amp,mean,sigma
        tof_fit->Fit(fit_dip3, "R");

        xp.push_back(fit_dip3->GetParameter(2));
        xpERR.push_back(4*fit_dip3->GetParError(2));

        peak++;
    }

    if(Peakstobefitted[4]){
        fit_res = new TF1("bw", "[0]/((x-[1])*(x-[1]) + 0.25*[2]*[2])",__Exp_Lower_Limit__[peak],__Exp_Upper_Limit__[peak]);
        fit_res->SetParameters(__Par_Exp_Res__[0], __Par_Exp_Res__[1], __Par_Exp_Res__[2]); //amp,mean,sigma
        fit_res->SetParNames("Amplitude", "Peak", "Width");
        tof_fit->Fit(fit_res, "R"); 
        
        xp.push_back(fit_res->GetParameter(1));
        xpERR.push_back(fit_res->GetParError(1));

        peak++;
    }

    //start fitting Simulation
    std::cout << "Starting the minimization of the Simulations" << std::endl;
    peak = 0;

    if(Peakstobefitted[0]){

        fit_tp_sim = new TF1("par_sim", "[0]*x*x*x + [1]*x*x + [2]*x + [3]",__Sim_Lower_Limit__[peak],__Sim_Upper_Limit__[peak]);
        fit_tp_sim->SetParameters(__Par_Sim_Thermal__[0], __Par_Sim_Thermal__[1], __Par_Sim_Thermal__[2], __Par_Sim_Thermal__[3]);
        sim_fit->Fit(fit_tp_sim, "RL");

        double as = fit_tp_sim->GetParameter(0);
        double bs = fit_tp_sim->GetParameter(1);
        double cs = fit_tp_sim->GetParameter(2);
        double ds = fit_tp_sim->GetParameter(3);

        std::cout << as << "   " << bs << "   " << cs << "   " << ds << std::endl;

        double x1sim = 0, x2sim = 0;
        x2sim = solve_quadratic(3*as, 2*bs, cs, x1sim, x2sim);

        // Check which root corresponds to a peak (maximum or minimum)
        double second_deriv_at_x1s = second_derivative(x1sim, as, bs);
        double second_deriv_at_x2s = second_derivative(x2sim, as, bs);

        e.push_back(x2sim);
        eERR.push_back(0.001*x2sim);

        peak++;
    }

    if(Peakstobefitted[1]){

        fit_dip1_sim = new TF1("dip1res", "[0] + [4] * x - [1]*exp(-0.5*((x-[2])/[3])**2)",__Sim_Lower_Limit__[peak],__Sim_Upper_Limit__[peak]);
        fit_dip1_sim->SetParameters(__Par_Sim_Dip1__[0], __Par_Sim_Dip1__[1], __Par_Sim_Dip1__[2], __Par_Sim_Dip1__[3], __Par_Sim_Dip1__[4]); //bg,amp,mean,sigma
        sim_fit->Fit(fit_dip1_sim, "RL");
        
        e.push_back(fit_dip1_sim->GetParameter(2));
        eERR.push_back(fit_dip1_sim->GetParError(2));

        peak++;
    }

    if(Peakstobefitted[2]){

        fit_dip2_sim = new TF1("dip2res", "[0] - [1]*exp(-0.5*((x-[2])/[3])**2)",__Sim_Lower_Limit__[peak],__Sim_Upper_Limit__[peak]);
        fit_dip2_sim->SetParameters(__Par_Sim_Dip2__[0], __Par_Sim_Dip2__[1], __Par_Sim_Dip2__[2], __Par_Sim_Dip2__[3]); //bg,amp,mean,sigma
        sim_fit->Fit(fit_dip2_sim, "RL");
        
        e.push_back(fit_dip2_sim->GetParameter(2));
        eERR.push_back(fit_dip2_sim->GetParError(2));
        
        peak++;
    }

    if(Peakstobefitted[3]){

        fit_dip3_sim = new TF1("dip3res", "[0] - [1]*exp(-0.5*((x-[2])/[3])**2)",__Sim_Lower_Limit__[peak],__Sim_Upper_Limit__[peak]);
        fit_dip3_sim->SetParameters(__Par_Sim_Dip3__[0], __Par_Sim_Dip3__[1], __Par_Sim_Dip3__[2], __Par_Sim_Dip3__[3]); //bg,amp,mean,sigma
        sim_fit->Fit(fit_dip3_sim, "RL");
        
        e.push_back(fit_dip3_sim->GetParameter(2));
        eERR.push_back(fit_dip3_sim->GetParError(2));
        
        peak++;
    }

    if(Peakstobefitted[4]){

        fit_res_sim = new TF1("ressim", "[0]/((x-[1])*(x-[1]) + 0.25*[2]*[2])",__Sim_Lower_Limit__[peak],__Sim_Upper_Limit__[peak]);
        fit_res_sim->SetParameters(__Par_Sim_Res__[0], __Par_Sim_Res__[1], __Par_Sim_Res__[2]); //amp,mean,sigma
        sim_fit->Fit(fit_res_sim, "RL");
        
        e.push_back(fit_res_sim->GetParameter(1));
        eERR.push_back(fit_res_sim->GetParError(1));

        peak++;        
    }

    for(int i = 0; i  < numberoffits; i++) std::cout << e[i] << std::endl;

    for(int i = 0; i < numberoffits; i++) std::cout << xp[i] << "  ->  " << e[i] << std::endl;

    //Fitting Sim ends

    //Fit flight path
    double tof[numberoffits], energy[numberoffits], errX[numberoffits], errY[numberoffits];
    for(int j = 0; j < numberoffits; j++){
        tof[j] = xp[j];
        energy[j] = e[j];
        errX[j] = xpERR[j];
        errY[j] = eERR[j];
    }

    TCanvas* cf = new TCanvas();
    const int npar=2;
    TGraph *gr1 = new TGraphErrors(numberoffits,tof,energy,errX,errY);

    TF1 *fitg = new TF1("f1", "522940.0*[0]*[0]/((x-[1])*(x-[1]))", 2000, 1.e7);
    fitg->SetParameters(1923, 0);

    gr1->Fit(fitg,"RML");
    __tof_energy_par__.push_back(fitg->GetParameter(0));
    __tof_energy_par__.push_back(fitg->GetParameter(1));
    std::cout << __tof_energy_par__[0] << "   " << __tof_energy_par__[1] << std::endl;
    gr1->SetMarkerSize(2.5);
    gr1->SetMarkerStyle(5);
    gr1->Write();

    gr1->Draw("AP");

    //Add Legend
    TLegend *legend = new TLegend(0.8,0.7,0.9,0.9);
    legend->SetHeader("","C");
    legend->AddEntry(tof_fit, "6Li(n,t)" ,"l");

    legend->Draw();

    //======================================================
    // Saving the experimental fit
    //======================================================

    __flight_path_file__->cd();

    tof_fit->SetName("Experimental_TOF_histogram");
    tof_fit->SetTitle("Experimental TOF with fits");

    TCanvas *cTOF = new TCanvas("cTOF","Experimental TOF");
    cTOF->SetLogx();
    cTOF->SetLogy();

    tof_fit->Draw("hist");

    if (fit_tp)   fit_tp->Draw("same");
    if (fit_dip1) fit_dip1->Draw("same");
    if (fit_dip2) fit_dip2->Draw("same");
    if (fit_dip3) fit_dip3->Draw("same");
    if (fit_res)  fit_res->Draw("same");

    tof_fit->Write();
    cTOF->Write();

    //======================================================
    // Saving the simulation fit
    //======================================================

    sim_fit->SetName("Simulation_histogram");
    sim_fit->SetTitle("Simulation with fits");

    TCanvas *cSIM = new TCanvas("cSIM","Simulation");

    cSIM->SetLogx();
    cSIM->SetLogy();

    sim_fit->Draw("hist");

    if (fit_tp_sim)   fit_tp_sim->Draw("same");
    if (fit_dip1_sim) fit_dip1_sim->Draw("same");
    if (fit_dip2_sim) fit_dip2_sim->Draw("same");
    if (fit_dip3_sim) fit_dip3_sim->Draw("same");
    if (fit_res_sim)  fit_res_sim->Draw("same");

    sim_fit->Write();
    cSIM->Write();

    //======================================================
    // Saving the experimental energy
    //======================================================

    energy_fit->SetName("Experimental_energy_histogram");
    energy_fit->SetTitle("Experimental energy");

    TCanvas *cenergy = new TCanvas("cenergy","Experimental energy");

    cenergy->SetLogx();
    cenergy->SetLogy();

    energy_fit->Rebin(5);

    double lower_energy = energy_fit->FindBin(1);
    double lower_sim    = sim_fit->FindBin(1);
    double upper_energy = energy_fit->FindBin(100);
    double upper_sim    = sim_fit->FindBin(100);
    double energy_integral = energy_fit->Integral(lower_energy, upper_energy);
    double sim_integral    = sim_fit->Integral(lower_sim, upper_sim);

    energy_fit->Scale(sim_integral / energy_integral / 5);

    energy_fit->Draw("hist");

    energy_fit->Write();
    cenergy->Write();

    //======================================================
    // Saving the Ratio between exp and sim
    //======================================================

    TH1D* Ratio = (TH1D*)energy_fit->Clone("Ratio_Exp_Sim");
    TH1D* Ratioaux = (TH1D*)sim_fit->Clone("");
    Ratioaux->SetDirectory(0);
    Ratio->SetDirectory(0);

    TCanvas *cRatio = new TCanvas("Ratio_Exp_Sim","Ratio");

    cRatio->SetLogx();
    cRatio->SetLogy();

    double expbin = energy_fit->GetXaxis()->GetNbins();
    double simbin = sim_fit->GetXaxis()->GetNbins();
    if(expbin == simbin){
        Ratio->Rebin(50);
        Ratioaux->Rebin(50);
        Ratio->Divide(Ratioaux);
    }else{
        Ratio = Rebinning(Ratio, Ratioaux);
        Ratio->Rebin(50);
        Ratioaux->Rebin(50);
        Ratio->Divide(Ratioaux);
    }

    Ratio->SetName("Ratio_Exp_Sim_Data");
    Ratio->SetTitle("Ratio Exp. Data / Sim. Data");
    Ratio->Write();

    Ratio->Draw("hist");

    cRatio->Write();

    //======================================================
    // Save flight path fit
    //======================================================

    TCanvas *cFlight = new TCanvas("cFlight", "Flight path calibration");

    gr1->Draw("AP");
    fitg->Draw("same");

    gr1->Write("Flight_path_graph");
    fitg->Write("Flight_path_function");

    cFlight->Write();

}

void FlightPathCalibration::flight_path_calibration_TC(){

    tof_fit->Rebin(2);
    TH1F *sim_fit = (TH1F*)__counting_rate__->Clone();
    sim_fit->SetDirectory(0);

    double lower_tof = tof_fit->FindBin(1e5);
    double lower_sim    = sim_fit->FindBin(1e5);
    double upper_tof = tof_fit->FindBin(1e6);
    double upper_sim    = sim_fit->FindBin(1e6);
    double tof_integral = tof_fit->Integral(lower_tof, upper_tof);
    double sim_integral    = sim_fit->Integral(lower_sim, upper_sim);

    tof_fit->Scale(sim_integral / tof_integral / 2);
    //tof_fit is related to the tof spectra
    //sim_fit is the simulated energy from lithium

    int peak = 0;

    //The first fitting is the thermal peak. Using a third degree polinom
    if(Peakstobefitted[0]){
        fit_tp = new TF1("par", "[0]*x*x*x + [1]*x*x + [2]*x + [3]", __Exp_Lower_Limit__[peak], __Exp_Upper_Limit__[peak]);
        fit_tp->SetParameters(__Par_Exp_Thermal__[0], __Par_Exp_Thermal__[1], __Par_Exp_Thermal__[2], __Par_Exp_Thermal__[3]);
        tof_fit->Fit(fit_tp, "R");

        double a = fit_tp->GetParameter(0);
        double b = fit_tp->GetParameter(1);
        double c = fit_tp->GetParameter(2);
        double d = fit_tp->GetParameter(3);

        double x1, x2;
        x2 = solve_quadratic(3*a, 2*b, c, x1, x2);

        // Check which root corresponds to a peak (maximum or minimum)
        double second_deriv_at_x1 = second_derivative(x1, a, b);
        double second_deriv_at_x2 = second_derivative(x2, a, b);

        xp.push_back(x2);
        xpERR.push_back(0.001*x2);

        peak++;
    }

    if(Peakstobefitted[1]){
        fit_dip1 = new TF1("dip1", "[0] + [4] * x - [1]*exp(-0.5*((x-[2])/[3])**2)",__Exp_Lower_Limit__[peak],__Exp_Upper_Limit__[peak]);
        fit_dip1->SetParameters(__Par_Exp_Dip1__[0], __Par_Exp_Dip1__[1], __Par_Exp_Dip1__[2], __Par_Exp_Dip1__[3], __Par_Exp_Dip1__[4]); //bg,amp,mean,sigma
        tof_fit->Fit(fit_dip1, "RL");

        xp.push_back(fit_dip1->GetParameter(2));
        xpERR.push_back(4*fit_dip1->GetParError(2));

        peak++;
    }

    if(Peakstobefitted[2]){
        fit_dip2 = new TF1("dip2", "[0] - [1]*exp(-0.5*((x-[2])/[3])**2)",__Exp_Lower_Limit__[peak],__Exp_Upper_Limit__[peak]);
        fit_dip2->SetParameters(__Par_Exp_Dip2__[0], __Par_Exp_Dip2__[1], __Par_Exp_Dip2__[2], __Par_Exp_Dip2__[3]); //bg,amp,mean,sigma
        tof_fit->Fit(fit_dip2, "R");  
        
        xp.push_back(fit_dip2->GetParameter(2));
        xpERR.push_back(4*fit_dip2->GetParError(2));

        peak++;
    }

    if(Peakstobefitted[3]){
        fit_dip3 = new TF1("dip3", "[0] - [1]*exp(-0.5*((x-[2])/[3])**2)",__Exp_Lower_Limit__[peak],__Exp_Upper_Limit__[peak]);
        fit_dip3->SetParameters(__Par_Exp_Dip3__[0], __Par_Exp_Dip3__[1], __Par_Exp_Dip3__[2], __Par_Exp_Dip3__[3]); //bg,amp,mean,sigma
        tof_fit->Fit(fit_dip3, "R");

        xp.push_back(fit_dip3->GetParameter(2));
        xpERR.push_back(4*fit_dip3->GetParError(2));

        peak++;
    }

    if(Peakstobefitted[4]){
        fit_res = new TF1("bw", "[0]/((x-[1])*(x-[1]) + 0.25*[2]*[2])",__Exp_Lower_Limit__[peak],__Exp_Upper_Limit__[peak]);
        fit_res->SetParameters(__Par_Exp_Res__[0], __Par_Exp_Res__[1], __Par_Exp_Res__[2]); //amp,mean,sigma
        fit_res->SetParNames("Amplitude", "Peak", "Width");
        tof_fit->Fit(fit_res, "R"); 
        
        xp.push_back(fit_res->GetParameter(1));
        xpERR.push_back(fit_res->GetParError(1));

        peak++;
    }

    //start fitting Simulation
    std::cout << "Starting the minimization of the Simulations" << std::endl;
    peak = 0;

    if(Peakstobefitted[0]){

        fit_tp_sim = new TF1("par_sim", "[0]*x*x*x + [1]*x*x + [2]*x + [3]",__Sim_Lower_Limit__[peak],__Sim_Upper_Limit__[peak]);
        fit_tp_sim->SetParameters(__Par_Sim_Thermal__[0], __Par_Sim_Thermal__[1], __Par_Sim_Thermal__[2], __Par_Sim_Thermal__[3]);
        sim_fit->Fit(fit_tp_sim, "RL");

        double as = fit_tp_sim->GetParameter(0);
        double bs = fit_tp_sim->GetParameter(1);
        double cs = fit_tp_sim->GetParameter(2);
        double ds = fit_tp_sim->GetParameter(3);

        std::cout << as << "   " << bs << "   " << cs << "   " << ds << std::endl;

        double x1sim = 0, x2sim = 0;
        x2sim = solve_quadratic(3*as, 2*bs, cs, x1sim, x2sim);

        // Check which root corresponds to a peak (maximum or minimum)
        double second_deriv_at_x1s = second_derivative(x1sim, as, bs);
        double second_deriv_at_x2s = second_derivative(x2sim, as, bs);

        e.push_back(x2sim);
        eERR.push_back(0.001*x2sim);

        peak++;
    }

    if(Peakstobefitted[1]){
        fit_dip1_sim = new TF1("dip1res", "[0] + [4] * x - [1]*exp(-0.5*((x-[2])/[3])**2)",__Sim_Lower_Limit__[peak],__Sim_Upper_Limit__[peak]);
        fit_dip1_sim->SetParameters(__Par_Sim_Dip1__[0], __Par_Sim_Dip1__[1], __Par_Sim_Dip1__[2], __Par_Sim_Dip1__[3], __Par_Sim_Dip1__[4]); //bg,amp,mean,sigma
        sim_fit->Fit(fit_dip1_sim, "RL");
        
        e.push_back(fit_dip1_sim->GetParameter(2));
        eERR.push_back(fit_dip1_sim->GetParError(2));

        peak++;
    }

    if(Peakstobefitted[2]){
        fit_dip2_sim = new TF1("dip2res", "[0] - [1]*exp(-0.5*((x-[2])/[3])**2)",__Sim_Lower_Limit__[peak],__Sim_Upper_Limit__[peak]);
        fit_dip2_sim->SetParameters(__Par_Sim_Dip2__[0], __Par_Sim_Dip2__[1], __Par_Sim_Dip2__[2], __Par_Sim_Dip2__[3]); //bg,amp,mean,sigma
        sim_fit->Fit(fit_dip2_sim, "RL");
        
        e.push_back(fit_dip2_sim->GetParameter(2));
        eERR.push_back(fit_dip2_sim->GetParError(2)); 
        
        peak++;
    }

    if(Peakstobefitted[3]){
        fit_dip3_sim = new TF1("dip3res", "[0] - [1]*exp(-0.5*((x-[2])/[3])**2)",__Sim_Lower_Limit__[peak],__Sim_Upper_Limit__[peak]);
        fit_dip3_sim->SetParameters(__Par_Sim_Dip3__[0], __Par_Sim_Dip3__[1], __Par_Sim_Dip3__[2], __Par_Sim_Dip3__[3]); //bg,amp,mean,sigma
        sim_fit->Fit(fit_dip3_sim, "RL");
        
        e.push_back(fit_dip3_sim->GetParameter(2));
        eERR.push_back(fit_dip3_sim->GetParError(2)); 
        
        peak++;
    }

    if(Peakstobefitted[4]){
        fit_res_sim = new TF1("ressim", "[0]/((x-[1])*(x-[1]) + 0.25*[2]*[2])",__Sim_Lower_Limit__[peak],__Sim_Upper_Limit__[peak]);
        fit_res_sim->SetParameters(__Par_Sim_Res__[0], __Par_Sim_Res__[1], __Par_Sim_Res__[2]); //amp,mean,sigma
        //fit_res_sim->SetParNames("Amplitude", "Peak", "Width");
        sim_fit->Fit(fit_res_sim, "RL");
        
        e.push_back(fit_res_sim->GetParameter(1));
        eERR.push_back(fit_res_sim->GetParError(1));
        
        peak++;
    }

    for(int i = 0; i < numberoffits; i++) std::cout << xp[i] << "  ->  " << e[i] << std::endl;

    //Fit flight path
    double tof[numberoffits], energy[numberoffits], errX[numberoffits], errY[numberoffits];
    for(int j = 0; j < numberoffits; j++){
        tof[j] = xp[j];
        energy[j] = e[j];
        errX[j] = xpERR[j];
        errY[j] = eERR[j];
    }

    //======================================================
    // Starting minimization
    //======================================================

    FlightPathCalibration::current_instance = this;

    int __npar__ = 2;
    double par[__npar__], parErr[__npar__];
    TMinuit *minuit = new TMinuit(__npar__);
    minuit->SetFCN(FlightPathCalibration::logLikelihood);

    minuit->DefineParameter(0, "TOF", 330.475, 20, -1000.0, 1000.0); // p0: Offset
    minuit->DefineParameter(1, "L", 0.974883, 0.01, 0.90, 1.1); // p1: Linear term
        
    //minuit->SetErrorDef(100); //Fix precision to 1e-2
    minuit->Migrad();

    // Get the fitted parameters
    for (int i = 0; i < __npar__; ++i) {
      minuit->GetParameter(i, par[i], parErr[i]);
    }

    // Print the results
    std::cout << "Fitted parameters for the energy calibration:" << std::endl;
    std::cout << "p0 = " << par[0] << " +/- " << parErr[0] << std::endl;
    std::cout << "p1 = " << par[1] << " +/- " << parErr[1] << std::endl;

    for(int i = 0; i < numberoffits; i++) std::cout << xp[i] + par[0] << "  ->  " << e[i] * par[1] << std::endl;

    //======================================================
    // Saving the experimental fit
    //======================================================

    __flight_path_file__->cd();

    tof_fit->SetName("Experimental_TOF_histogram");
    tof_fit->SetTitle("Experimental TOF with fits");

    TCanvas *cTOF = new TCanvas("Experimental_TOF","Experimental TOF");
    cTOF->SetLogx();
    cTOF->SetLogy();

    tof_fit->Draw("hist");

    if (fit_tp)   fit_tp->Draw("same");
    if (fit_dip1) fit_dip1->Draw("same");
    if (fit_dip2) fit_dip2->Draw("same");
    if (fit_dip3) fit_dip3->Draw("same");
    if (fit_res)  fit_res->Draw("same");

    tof_fit->Write();
    cTOF->Write();

    //======================================================
    // Saving the simulation fit
    //======================================================

    sim_fit->SetName("Simulated_Energy_histogram");
    sim_fit->SetTitle("Simulation with fits");

    TCanvas *cSIM = new TCanvas("Energy_SIM","Simulation");

    cSIM->SetLogx();
    cSIM->SetLogy();

    sim_fit->Draw("hist");

    if (fit_tp_sim)   fit_tp_sim->Draw("same");
    if (fit_dip1_sim) fit_dip1_sim->Draw("same");
    if (fit_dip2_sim) fit_dip2_sim->Draw("same");
    if (fit_dip3_sim) fit_dip3_sim->Draw("same");
    if (fit_res_sim)  fit_res_sim->Draw("same");

    sim_fit->Write();
    cSIM->Write();

    //======================================================
    // Saving the Ratio between exp and sim
    //======================================================

    TH1D* Ratio = (TH1D*)tof_fit->Clone("Ratio_Exp_Sim");
    TH1D* Ratioaux = (TH1D*)sim_fit->Clone("");
    Ratioaux->SetDirectory(0);
    Ratio->SetDirectory(0);

    TCanvas *cRatio = new TCanvas("Ratio_Exp_Sim","Ratio_Exp_Sim");

    cRatio->SetLogx();
    cRatio->SetLogy();

    double expbin = tof_fit->GetXaxis()->GetNbins();
    double simbin = sim_fit->GetXaxis()->GetNbins();
    if(expbin == simbin){
        Ratio->Rebin(50);
        Ratioaux->Rebin(50);
        Ratio->Divide(Ratioaux);
    }else{
        Ratio = Rebinning(Ratio, Ratioaux);
        Ratio->Rebin(50);
        Ratioaux->Rebin(50);
        Ratio->Divide(Ratioaux);
    }

    Ratio->SetName("Ratio_Exp_Sim_Data");
    Ratio->SetTitle("Ratio Exp. Data / Sim. Data");
    Ratio->Write();

    Ratio->Draw("hist");

    cRatio->Write();
}

void FlightPathCalibration::logLikelihood(int &npar, double *grad, double &fval, double *par, int iflag) {
    // Parameters:
    // par[0], par[1], par[2]: Energy calibration (p0, p1, p2)
    // par[3], par[4]: Resolution parameters (a, b, c)
    // par[5]: Normalization value

    std::vector<double> means_sim, means_exp;

    means_sim = current_instance->e;
    means_exp = current_instance->xp;

    int numberfit;

    numberfit = current_instance->numberoffits;

    //Here we applied a preliminary parameters to convolve the simulated spectra
    for(int k = 0; k < numberfit; k++){
        means_sim[k] = means_sim[k]*par[1];
    }

    for(int j = 0; j < numberfit; j++){
        means_exp[j] = means_exp[j] + par[0];
    }


    // Compute the log-likelihood
    double logL = 0.0;
    fval = 0;
    for(int i = 0; i < numberfit; i++){

        double obs = means_sim[i];
        double exp = means_exp[i];
        double ratio = obs / exp;
        
        fval += std::abs(1 - ratio);

    }

    std::cout << "    p0 " << "   " << "      p1 " << "Minimization factor" << std::endl;
    std::cout << par[0] << "   " << par[1] << "   " << fval << std::endl;

  
    // Minimize -2*log(L)

    means_sim.clear();
    means_exp.clear();

}
