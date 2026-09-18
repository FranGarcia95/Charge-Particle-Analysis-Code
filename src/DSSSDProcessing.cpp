#include "DSSSDProcessing.h" 
#include "CoincidenceClass.h"

//std::string is from root and you can use more options implemented that by using only string
DSSSDProcessing::DSSSDProcessing(DetectorClass* det, UserInput *u){

    Detector = det;
    userInp = u;

}

DSSSDProcessing::~DSSSDProcessing(){}

void DSSSDProcessing::Inizialization(){

	__Root_Files__ = userInp->Get_Root_Files(); 
    __DetType__ = userInp->Get_One_Detector();
    __RunList__ = userInp->Get_Run_List();
    __NumberDet__ = userInp->Get_Number_Detectors();
    __Number_Strips__ = userInp->Get_Number_Strips();
    __Shift_Factor__ = userInp->Get_Shift_Factor();

    __pulse_type__ = {"Dedicated", "Parasitic"};
        
}

void DSSSDProcessing::Clear(){

    __RunFiles__.clear();

}

void DSSSDProcessing::Processing(std::string Processing){

    if(Processing == "DeadTime"){

       __RunFiles__ = Detector->Read_RunFile(__RunList__[0]);
        DeadTime_Processing(__Root_Files__[0]);

    }else if(Processing == "Coincidence"){
    
        __RunFiles__ = Detector->Read_RunFile(__RunList__[0]);
        if(__Number_Strips__ == 16){

            Coincidence_Processing_Strips(Processing, __Root_Files__[0]);

        }else if(__NumberDet__ == 1 && __Number_Strips__ == 32){

            Coincidence_Processing_FrontBack(Processing, __Root_Files__[0]);

        }else if(__NumberDet__ == 2 && __Number_Strips__ == 32){

            Coincidence_Processing_Forward_Backward(Processing, __Root_Files__[0]);

        }

    }else if(Processing == "FlightPath" || Processing == "DataProcessing"){
    
        __RunFiles__ = Detector->Read_RunFile(__RunList__[0]);
        Data_Processing(Processing, __Root_Files__[0]);
        Detector->NormHistogram();
        if(userInp->Get_Pile_Up()){
            Detector->PileUpCorrection();
        }

    }else if(Processing == "Calibration"){
        for(int i = 0; i < __RunList__.size(); i++){
            __RunFiles__.clear();
            __RunFiles__ = Detector->Read_RunFile(__RunList__[i]);
            Data_Processing(Processing, __Root_Files__[i], i);
        }
    }else if(Processing == "Stability"){
        __RunFiles__ = Detector->Read_RunFile(__RunList__[0]);
        Stability_Check(__Root_Files__[0]);
    }else{
        
        std::cout << "Not processing specific " << std::endl;
        std::cout << "Please, specify the processing you want to do" << std::endl;
        exit(-1);

    }

}

