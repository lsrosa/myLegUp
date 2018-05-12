/*
 * File:   main.cpp
 * Author: nazanin
 *
 * Created on June 16, 2013, 2:21 PM
 */

#include <QtGui/QApplication>
#include <QWidget>
#include <iostream>
#include <pthread.h>
#include <vector>
#include <sys/param.h>
#include <fstream>

#include "formMain.h"
#include "OnChipDebugEngine.h"
#include "GDBWrapper.h"
#include "startForm.h"

#include "TCPClient.h"
#include "ModelsimStarter.h"
#include "Utility.h"
//#include "Globals.h"

//*********************** Global Variables ***********************//

#ifdef TCPCLIENT_H
TCPClient tcpc("127.0.0.1", "2000");
#endif
std::string modelsimListenerFilename = "ModelsimListener.tcl";
std::string initilizeDesignTclFileName = "init.tcl";
std::string vsimRunCommand = "vsim -c -do ";

//these variables will be set from the Inspect.config file
std::string vsimDir, workDir, legUpDir, fileName, alteraMFLibPath;
std::string dbHost, dbUser, dbPass, dbName;
std::vector<std::string> alteraFPPaths;

//these variable values willl be set in initialization (based on the fileName variable)
std::string designFilename, rawSourceFilename, codeFilename, SWBinaryFilename, stpFilename, csvFileName;

std::string referenceSimulationDataFilename = "refsim.dat";
std::string referenceSimulationCyclesToPathsFileName = "refsim_cycles_to_paths.dat";
std::string onchipBugLogFilename = "onchip_bugs.log";

bool runOnChip;
bool writeOnChipDebugInfoOnFile;
std::string onChipDebugInfoFileAddress = "onChipDebugInfo.dat";
std::string statesToCyclesFileAddress;
#ifdef DEBUGENGINE_H
OnChipDebugEngine *dbgEngine;
#endif

std::string nodeNamesFilename = "nodenames.txt";
std::string deviceInfoFileName = "deviceinfo.txt";

//debug make script files
std::string dbgMakeFilePath = "dbgMake.sh";
std::string increamentalDebugMakeFilePath = "incrementalDebug.sh";


#ifdef DATAACCESS_H
DataAccess *DA;
#endif
#ifdef IRINSTRUCTION_H
std::vector<IRInstruction*> IRInstructions;
std::map<int, IRInstruction*> IRIdsToInstructions;
#endif
#ifdef HLSTATEMENT_H
std::vector<HLStatement*> HLStatements;
std::map<int, HLStatement*> HLIdsToStatements;
std::map<int, std::vector<HLStatement*> > lineNumToStatement;//this is used to set end_col_nums
#endif
#ifdef HWSIGNAL_H
std::vector<HWSignal*> Signals;
std::map<int, HWSignal*> IdsToSignals;
#endif
#ifdef STATE_H
std::vector<State*> States;
std::map<int, State*> IdsToStates;
#endif
#ifdef VARIABLE_H
std::vector<Variable*> Variables;
std::map<int, Variable*> IdsToVariables;
#endif
#ifdef VARIABLETYPE_H
std::vector<VariableType*> VariableTypes;
std::map<int, VariableType*> IdsToVariableTypes;
#endif

#ifdef VARIABLE_H
#ifdef FUNCTION_H
std::map<Function*, std::vector<Variable*> > functionsToVariables;
#endif
std::vector<Variable*> globalVariables;
#endif

#ifdef VARIABLE_H
std::map<Variable*, VariableUpdateInfo*> variablesToUpdateInfo;
#endif
#ifdef HLSTATEMENT_H
std::map<int, std::vector<HLStatement*> > statesToEffectiveStatements;
#endif
#ifdef FUNCTION_H
std::vector<Function*> functions;
std::map<int, Function*> IdsToFunctions;
#endif
std::map<std::string, std::vector<int> > statesToCycles;

int onChipDebugWindowSize;

//recently added 
#ifdef ONCHIPSIGNAL_H
std::vector<OnChipSignal*> OnChipSignals;
std::map<int, OnChipSignal*> IdsToOnChipSignals;
#endif

#ifdef GDBWRAPPER_H
GDBWrapper *gdbWrapper;
#endif
#ifdef STATE_H
std::vector<State*> observedStates;
#endif
std::string simulationMainReturnVal;
int unInitializedIntValue;
float unInitializedFloatValue;
double unInitializedDoubleValue;
long long unInitializedLongLongValue;

int cycle_counter;
int dummy_cycle_counter;

#ifdef DISCREP
std::string discrepancyFilename = "SWRTLDiscrepancy.log";
#endif

//********************* end Global Variables *********************//

int main(int argc, char *argv[]) {
    
    QApplication app(argc, argv);            
    
    //loading the Inspect.config file
    loadConfigs();
    
    //setting the file name extensions based on the input example
    setFileNames();
    
    #ifndef DISCREP
    //create necessary makeFiles to be used for onChip debug based on the input example directory
    createDebugAndIncrementalScripts();
    #endif

    //starting the modelsim process.. the modelsim process starts regardless of the program's mode... it may not be used at all.
    StartModelsim();
        
    DA = new DataAccess();    
    #ifndef DISCREP
    dbgEngine = new OnChipDebugEngine();
    #endif

    //gdbWrapper object is initialized regardless of the program's mode... it may not be used at all...
    gdbWrapper = new GDBWrapper();
    gdbWrapper->initialize();
    
    //deleting any previous log files
    remove ((workDir + "varlog.txt").c_str());
    remove ((workDir + "stackFrameLog.txt").c_str());
    remove ((workDir + onchipBugLogFilename).c_str());
    
    #ifdef DISCREP
    printf("Automatic Discrepancy Detection\n");
    formMain *nf = new formMain(GDB_BUG_DETECTION);

	bool connect;
	do {
		usleep(100000);
		printf("Trying to connect to Modelsim...\n");
		connect = nf->pushButtonOpenConnection_clicked();
	} while (connect == false);

	nf->pushButtonLoadDesign_clicked();
	nf->pushButtonSingleStepping_clicked();

    printf("Output to file: %s\n", (workDir + discrepancyFilename).c_str());
	nf->printBugMessagesToFile(workDir + discrepancyFilename);

	return 0;

    #else
    
    startForm *sf = new startForm();
    
    sf->exec();
    
    if (sf->mode_number == -1)
        exit(1);
    
    formMain *nf;
    
    switch (sf->mode_number) {
        case 1: {
            nf = new formMain(MODELSIM);
            nf->show();
            break;
        }
        case 2: {
            nf = new formMain(ONCHIP);
            nf->show();
            break;
        }
        case 3: {
            nf = new formMain(GDB);
            nf->show();
            break;
        }
        case 4: {
            nf = new formMain(GDB_SYNC);
            nf->show();
            break;
        }
        case 5: {
            nf = new formMain(GDB_BUG_DETECTION);
            nf->show();
            break;
        }
        case 6: {
            nf = new formMain(ONCHIP_VS_TIMING_SIM);
            nf->show();
            break;
        }
    }                

    return app.exec();
    #endif
}
