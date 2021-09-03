/*
 * linRBF.cpp
 *
 *  Created on: 22/mar/2019 (update YuLi implementation in DistriLake)
 *      Author: MatteoG
 */

#include <math.h>
#include "linRBF.h"

using namespace std;

linRBF::linRBF()
{

}

linRBF::~linRBF(){

}


linRBF::linRBF(unsigned int pM, unsigned int pK, unsigned int pN){
    M = pM;
    K = pK;
    N = pN;
}

void linRBF::setParameters(double* pTheta){

    linRBFparams cParam;

    unsigned int count = 0;
    for(unsigned int i = 0; i < N; i++){
        for(unsigned int j = 0; j < M; j++){
            cParam.c.push_back( pTheta[count] );
            cParam.b.push_back( pTheta[count+1] );
	    			count=count+2;
        }
	  		cParam.theta= pTheta[count];
        for(unsigned int k = 0; k < K; k++){
        	cParam.w.push_back( pTheta[count+1] );
      		count=count+1;
        }
        param.push_back(cParam);
        cParam.c.clear();
        cParam.b.clear();
        cParam.w.clear();
        count=count+1;
    }
}

void linRBF::clearParameters(){

    for(unsigned int i=0; i<param.size(); i++){
        param[i].c.clear();
        param[i].b.clear();
        param[i].w.clear();
    }
    param.clear();

}


vector<double> linRBF::get_output(vector<double> input){

    
	// linRBF
	vector<double> phi;
	double bf, num, den, th;
	for(unsigned int j = 0; j < N; j++){
		bf = 0;
		for(unsigned int i = 0; i < M; i++){
			num = (input[i] - param[j].c[i])*(input[i] - param[j].c[i]) ;
			den = (param[j].b[i]*param[j].b[i]);
			if(den < pow(10,-6)) den = pow(10,-6) ;
			bf = bf + num / den ;
			}
			th = param[j].theta;
			phi.push_back( exp(-bf) + th);
		}

	// output
	vector<double> y;
	double o;
	for(unsigned int k = 0; k < K; k++){
		o = 0;
		for(unsigned int i = 0; i < N; i++){
			o = o + param[i].w[k]*phi[i] ;
		}
		if(o<0) o=0;
		y.push_back(o);
	}

    return y;
}
