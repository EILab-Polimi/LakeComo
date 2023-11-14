/*
 * lake.cpp
 *
 *  Created on: 30/jan/2014
 *      Author: MatteoG
 */

#include "lake.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <math.h>

using namespace std;

lake::lake() {
    // TODO Auto-generated constructor stub
}

lake::~lake() {
    // TODO Auto-generated destructor stub
}

vector<double> lake::integration( int HH, int tt, double s0, double uu, double n_sim, int cday, int ps){

    int sim_step = 3600*24/HH;    // s/step
    double S, E ;

    // INITIALIZATION: storage, level, decision, release
    vector<double> s (HH+1,-999) ;
    vector<double> r (HH,-999) ;
    vector<double> stor_rel;

    // intial conditions
    s[0] = s0;

    for(unsigned int i=0; i<HH; i++){
        // compute actual release
        r[i] = actual_release(uu,s[i],cday);

        // compute evaporation
        if(EV == 1){
            S = this->levelToSurface( this->storageToLevel(s[i]) );
            E = evap_rates[cday-1]/1000 * S /86400;
        }else if (EV>1) {
            // E = compute_evaporation(); TO BE IMPLEMENTED
        }else{
            E = 0.0;
        }

        // system transition
        s[i+1] = s[i] + sim_step*( n_sim - r[i] - E );
    }

    stor_rel.push_back( s[HH] );
    stor_rel.push_back( utils::computeMean(r) );

    return stor_rel;
}

double lake::actual_release(double uu, double s, int cday){

    // min-Max storage-discharge relationship
    double qm = this->min_release( s, cday );
    double qM = this->max_release( s, cday );

    // actual release
    double rr = min( qM , max( qm , uu ) ) ;
    return rr;
}

double lake::relToTailwater(double r){
    double hd = 0.0;
    if(tailwater.size()>0){
        hd = utils::interp_lin( tailwater[0], tailwater[1], r );
    }
    return hd;
}

void lake::setInitCond(double ci){
    init_condition = ci;
}

double lake::getInitCond(){
    return init_condition;
}

void lake::setEvap(int pEV){
    EV = pEV;
}

void lake::setEvapRates(myFile pEvap){
    evap_rates = utils::loadVector(pEvap.filename,pEvap.row);
}

void lake::setRatCurve(myFile pRatCurve){
    rating_curve = utils::loadMatrix(pRatCurve.filename,pRatCurve.row,pRatCurve.col);
}

void lake::setLSV_Rel(myFile pLSV_Rel){
    lsv_rel = utils::loadMatrix(pLSV_Rel.filename,pLSV_Rel.row,pLSV_Rel.col);
}

void lake::setSurface(double pA){
    A = pA;
}

void lake::setTailwater(myFile pTailWater){
    tailwater = utils::loadMatrix( pTailWater.filename, pTailWater.row, pTailWater.col );
}

void lake::setMEF(myFile pMEF){
    minEnvFlow = utils::loadVector( pMEF.filename, pMEF.row );
}

double lake::getMEF(double pDoy){
    return minEnvFlow[pDoy] ;
}
