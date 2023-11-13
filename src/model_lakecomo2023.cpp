/*
 * model_lakecomo.cpp
 *
 *  Created on: 14/oct/2014
 *      Author: MatteoG
 * Updated on: 28/02/2022
 *      Author: DennisZ
 */


#include "model_lakecomo2023.h"
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
    lakecomo* ml = new lakecomo();
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
    h_flo = 1.10 ;
    demand = utils::loadVector("../data/comoDemand.txt", 365);
    //s_low = utils::loadVector("../data/static_low.txt", 365);
    s_low.push_back(-0.2); //instead of loading the vector
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
        /*vector<double> Jcum(Nobj, 0.0);
        // MC simulation 
        for(unsigned int i = 0; i<Nsim; i++) {
            J = simulate(i); //i

            // accumulate the cost for each objective 
            for(unsigned int k=0; k<Nobj; k++){
                Jcum[k] += J[k];
            }
        }

        // save average cost 
        for(unsigned int i=0; i<Nobj; i++){
            obj[i] = Jcum[i]/Nsim;
        }
        */
	}
    mPolicy->clearParameters();

}



vector<double> model_lakecomo::simulate(int ps){

    // INITIALIZATION: storage, level, decision, release
    vector<double> s (H+1,-999) ;
    vector<double> h (H+1,-999) ;
    vector<double> u (H,-999) ;
    vector<double> r (H+1,-999) ;
    double h_p = 0;         //using Malgrate

    // simulation variables
    double qIn; //, qIn_1;      // daily inflow (today and yesterday)
    double r_1;             // yesterday release 
    vector<double> sh_rh;   // storage and release resulting from hourly integration
    vector<double> uu;      // decision vector
    vector<double> input;   // policy input vector
    vector<double> JJ;      // objective vector

    // initial condition
    h[0] = LakeComo->getInitCond();
    s[0] = LakeComo->levelToStorage(h[0], h_p);
    //qIn_1 = inflow00 ;
    r_1 = release00;
    
    //ofstream output;
    //output.open("./trajectories_o.txt");
    // Run simulation:
    for(unsigned int t = 0; t < H; t++){

        // inflows
        qIn = ComoCatchment->getInflow(t, ps);
        //cout << qIn <<  endl;

        // compute decision - standard
        input.push_back( sin( 2*PI*doy_file[t]/T) );
        input.push_back( cos( 2*PI*doy_file[t]/T) );
        input.push_back( h[t] );
        // additional signals
        for(unsigned int i= 0; i<Nex ; i++ ){
            input.push_back( ex_signal.at(i).at(t) );
            //cout <<" " << ex_signal[i].getInflow(t,0) <<endl;
        }
        
        uu = mPolicy->get_NormOutput(input);
        u[t] = uu[0]; // single release decision
        //cout << endl; cout << u[t] <<  endl;
        
        // hourly integration with or without speed constraint
        if ( release00 > 0){
            sh_rh = LakeComo->integration(integStep,t,s[t],u[t],qIn,doy_file[t],ps,h_p,r_1);
        }
        else {
            sh_rh = LakeComo->integration(integStep,t,s[t],u[t],qIn,doy_file[t],ps,h_p);
        }
        
        // assignment of daily values
        s[t+1] = sh_rh[0];
        h[t+1] = LakeComo->storageToLevel(s[t+1], h_p);
        r[t+1] = sh_rh[1];
        
        //output << r[t+1] << " " << h[t+1] << endl;
        
        // for next loop iteration
        r_1 = r[t+1];
        //qIn_1 = qIn;

        // clear subdaily values
        sh_rh.clear();
        input.clear();
        uu.clear();
    }
    
    //output.close();
    
    // remove intial condition from level-storage, we can't penalize s[0], but we have to do it for s[t] with t = 1:H
    // I may do this strating from 2 instead of 1 in the objective functions.
    h.erase(h.begin()); 
    s.erase(s.begin());
    r.erase(r.begin());

    // remove warmup
    if (warmup>0) {
        h.erase(h.begin(),h.begin()+warmup);
        r.erase(r.begin(),r.begin()+warmup);
       //doy.erase(doy.begin(),doy.begin()+warmup); //I won't use warmup, but this problem need to be adressed:
                // If doy is now constant and used in doy_file maybe use by reference and not by value
    }
    
    // remove comment to log level/release trajectory
    //utils::logVector(h,"./logs/levTraj19962008.txt");
    //utils::logVector(r,"./logs/relTraj19962008.txt");

    // compute objectives
    // number of years : they are int so no problem if I have some days more, it truncates the fraction part
    int NYears = H/T;                                   
    
    // mean annual number of flood days
    JJ.push_back( floodDays( &h, h_flo )/NYears );       
    
    // daily average squared deficit (r_{t+1} should be confronted to demand[doy_{t}])
    JJ.push_back( avgDeficitBeta(&r,&demand, &doy_file) ); 
    //JJ.push_back( avgDeficitBeta(&r,&demand,&rain_weight, &doy_file) );      // with time varying exponent loaded from file

    // low levels indicator (h_{t+1} should be confronted to s_low[doy_{t+1}])
    JJ.push_back(staticLow(&h, s_low[0])/NYears);        //static 
    //JJ.push_back(staticLow(&h, &s_low, &doy)/NYears);    //time-varying
    return JJ;
}