void DSSSDProcessing::Data_Processing(std::string Processing, std::string Rootfiles, int samplenumber){

    //Definition of the chain

    float entries = 0, entriespkup = 0;
    if(__DetType__ == "DEED"){
        SILI = Chain_Def("DEED", Rootfiles);
        entries = SILI->GetEntries();
    }else if(__DetType__ == "EDET"){
        SILI = Chain_Def("EDET", Rootfiles);
        entries = SILI->GetEntries();
    } 

    entriespkup = PKUP->GetEntries();

    std::int32_t aux=0, aux_det = 0;

    int step = std::max(1, static_cast<int>(entries / 10));
    int pkupaux = 0;
    int Bunch_pkup = 0;
    double tof_pkup = 0;

    for(int irow = 0; irow < entries; irow++){

        SILI->GetEntry(irow);

        if (BunchNumber != Bunch_pkup) {
            for (int jrow = pkupaux; jrow < entriespkup; ++jrow) {
                PKUP->GetEntry(jrow);

                Bunch_pkup = BunchNumber_PKUP;
                tof_pkup   = tof_PKUP;
                pkupaux++;

                break;   // stop after one PKUP entry
            }
        }

        if (irow % step == 0) {
            std::cout << "For the Detector: " << irow * 100 / entries << " %\r processed..." << std::endl;
        } 

        /*if((tof - tof_pkup + userInp->Get_TOF_Offset()) > 10800 &&  (tof - tof_pkup + userInp->Get_TOF_Offset()) < 11800){
            if(amp >= 750 && amp < 1000 && PulseIntensity > 7e12){
                std::cout << amp << "   " << tof << "   " << detn << "   " << segment << "   " << event << "   " << BunchNumber << "   " << RunNumber << std::endl;
            }
        }*/
  
        if(Processing == "Calibration"){
            Detector->CalibrationProcessing(amp, tof * __Shift_Factor__, PulseIntensity, detn, tof_pkup, samplenumber);
        }else if(Processing == "DataProcessing" || Processing == "FlightPath"){
            Detector->FillingHistogramProcessing(amp, tof * __Shift_Factor__, PulseIntensity, detn, BunchNumber, tof_pkup);
        }
    }

    std::cout << " 100% processed" << std::endl;
    
}

void DSSSDProcessing::Stability_Check(std::string Rootfiles){

    //Definition of the chain

    float entries = 0, entriespkup = 0;
    if(__DetType__ == "DEED"){
        SILI = Chain_Def("DEED", Rootfiles);
        entries = SILI->GetEntries();
    }else if(__DetType__ == "EDET"){
        SILI = Chain_Def("EDET", Rootfiles);
        entries = SILI->GetEntries();
    } 

    entriespkup = PKUP->GetEntries();

    std::int32_t aux=0, aux_det = 0;

    int step = std::max(1, static_cast<int>(entries / 10));
    int pkupaux = 0, Bunch_pkup = 0;
    float area_pkup = 0;
    double tof_pkup = 0;

    std::cout << "Show number of entries: " << entries << std::endl;
    const Long64_t nEntries = entries;
    for(Long64_t irow = 0; irow < nEntries; irow++){

        SILI->GetEntry(irow);

        if (BunchNumber != Bunch_pkup) {
            for (int jrow = pkupaux; jrow < entriespkup; ++jrow) {
                PKUP->GetEntry(jrow);

                Bunch_pkup = BunchNumber_PKUP;
                tof_pkup   = tof_PKUP;
                area_pkup  = area_PKUP;
                pkupaux++;

                break;   // stop after one PKUP entry
            }
        }

        if (irow % step == 0) {
            std::cout << irow * 100 / nEntries + 1 << "% processed" << std::endl;
        } 

        if(irow == entries - 1){
            std::cout << "We are here" << std::endl;
            Detector->StabilityProcessing(amp, tof * __Shift_Factor__, PulseIntensity, detn, RunNumber, BunchNumber, tof_pkup, area_pkup, true);
        }else{
            Detector->StabilityProcessing(amp, tof * __Shift_Factor__, PulseIntensity, detn, RunNumber, BunchNumber, tof_pkup, area_pkup, false);
        }

    }

    std::cout << " 100% processed" << std::endl;
    
}

