/*
 * lakecomo.h
 *
 *  Created on: 07/feb/2014
 *      Author: MatteoG
 *  Updated on: 28/02/2022
 *      Author: DennisZ
 */

#ifndef lakecomo_H_
#define lakecomo_H_

#include <math.h>
#include <vector>
#include <string>
#include "lake.h"

namespace std{

class lakecomo : public lake {
public:
        lakecomo();
        virtual ~lakecomo();

        /**
          * Level-Surface-Storage functions
          *  - h = level [m]
          *  - s = storage [m^3]
          *  - p = shift for h0 [m]
          */
        double levelToSurface(double h);
        double levelToStorage(double h);
        double storageToLevel(double s);
        double levelToStorage(double h, double p);
        double storageToLevel(double s, double p);
        

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
        vector<double> integration( int HH, int tt, double s0, double uu, double n_sim, int cday, int ps, double p);
        vector<double> integration( int HH, int tt, double s0, double uu, double n_sim, int cday, int ps, double p, double r0 );

        // override of actual release function 
        double actual_release( double uu, double s, int cday, double p );
        double actual_release( double uu, double s, int cday, double p, double q); // included check of mef vs inflow (q)

protected:


        /**
          * Function to compute min-max release
          *  - s = lake storage
          *  - cday = day of the year (for MEF)
          *  - p = shift for h0 [m]
          *  - q = inflow of that day (italian legislation defines MEF as the minimum between the value and the available stream)
          */
        double min_release( double s, int cday );
        double max_release( double s, int cday );
        double min_release( double s, int cday, double p );
        double max_release( double s, int cday, double p );
        double min_release( double s, int cday, double p, double q); // included check of mef vs inflow (q)
};

}

#endif
