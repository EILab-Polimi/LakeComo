/*
 * model_lakecomo.h
 *
 *  Created on: 7/oct/2014
 *      Author: MatteoG
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

        // reservoir: Lake Como
        reservoir_param Como_param;
        lake* LakeComo;

        // operating policy
        pFunction_param p_param;
        param_function* mPolicy;

        // objective function data
        int warmup;                                 // number of days of warmup before obj calculation starts
        vector<vector<double> > level_areaFlood;    // level (cm) - flooded area in Como (m2)
        vector<double> demand;                      // total downstream demand (m3/s)
        double h_flo;       // flooding threshold

        /**
         * function to perform the simulation over the scenario ps
         */
         vector<double> simulate(int ps);

        /**
         * Functions to compute the objective functions:
         **/
         double floodDays(vector<double> h, double h_flo);
         double avgDeficitBeta(vector<double> q, vector<double> w, vector<double> doy);
};

}

#endif /* model_lakecomo_H_ */