void DSSSDProcessing::Coincidence_Processing_Forward_Backward(std::string Processing, std::string Rootfiles){

    DSSSD = Chain_Def(__DetType__.c_str(), Rootfiles);
    SSD = Chain_SSD("DEED", Rootfiles);

    Long64_t entries_DEED = DSSSD->GetEntries();
    Long64_t entries_SSD = SSD->GetEntries();
    Long64_t entriespkup = PKUP->GetEntries();

    int pkupaux = 0;
    int Bunch_pkup = 0;
    double tof_pkup = 0;

    Long64_t forwardcounter = 0;      
    Long64_t frontcounter   = 0;
    Long64_t backcounter    = -1;

    bool back_initialized = false;

    int currentRun = -1;
    bool startSSD = false;

   while(frontcounter < entries_DEED){

        // Read current front event
        DSSSD->GetEntry(frontcounter);

        int run = RunNumber;
        int bunch = BunchNumber;


        // New run detected
        if(run != currentRun){

            currentRun = run;
            back_initialized = false;
            backcounter = -1;

        }

        //Take PKUPTOF
        if (BunchNumber != Bunch_pkup) {
            while(pkupaux < entriespkup){

                PKUP->GetEntry(pkupaux);

                Bunch_pkup = BunchNumber_PKUP;
                tof_pkup   = tof_PKUP;
                pkupaux++;

                break;   // stop after one PKUP entry
            }
        }

        // -------------------------------------
        // Find beginning of back detectors
        // only once per run
        // -------------------------------------

        if(!back_initialized){

            Long64_t search = frontcounter;


            while(search < entries_DEED)
            {

                DSSSD->GetEntry(search);


                if(RunNumber != currentRun)
                    break;


                if(detn > 16)
                {

                    backcounter = search;
                    back_initialized = true;

                    break;

                }


                search++;

            }


            if(backcounter < 0)
            {
                std::cout << "Back detectors not found for run "
                          << currentRun << std::endl;

                break;
            }

        }

        // -----------------------------
        // FRONT
        // -----------------------------

        while(frontcounter < entries_DEED)
        {

            DSSSD->GetEntry(frontcounter);

            if(BunchNumber != bunch || RunNumber != run){
                break;
            } 

            if(detn <= 16){
                CoincidenceAnalysis->Hits(amp, tof * __Shift_Factor__, detn, BunchNumber, PulseIntensity, "DSSSD", false, tof_pkup);
            }

            frontcounter++;
        }

        // -----------------------------
        // BACK
        // -----------------------------

        bool back_hit_found = false;

        while(backcounter < entries_DEED){

            DSSSD->GetEntry(backcounter);

            if(BunchNumber != bunch){
                break;
            }else if(RunNumber != run){
                frontcounter = backcounter;
                break;
            }

            if(detn > 16){

                back_hit_found = true;

                // Check if this is the last hit of the bunch
                startSSD = false;

                if(backcounter + 1 >= entries_DEED){

                    startSSD = true;

                }else{
                    DSSSD->GetEntry(backcounter + 1);

                    if(BunchNumber != bunch || RunNumber != run) startSSD = true;

                    DSSSD->GetEntry(backcounter);

                    CoincidenceAnalysis->Hits(amp, tof * __Shift_Factor__, detn, BunchNumber, PulseIntensity, "DSSSD", false, tof_pkup);
                }

            }

            backcounter++;

        }

        // -----------------------------
        // FORWARD
        // -----------------------------

        while (forwardcounter < entries_SSD && startSSD){

            SSD->GetEntry(forwardcounter);

            if(BunchNumber_SSD != bunch || RunNumber_SSD != run){
                break;
            }

            // Check if this is the last hit of the bunch
            bool last_event = false;

            if(forwardcounter + 1 >= entries_SSD){

                last_event = true;

            }else{

                SSD->GetEntry(forwardcounter + 1);

                if(BunchNumber_SSD != bunch || RunNumber_SSD != run){

                    last_event = true;
                    startSSD  = false;

                } 

                SSD->GetEntry(forwardcounter);

            }

            forwardcounter++;

            CoincidenceAnalysis->Hits(amp_SSD, tof_SSD * __Shift_Factor__, detn_SSD, BunchNumber_SSD, PulseIntensity_SSD, "SSD", last_event, tof_pkup);

        }
        

    }


    std::cout << "100% processed" << std::endl;

}

void DSSSDProcessing::SetCoincidenceClass(CoincidenceClass* coincidence){

    CoincidenceAnalysis = coincidence;

}

