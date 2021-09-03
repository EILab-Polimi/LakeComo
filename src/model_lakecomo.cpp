/*
 * model_lakecomo.cpp
 *
 *  Created on: 14/oct/2014
 *      Author: MatteoG
 */


#include "model_lakecomo.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>

using namespace std;

#define PI 3.14159265

model_lakecomo::model_lakecomo() {
    // TODO Auto-generated constructor stub
}

model_lakecomo::~model_lakecomo() {
    // TODO Auto-generated destructor stub
}

model_lakecomo::model_lakecomo(string filename){
    
    readFileSettings(filename);

    // create catchment
    catchment* mc = new catchment(Como_catch_param);
    ComoCatchment = mc;

    // create lake
    lake* ml = new lakecomo();
    LakeComo = ml;
    LakeComo->setEvap(0);
    Como_param.minEnvFlow.filename = "../data/MEF_como.txt";
    Como_param.minEnvFlow.row = T;
    LakeComo->setMEF(Como_param.minEnvFlow);
    LakeComo->setSurface(145900000);
    LakeComo->setInitCond(Como_param.initCond);

    // policy
    if (p_param.tPolicy == 4) {
        param_function* mp4 = new ncRBF(p_param.policyInput,p_param.policyOutput,p_param.policyStr);
        mPolicy = mp4;
    }else if(p_param.tPolicy == 7){
        param_function* mp7 = new linRBF(p_param.policyInput,p_param.policyOutput,p_param.policyStr);
        mPolicy = mp7;   
    }else{
        cout << "Error: policy architecture not defined";
    }

    // min-max policy input
    mPolicy->setMaxInput(p_param.MIn); mPolicy->setMaxOutput(p_param.MOut);
    mPolicy->setMinInput(p_param.mIn); mPolicy->setMinOutput(p_param.mOut);


    // OBJECTIVES
    h_flo = 1.24 ;
    demand = utils::loadVector("../data/comoDemand.txt", 365);

}

void model_lakecomo::clear_model_lakecomo(){
    delete LakeComo;
    delete mPolicy;
    delete ComoCatchment;
}


int model_lakecomo::getNobj() {
	return Nobj;
}

int model_lakecomo::getNvar() {
        return Nvar;
}


void model_lakecomo::evaluate(double* var, double* obj){

    // set CONTROL POLICY
    mPolicy->setParameters(var);

    vector<double> J ;

    if(Nsim < 2){ // single simulation
        J = simulate(0);
        for(unsigned int i=0; i<Nobj; i++){
            obj[i] = J[i];
        }
    }else{
        // MC simulation to be implemented
	}
    mPolicy->clearParameters();

}



vector<double> model_lakecomo::simulate(int ps){

    // INITIALIZATION: storage, level, decision, release
    vector<double> s (H+1,-999) ;
    vector<double> h (H+1,-999) ;
    vector<double> u (H,-999) ;
    vector<double> r (H+1,-999) ;
    vector<double> doy (H,-999) ;

    // simulation variables
    double qIn;      // daily inflow 
    vector<double> sh_rh;   // storage and release resulting from hourly integration
    vector<double> uu;      // decision vector
    vector<double> input;   // policy input vector
    vector<double> JJ;      // objective vector

    // initial condition
    h[0] = LakeComo->getInitCond();
    s[0] = LakeComo->levelToStorage(h[0]);
    
    // exogeneous information
    vector<double> qForecast = utils::loadVector("../data/qSimAnomL51.txt", H); // perfect forecast of 51-day cumulated streamflow 
    
    // Run simulation:
    for(unsigned int t = 0; t < H; t++){

        // day of the year
        doy[t] =  (initDay+t-1)%T+1;  // day of the year

        // inflows
        qIn = ComoCatchment->getInflow(t, ps);

        // compute decision - standard
        input.push_back( sin( 2*PI*doy[t]/T) );
        input.push_back( cos( 2*PI*doy[t]/T) );
        input.push_back( h[t] );
        if( p_param.policyInput > 3 )
        	input.push_back( qForecast[t] );
        
        uu = mPolicy->get_NormOutput(input);
        u[t] = uu[0]; // single release decision

        // hourly integration
        sh_rh = LakeComo->integration(integStep,t,s[t],u[t],qIn,doy[t],ps);

        // assignment of daily values
        s[t+1] = sh_rh[0];
        r[t+1] = sh_rh[1];
        h[t+1] = LakeComo->storageToLevel(s[t+1]);

        // clear subdaily values
        sh_rh.clear();
        input.clear();
        uu.clear();
    }
    
    // remove warmup
    h.erase(h.begin()); r.erase(r.begin());
    h.erase(h.begin(),h.begin()+warmup);
    r.erase(r.begin(),r.begin()+warmup);
    doy.erase(doy.begin(),doy.begin()+warmup);
    
    // remove comment to log level/release trajectory
    //utils::logVector(h,"./logs/levTraj19962008.txt");
    //utils::logVector(r,"./logs/relTraj19962008.txt");

    // compute objectives
    int NYears = H/T; // number of years
    JJ.push_back( floodDays( h, h_flo )/NYears );       // mean annual number of flood days
    JJ.push_back( avgDeficitBeta(r,demand,doy) );      // daily average squared deficti
    return JJ;
}

