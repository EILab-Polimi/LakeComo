/*
 * lakecomo.cpp
 *
 *  Created on: 7/feb/2014
 *      Author: MatteoG
 *  Updated on: 28/02/2022
 *      Author: DennisZ
 */

#include "lakecomo.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <math.h>

using namespace std;

lakecomo::lakecomo() {
    // TODO Auto-generated constructor stub
}

lakecomo::~lakecomo() {
    // TODO Auto-generated destructor stub
}

/**
* Level-Surface-Storage functions
*  - h = level [m]
*  - s = storage [m^3]
*  - p = shift for h0 [m]
*/

double lakecomo::storageToLevel(double s){
    double h0 = -0.5;
    double h = s/A + h0;
    return h;
}

double lakecomo::levelToStorage(double h){
    double h0 = -0.5;
    double s = A*(h - h0);
    return s;
}

double lakecomo::levelToSurface(double h){
    double S = A;
    return S;
}

double lakecomo::storageToLevel(double s, double p){
    double h0 = -0.4 +p;
    double h = s/A + h0;

    return h;
}

double lakecomo::levelToStorage(double h, double p){
    double h0 = -0.4 + p;
    double s = A*(h - h0);
    return s;
}

// override of integration function with speed release constraint
/**
* Integration:
*  - HH = fraction of day
*  - tt = current day
*  - s0 = initial condition (storage)
*  - uu = decision
*  - n_sim = inflow
*  - cday = day of the year (for MEF)
*  - ps = simulation scenario
*  - p = shift for h0 [m]
*  - r0 = intial condition (release)
*/
vector<double> std::lakecomo::integration(int HH, int tt, double s0, double uu, double n_sim, int cday, int ps, double p)
{
    return integration(HH, tt, s0, uu, n_sim, cday, ps, p, 10000.0);
}
vector<double> lakecomo::integration(int HH, int tt, double s0, double uu, double n_sim, int cday, int ps, double p, double r0)
{

    int sim_step = 3600*24/HH;    // s/step
    double E ;

    // INITIALIZATION: storage, level, decision, release
    vector<double> s (HH+1,-999) ;
    vector<double> r (HH,-999) ;
    vector<double> stor_rel;

    // intial conditions
    s[0] = s0;

    for(unsigned int i=0; i<HH; i++){
        // compute actual release
        r[i] = actual_release(uu,s[i],cday,p,n_sim);

        // compute evaporation
        if(EV == 1){
            double S;
            S = this->levelToSurface( this->storageToLevel(s[i], p) );
            
            E = evap_rates[cday-1]/1000 * S /86400;
        }else if (EV>1) {
            // E = compute_evaporation(); TO BE IMPLEMENTED
        }else{
            E = 0.0;
        }

        // system transition
        s[i+1] = s[i] + sim_step*( n_sim - r[i] - E );
    }
    double rHH = utils::computeMean(r) ;

    // constraint on max dam opening
    double h = storageToLevel(s0,p);
    double rDay, sDay;
    if(h > 1.10){       // dam is already completely open
        rDay = rHH;
        sDay = s[HH];
    }else{
        // daily increases assuming dr = 30 m3s-1 every 2 hours, 
        // but only during daytime 16 hr (no flood risk, h < 0.8)
        // for the entire day due to flood risk ( h > 0.8)
        double vr =  h<0.8 ? 240:360 ; 
        rDay = min( rHH, r0+vr );
        sDay = s0 + 3600*24*( n_sim - rDay ) ;
    }

    stor_rel.push_back( sDay );
    stor_rel.push_back( rDay );

    return stor_rel;
}

/**
* Function to compute min-max release
*  - s = lake storage
*  - cday = day of the year (for MEF)
*  - p = shift in h0 [m]
*  - q = inflow of that day (italian legislation defines MEF as the minimum between the value and the available stream)
*/
double lakecomo::min_release(double s, int cday){
    double q = 0.0;
    double DMV = minEnvFlow[cday-1];
    double h = storageToLevel(s);
    if(h <= -0.50){
        q = 0.0;
    }else if (h <= 1.25) {
        q = DMV;
    //}else if (h <= 1.30) {
    //    q = 938.2*h - 1229.02;
    }else{
        q = 33.37*pow( (h + 2.5), 2.015);
    }
    return q;
}

double lakecomo::max_release(double s, int cday){

    double q = 0.0;
    double h = storageToLevel(s);
    if(h <= -0.5){
        q = 0.0;
    }else if (h <= -0.40) {
        q = 1488.1*h + 744.05;
    }else{
        q = 33.37*pow( (h + 2.5), 2.015);
    }

    return q;
}

double lakecomo::min_release(double s, int cday, double p){
    double h0 = -0.4 + p;
    double r = 0.0;
    double DMV = minEnvFlow[cday-1];
    double h = storageToLevel(s, p);

    double idx = 33.37*pow( (h0 + 0.1 + 2.5), 2.015);
    double m = idx/0.1;
    double it = -(idx/0.1)*h0;
    
    if(h <= h0){
        r = 0.0;
    }else if (h <= 1.10) {
        r = min(DMV, m*h+it);
    //}else if (h <= 1.30) {
    //    r = 938.2*h - 1229.02;
    }else{
        r = 33.37*pow( (h + 2.5), 2.015);
    }
    return r;
}

double lakecomo::max_release(double s, int cday, double p){
    double h0 = -0.4 + p;
    double r = 0.0;
    double h = storageToLevel(s, p);
    
    double idx = 33.37*pow( (h0 + 0.1 + 2.5), 2.015);
    double m = idx/0.1;
    double it = -(idx/0.1)*h0;
    
    if(h <= h0){
        r = 0.0;
    }else if (h <= h0 + 0.1) {
        r = m*h + it;
    }else{
        r = 33.37*pow( (h + 2.5), 2.015);
    }

    return r;
}


double lakecomo::min_release(double s, int cday, double p, double q){
    double h0 = -0.4 + p;
    double r = 0.0;
    double DMV = minEnvFlow[cday-1];
    double h = storageToLevel(s, p);

    double idx = 33.37*pow( (h0 + 0.1 + 2.5), 2.015);
    double m = idx/0.1;
    double it = -(idx/0.1)*h0;
    
    if(h <= h0){
        r = 0.0;
    }else if (h <= 1.10) {
        r = min(DMV, m*h+it);
        r = min( r, q );
    //}else if (h <= 1.30) {
    //    r = 938.2*h - 1229.02;
    }else{
        r = 33.37*pow( (h + 2.5), 2.015);
    }
    return r;
}

double lakecomo::actual_release(double uu, double s, int cday, double p ){

    // min-Max storage-discharge relationship
    double qm = min_release( s, cday, p);
    double qM = max_release( s, cday, p );

    // actual release
    double rr = min( qM , max( qm , uu ) ) ;
    return rr;
}

double lakecomo::actual_release(double uu, double s, int cday, double p, double q){

    // min-Max storage-discharge relationship
    double qm = min_release( s, cday, p, q );
    double qM = max_release( s, cday, p );

    // actual release
    double rr = min( qM , max( qm , uu ) ) ;
    return rr;
}