void DSSSDProcessing::Coincidence_Processing_FrontBack(std::string Processing, std::string Rootfiles){

    DSSSD = Chain_Def(__DetType__.c_str(), Rootfiles);

    Long64_t entries_DEED = DSSSD->GetEntries();
    Long64_t entriespkup = PKUP->GetEntries();

    int pkupaux = 0;
    int Bunch_pkup = 0;
    double tof_pkup = 0;

    Long64_t frontcounter = 0;
    Long64_t backcounter  = -1;

    bool back_initialized = false;

    int currentRun = -1;

   while(frontcounter < entries_DEED){

        // Read current front event
        DSSSD->GetEntry(frontcounter);

        int run = RunNumber;
        int bunch = BunchNumber;

        //Take PKUPTOF
        if (BunchNumber != Bunch_pkup) {
            while(pkupaux < entriespkup){

                PKUP->GetEntry(pkupaux);

                Bunch_pkup = BunchNumber_PKUP;
                tof_pkup   = tof_PKUP;
                pkupaux++;

                break;   // stop after one PKUP entry
            }
        }

        // New run detected
        if(run != currentRun){

            currentRun = run;
            back_initialized = false;
            backcounter = -1;

        }

        // -------------------------------------
        // Find beginning of back detectors
        // only once per run
        // -------------------------------------

        if(!back_initialized){

            Long64_t search = frontcounter;


            while(search < entries_DEED)
            {

                DSSSD->GetEntry(search);


                if(RunNumber != currentRun)
                    break;


                if(detn > 16)
                {

                    backcounter = search;
                    back_initialized = true;

                    break;

                }


                search++;

            }


            if(backcounter < 0)
            {
                std::cout << "Back detectors not found for run "
                          << currentRun << std::endl;

                break;
            }

        }

        // -----------------------------
        // FRONT
        // -----------------------------

        while(frontcounter < entries_DEED)
        {

            DSSSD->GetEntry(frontcounter);

            if(BunchNumber != bunch || RunNumber != run) break;

            if(detn <= 16){
                CoincidenceAnalysis->Hits(amp, tof * __Shift_Factor__, detn, BunchNumber, PulseIntensity, "DEED", false, tof_pkup);
            }

            frontcounter++;
        }

        // -----------------------------
        // BACK
        // -----------------------------

        bool back_hit_found = false;

        while(backcounter < entries_DEED){

            DSSSD->GetEntry(backcounter);

            if(BunchNumber != bunch){
                break;
            }else if(RunNumber != run){
                frontcounter = backcounter;
                break;
            }

            if(detn > 16){

                back_hit_found = true;

                // Check if this is the last hit of the bunch
                bool last_hit = false;

                if(backcounter + 1 >= entries_DEED){

                    last_hit = true;

                }else{
                    DSSSD->GetEntry(backcounter + 1);

                    if(BunchNumber != bunch || RunNumber != run) last_hit = true;

                    DSSSD->GetEntry(backcounter);

                }

                CoincidenceAnalysis->Hits(amp, tof * __Shift_Factor__, detn, BunchNumber, PulseIntensity, "DEED", last_hit, tof_pkup);

            }

            backcounter++;

        }

    }

    std::cout << "100% processed" << std::endl;

}

