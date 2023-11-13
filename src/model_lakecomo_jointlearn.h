/*
 * model_lakecomo.h
 *
 *  Created on: 7/oct/2014
 *      Author: MatteoG
 * Updated on: 28/02/2022
 *      Author: DennisZ
 */

#ifndef model_lakecomo_H_
#define model_lakecomo_H_

#include <math.h>
#include <vector>
#include <string>
#include "lake.h"
#include "lakecomo.h"
#include "catchment.h"
#include "param_function.h"
#include "rbf.h"
#include "ncRBF.h"
#include "linRBF.h"

namespace std{

class model_lakecomo {
public:
        model_lakecomo();
        virtual ~model_lakecomo();

        model_lakecomo(string filename);
        void clear_model_lakecomo();

        /**
        * number of objectives and variables
        */
        int getNobj();
        int getNvar();

        /**
         * function to perform the optimization:
         *  - var = decision variables
         *  - obj = objectives
        */
        void evaluate(double* var, double* obj);

protected:
        void readFileSettings(string filename);

        // problem setting
        int Nsim;               // number of simulation (1=deterministic, >1 MC)
        int NN;                 // dimension of the stochastic ensemble
        int T;                  // period
        int integStep;          // integration timestep = number of subdaily steps
        int H;                  // simulation horizon
        int Nobj;               // number of objectives
        int Nvar;               // number of variables
        int initDay;            // first day of simulation
        vector<double> doy_file;     // day of the year (it includes leap years, otherwise doy is computed runtime in the simulation)

        // catchment
        catchment_param Como_catch_param;
        catchment* ComoCatchment;

        double Nex;             // number of exogenous signals to append to BOP
        vector<vector<double> > ex_signal;   // vector of exogneous signals for input policy (ready for input)
        vector<vector<vector<double> > > raw_data;    // vector of signals to process for input policy (raw data)

        // reservoir: Lake Como
        reservoir_param Como_param;
        lakecomo* LakeComo;

        // operating policy
        pFunction_param p_param;
        param_function* mPolicy;

        vector<vector<double> > processed_data;    // vector of processed data for input policy (ready for input)


        // objective function data
        int warmup;                                 // number of days of warmup before obj calculation starts
        vector<vector<double> > level_areaFlood;    // level (cm) - flooded area in Como (m2)
        vector<double> demand;                      // total downstream demand (m3/s)
        vector<double> s_low;                      // total downstream demand
        vector<double> rain_weight;                      // total downstream demand
        double h_flo;       // flooding threshold
        double q_hp;        // downstream hydropower demand
        double inflow00;    // previous day inflow
        double release00;   // previous day release for dam speed constraint, if negative constraint is not active. 

        /**
         * function to perform the simulation over the scenario ps
         */
         vector<double> simulate(int ps);

        /**
         * Functions to compute the objective functions:
         **/
         double floodDays(vector<double> *h, double h_flo);
         double avgDeficitBeta(vector<double> *q, vector<double> *w, vector<double> *doy);
         double avgDeficitBeta(vector<double> *q, vector<double> *w, vector<double> *rain_weight, vector<double> *doy);
         double staticLow(vector<double> *h, double s_low);
         double staticLow(vector<double> *h, vector<double> *s_low, vector<double> *doy);
         
};

}

#endif /* model_lakecomo_H_ */