double model_lakecomo::floodDays(vector<double> *h, double h_flo){
    
    double c=0.0;
    for(unsigned int i=0; i<h->size(); i++){
        if((*h)[i]>h_flo){
            c=c+1;
        }
    }
    return c;
}

double model_lakecomo::avgDeficitBeta(vector<double> *q, vector<double> *w, vector<double> *doy){
    
    double d, qdiv;
    double gt = 0.0;
    for(unsigned int i=0; i<q->size(); i++){ //
        qdiv = (*q)[i] - LakeComo->getMEF(i);
        //cout << endl; cout << LakeComo->getMEF(i) << endl;
        if( qdiv<0.0 ){
            qdiv = 0.0;
        }
        d = (*w)[(*doy)[i]-1] - qdiv;
        if( d < 0.0 ){
            d = 0.0;
        }
        if( ((*doy)[i] >= 91) && ((*doy)[i] <= 283) ){ // from April to 10th October
            d = d*d;
        }
        //cout << endl; cout << qdiv << " " <<  w[doy[i]-1] << " " << doy[i] << " " << q[i] << endl;
        gt = gt + d ;
    }
    return gt/q->size();
}

double model_lakecomo::avgDeficitBeta(vector<double> *q, vector<double> *w, vector<double> *rain_weight, vector<double> *doy){
    
    double d, qdiv;
    double gt = 0.0;
    for(unsigned int i=0; i<q->size(); i++){ //
        qdiv = (*q)[i] - LakeComo->getMEF(i);
        //cout << endl; cout << LakeComo->getMEF(i) << endl;
        if( qdiv<0.0 ){
            qdiv = 0.0;
        }
        d = (*w)[(*doy)[i]-1] - qdiv;
        if( d < 0.0 ){
            d = 0.0;
        }
        if( ((*doy)[i] >= 91) && ((*doy)[i] <= 283) ){ // from April to 10th October
            d = pow(d, 2-(*rain_weight)[i]);
        }
        //cout << endl; cout << qdiv << " " <<  w[doy[i]-1] << " " << doy[i] << " " << q[i] << endl;
        gt = gt + d ;
    }
    return gt/q->size();
}

double model_lakecomo::staticLow(vector<double> *h, double hls){
    
    double c=0.0;
    for(unsigned int i=0; i<h->size(); i++){
        if((*h)[i]<hls){
            
            c=c+1;
            
        }
    }
    return c;
    
}

double model_lakecomo::staticLow(vector<double> *h, vector<double> *hls, vector<double> *doy){
    
    double c=0.0;
    for(unsigned int i=0; i<h->size(); i++){
        // cout << endl << "index=" << i <<" doy=" << doy[i] <<" hls=" <<hls[doy[i]-1] <<endl;
        if((*h)[i]<(*hls)[(*doy)[i]-1]){
            
            c=c+1;
            
        }
    }
    return c;
    
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
        vector<double> doy_temp (H+1,-999);
        for(unsigned int t = 0; t < H+1; t++){
            doy_temp[t] =  (initDay+t-1)%T+1;  // day of the year
        }
        doy_file = doy_temp;
    }else{
        in.ignore(1000,'\n');
        in >> file0;
        doy_file = utils::loadVector(file0,H+1);
        initDay = 0;
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
    Como_catch_param.inflow_file.row = 1;
    Como_catch_param.inflow_file.col = H;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    // EXOGENOUS SIGNAL FOR POLICY INPUT
    //Look for the <CATCHMENT> key
    while (sJunk != "<ESIGNALS>")
    {
        in >> sJunk;
    }
    in >> Nex;
    //cout << "signals: " <<Nex <<endl;
    in.ignore(1000,'\n');
    for( int i = 0; i < Nex; i++) {
        std::string file0;
        in >> file0;
        in.ignore(1000,'\n');

        ex_signal.push_back(utils::loadVector(file0,H));
        //cout << " inflow " << ex_signal[i].getInflow(0,0) <<endl;
    }
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

    //Look for the <INIT_INFLOW> key
    while (sJunk != "<INIT_INFLOW>")
    {
        in >> sJunk;
    }
    in >> inflow00;
    //Return to the beginning of the file
    in.seekg(0, ios::beg);

    //Look for the <INIT_INFLOW> key
    while (sJunk != "<INIT_RELEASE>")
    {
        in >> sJunk;
    }
    in >> release00;
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