void DSSSDProcessing::Coincidence_Processing_Strips(std::string Processing, std::string Rootfiles){

    //Definition of the chain

    float entries_DEED = 0, entries_EDET;
    SSD = Chain_Def(__DetType__.c_str(), Rootfiles);
    entries_DEED = SSD->GetEntries();

    Long64_t entriespkup = PKUP->GetEntries();

    int pkupaux = 0;
    int Bunch_pkup = 0;
    double tof_pkup = 0;

    int deed_bunch=0, aux_bunch = 0;

    SSD->GetEntry(0);

    deed_bunch = BunchNumber;

    for(int irow = 0; irow < entries_DEED; irow++){


        if(irow < (entries_DEED - 1)){

            SSD->GetEntry(irow + 1);
            aux_bunch = BunchNumber;
        
        }

        SSD->GetEntry(irow);

        //Take PKUPTOF
        if (BunchNumber != Bunch_pkup) {
            while(pkupaux < entriespkup){

                PKUP->GetEntry(pkupaux);

                Bunch_pkup = BunchNumber_PKUP;
                tof_pkup   = tof_PKUP;
                pkupaux++;

                break;   // stop after one PKUP entry
            }
        }

        if (aux_bunch != deed_bunch || irow == (entries_DEED - 1)) {

            CoincidenceAnalysis->Hits(amp, tof * __Shift_Factor__, detn, BunchNumber, PulseIntensity, "DEED", true, tof_pkup);
            deed_bunch = aux_bunch;

        }else{
            CoincidenceAnalysis->Hits(amp, tof * __Shift_Factor__, detn, BunchNumber, PulseIntensity, "DEED", false, tof_pkup);
        }

    }

    std::cout << " 100% processed" << std::endl;
    
}

void DSSSDProcessing::DeadTime_Processing(std::string Rootfiles){

    //Definition of the chain

    float entries = 0, entriespkup = 0;
    if(__DetType__ == "DEED"){
        SILI = Chain_Def("DEED", Rootfiles);
        entries = SILI->GetEntries();
    }else if(__DetType__ == "EDET"){
        SILI = Chain_Def("EDET", Rootfiles);
        entries = SILI->GetEntries();
    } 

    entriespkup = PKUP->GetEntries();

    double tof1, tof2;
    float amp1, amp2;
    int bunch1, bunch2, detn1, detn2;
    float Intensity;
    int detnumber;

    int pkupaux = 0;
    int Bunch_pkup = 0;
    double tof_pkup = 0;

    int step = std::max(1, static_cast<int>(entries / 10));

    for(int irow = 0; irow < entries; irow++){

        if (irow % step == 0) {
            std::cout << "For the Detector: " << irow * 100 / entries << " %\r processed..." << std::endl;
        } 

        SILI->GetEntry(irow);

        tof1 = tof;
        amp1 = amp;
        bunch1 = BunchNumber;
        detn1 = detn;
        Intensity = PulseIntensity;

        if (BunchNumber != Bunch_pkup) {
            for (int jrow = pkupaux; jrow < entriespkup; ++jrow) {
                PKUP->GetEntry(jrow);

                Bunch_pkup = BunchNumber_PKUP;
                tof_pkup   = tof_PKUP;
                pkupaux++;

                break;   // stop after one PKUP entry
            }
        }

        SILI->GetEntry(irow + 1);

        tof2 = tof;
        amp2 = amp;
        bunch2 = BunchNumber;
        detn2 = detn;

        if(Intensity > 7e12 && bunch1 == bunch2 && detn1 == detn2){
            Detector->DeadTimeCorrection(amp1, amp2, tof1, tof2, detn, tof_pkup);
        }

    }

    std::cout << " 100% processed" << std::endl;
    
}