double model_lakecomo::floodDays(vector<double> h, double h_flo){
    
    double c=0.0;
    for(unsigned int i=0; i<h.size(); i++){
        if(h[i]>h_flo){
            c=c+1;
        }
    }
    return c;
}

double model_lakecomo::avgDeficitBeta(vector<double> q, vector<double> w, vector<double> doy){
    
    double d, qdiv;
    double gt = 0.0;
    for(unsigned int i=0; i<q.size(); i++){
        qdiv = q[i] - LakeComo->getMEF(doy[i]-1);
        if( qdiv<0.0 ){
            qdiv = 0.0;
        }
        d = w[doy[i]-1] - qdiv;
        if( d < 0.0 ){
            d = 0.0;
        }
        if( (doy[i] > 120) && (doy[i] <= 243) ){ // from May to August
            d = 2*d;
        }
        gt = gt + d*d ;
    }
    return gt/q.size();
}

void model_lakecomo::readFileSettings(string filename){

    ifstream in;
    string sJunk = "";

    in.open(filename.c_str(), ios_base::in);
    if(!in)
    {
        cout << "The input file specified: " << filename << " could not be found!" << endl;
        exit(1);
    }

    // PROBLEM SETTING
    //Look for the <NUM_SIM> key
    while (sJunk != "<NUM_SIM>")
    {
        in >> sJunk;
    }
    in >> Nsim;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <DIM_ENSEMBLE> key
    while (sJunk != "<DIM_ENSEMBLE>")
    {
        in >> sJunk;
    }
    in >> NN;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <PERIOD> key
    while (sJunk != "<PERIOD>")
    {
        in >> sJunk;
    }
    in >> T;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <INTEGRATION> key
    while (sJunk != "<INTEGRATION>")
    {
        in >> sJunk;
    }
    in >> integStep;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <SIM_HORIZON> key
    while (sJunk != "<SIM_HORIZON>")
    {
        in >> sJunk;
    }
    in >> H;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <NUM_OBJ> key
    while (sJunk != "<NUM_OBJ>")
    {
        in >> sJunk;
    }
    in >> Nobj;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <NUM_VAR> key
    while (sJunk != "<NUM_VAR>")
    {
        in >> sJunk;
    }
    in >> Nvar;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <WARMUP> key
    while (sJunk != "<WARMUP>")
    {
        in >> sJunk;
    }
    in >> warmup;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <DOY> key
    string file0;
    int temp=0;
    while (sJunk != "<DOY>")
    {
        in >> sJunk;
    }
    in >> temp;
    if(temp>0){
        initDay = temp;
    }else{
        in.ignore(1000,'\n');
        in >> file0;
        doy_file = utils::loadVector(file0,H);
    }
    //Return to the beginning of the file
    in.seekg(0, ios::beg);


    // CATCHMENT MODEL
    //Look for the <CATCHMENT> key
    while (sJunk != "<CATCHMENT>")
    {
        in >> sJunk;
    }
    in >> Como_catch_param.CM;
    in.ignore(1000,'\n');
    in >> Como_catch_param.inflow_file.filename;
    Como_catch_param.inflow_file.row = NN;
    Como_catch_param.inflow_file.col = H;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    // MODEL INITIALIZATION
    //Look for the <INIT_CONDITION> key
    while (sJunk != "<INIT_CONDITION>")
    {
        in >> sJunk;
    }
    in >> Como_param.initCond;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);


    //Look for the <POLICY_CLASS> key
    while (sJunk != "<POLICY_CLASS>")
    {
        in >> sJunk;
    }
    in >> p_param.tPolicy;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <NUM_INPUT> key
    double i1, i2;
    while (sJunk != "<NUM_INPUT>")
    {
        in >> sJunk;
    }
    in >> p_param.policyInput;
    in.ignore(1000,'\n');
    //Loop through all of the input data and read in this order:
    for (int i=0; i<p_param.policyInput; i++)
    {
        in >> i1 >> i2;
        p_param.mIn.push_back(i1);
        p_param.MIn.push_back(i2);
        in.ignore(1000,'\n');
    }
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <NUM_OUTPUT> key
    double o1, o2;
    while (sJunk != "<NUM_OUTPUT>")
    {
        in >> sJunk;
    }
    in >> p_param.policyOutput;
    in.ignore(1000,'\n');
    //Loop through all of the input data and read in this order:
    for (int i=0; i<p_param.policyOutput; i++)
    {
        in >> o1 >> o2;
        p_param.mOut.push_back(o1);
        p_param.MOut.push_back(o2);
        in.ignore(1000,'\n');
    }
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <POLICY_STRUCTURE> key
    while (sJunk != "<POLICY_STRUCTURE>")
    {
        in >> sJunk;
    }
    in >> p_param.policyStr;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Close the input file
    in.close();

    return;

}

