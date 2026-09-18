#include <iostream>
#include <string>
#include "UserInput.h"
#include "DetectorClass.h"
#include "DSSSDProcessing.h" 
#include "DetectorCalibration.h" 
#include "FlightPathCalibration.h" 
#include "CoincidenceClass.h" 

//g++ myDSSSD.cpp DSSSDProcessing.cpp DetectorClass.cpp DetectorCalibration.cpp FlightPathCalibration.cpp CoincidenceClass.cpp UserInput.cpp -o myDSSSD `root-config --cflags --glibs` -lm -ldl -lMinuit

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrintUsage() {
    std::cerr << " Usage: " << std::endl;
    std::cerr << " NoOp [-i inputFileName ] " << std::endl;
    std::cerr << " correct argument not provided "
        << std::endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc, char** argv)
{

    //Argument evaluation
    if (argc > 3) {
        PrintUsage();
        return 1;
    }

    char *configFileName = (char *)"Error";
    for (int i = 1; i < argc; i = i + 2) {
        if (std::string(argv[i]) == "-i")
            configFileName = argv[i + 1];
        else {
            PrintUsage();
            return 1;
        }
    }

    UserInput *myuserinp  = new UserInput(configFileName);

    DetectorClass *myDetector  = new DetectorClass(myuserinp);
    myDetector->Inizialization();

    DSSSDProcessing *myDSSSD  = new DSSSDProcessing(myDetector, myuserinp);

    std::string Processing = myuserinp->Get_Processing();

    //We link the Processing and the Coincidence Class, defining the object first in the class
    //And then passing to the class the full object   

    if(Processing == "DataProcessing"){
        myDSSSD->Inizialization();
        myDSSSD->Processing(Processing);
    }else if(Processing == "Calibration"){
        DetectorCalibration *myCalibration  = new DetectorCalibration(myDetector, myuserinp);
        myDSSSD->Inizialization();
        myDSSSD->Processing(Processing);
        myCalibration->Inizialization();
        myCalibration->Processing();
        delete myCalibration;
    }else if(Processing == "DeadTime"){
        myDSSSD->Inizialization();
        myDSSSD->Processing(Processing);
    }else if(Processing == "FlightPath"){
        FlightPathCalibration *myFlightPath  = new FlightPathCalibration(myDetector, myuserinp);
        if(myuserinp->Get_Histo_From_File()){
            myFlightPath->Inizialization();
            myFlightPath->Processing();
        }else{
            myDSSSD->Inizialization();
            myDSSSD->Processing(Processing);
            myFlightPath->Inizialization();
            myFlightPath->Processing();
        }
        delete myFlightPath;
    }else if(Processing == "Stability"){
        myDSSSD->Inizialization();
        myDSSSD->Processing(Processing);
    }else if(Processing == "Coincidence"){
        CoincidenceClass *myCoincidence = new CoincidenceClass(myDetector, myuserinp);
        myDSSSD->SetCoincidenceClass(myCoincidence);
        myCoincidence->Inizialization();
        myDSSSD->Inizialization();
        myDSSSD->Processing(Processing);
        delete myCoincidence;
    }

    delete myuserinp;
    delete myDetector;
    delete myDSSSD;

}