TChain* DSSSDProcessing::Chain_Def(const char* ftree_name, std::string Rootfiles){

    TChain* ftree = new TChain(ftree_name);  // Dynamically allocate TChain
    PKUP = new TChain("PKUP");
    
    for (auto run : __RunFiles__) {
            std::cout << Rootfiles << std::endl;
            std::string filename = std::string(Rootfiles) + "run" + run + ".root";

            std::ifstream file(filename);
            if (!file) {
                std::cerr << "Error: File does not exist or cannot be opened: " << filename << std::endl;
                continue;  
            }

            ftree->Add(filename.c_str());
            PKUP->Add(filename.c_str());
            std::cout << "Added file: " << filename << std::endl;
    }

    std::cout << "Entries in TChain for SSD or DSSSD: " << ftree->GetEntries() << std::endl;
    std::cout << "Entries in TChain for PKUP: " << PKUP->GetEntries() << std::endl;
    if (ftree->GetEntries() == 0 || PKUP->GetEntries() == 0) {
        std::cerr << "No entries found in the TChain " << ftree_name << " or " << "PKUP. Check if files exist and are valid." << std::endl;
        delete ftree;  // Prevent memory leak
        delete PKUP;
        return nullptr;
    }

    // Set branch addresses
    ftree->SetBranchAddress("event", &event);
    ftree->SetBranchAddress("segment", &segment);
    ftree->SetBranchAddress("area", &area);
    ftree->SetBranchAddress("amp", &amp);
    ftree->SetBranchAddress("detn", &detn);
    ftree->SetBranchAddress("tof", &tof);
    ftree->SetBranchAddress("tflash", &tflash);
    ftree->SetBranchAddress("risetime", &risetime);
    ftree->SetBranchAddress("BunchNumber", &BunchNumber);
    ftree->SetBranchAddress("RunNumber", &RunNumber);
    ftree->SetBranchAddress("PSpulse", &PSpulse);
    ftree->SetBranchAddress("PulseIntensity", &PulseIntensity);

    PKUP->SetBranchAddress("amp", &amp_PKUP);
    PKUP->SetBranchAddress("area", &area_PKUP);
    PKUP->SetBranchAddress("tof", &tof_PKUP);
    PKUP->SetBranchAddress("tflash", &tflash_PKUP);
    PKUP->SetBranchAddress("BunchNumber", &BunchNumber_PKUP);
    PKUP->SetBranchAddress("RunNumber", &RunNumber_PKUP);
    PKUP->SetBranchAddress("PulseIntensity", &PulseIntensity_PKUP);

    // Try accessing the first entry
    ftree->GetEntry(0);
    PKUP->GetEntry(0);

    return ftree;  // Return the dynamically created TChain
}

TChain* DSSSDProcessing::Chain_SSD(const char* ftree_name, std::string Rootfiles){

    TChain* ftree = new TChain(ftree_name);  // Dynamically allocate TChain
    
    for (auto run : __RunFiles__) {
            std::cout << Rootfiles << std::endl;
            std::string filename = std::string(Rootfiles) + "run" + run + ".root";

            std::ifstream file(filename);
            if (!file) {
                std::cerr << "Error: File does not exist or cannot be opened: " << filename << std::endl;
                continue;  
            }

            ftree->Add(filename.c_str());
            std::cout << "Added file: " << filename << std::endl;
    }

    std::cout << "Entries in TChain: " << ftree->GetEntries() << std::endl;
    if (ftree->GetEntries() == 0) {
        std::cerr << "No entries found in the TChain. Check if files exist and are valid." << std::endl;
        delete ftree;  // Prevent memory leak
        return nullptr;
    }

    // Set branch addresses
    ftree->SetBranchAddress("event", &event_SSD);
    ftree->SetBranchAddress("segment", &segment_SSD);
    ftree->SetBranchAddress("area", &area_SSD);
    ftree->SetBranchAddress("amp", &amp_SSD);
    ftree->SetBranchAddress("detn", &detn_SSD);
    ftree->SetBranchAddress("tof", &tof_SSD);
    ftree->SetBranchAddress("tflash", &tflash_SSD);
    ftree->SetBranchAddress("risetime", &risetime_SSD);
    ftree->SetBranchAddress("BunchNumber", &BunchNumber_SSD);
    ftree->SetBranchAddress("RunNumber", &RunNumber_SSD);
    ftree->SetBranchAddress("PSpulse", &PSpulse_SSD);
    ftree->SetBranchAddress("PulseIntensity", &PulseIntensity_SSD);

    // Try accessing the first entry
    ftree->GetEntry(0);

    return ftree;  // Return the dynamically created TChain
}



