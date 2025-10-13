/*
 * ncRBF.cpp
 *
 *  Created on: 13/feb/2015
 *      Author: MatteoG
 */

#include "ncRBF.h"

using namespace std;

ncRBF::ncRBF()
{
}

ncRBF::~ncRBF(){
}


ncRBF::ncRBF(unsigned int pM, unsigned int pK, unsigned int pN){
    M = pM;
    K = pK;
    N = pN;
}

void ncRBF::setParameters(double* pTheta){

    RBFparams cParam;
    // linear parameters
    unsigned int count = 0;
    for(unsigned int k=0; k<K; k++){
        lin_param.push_back( pTheta[count] ) ;
        count = count+1;
    }
    // RBF
    for(unsigned int i = 0; i < N; i++){
        for(unsigned int j = 0; j < M; j++){
            cParam.c.push_back( pTheta[count] );
            cParam.b.push_back( pTheta[count+1] );
            count = count+2;
        }
        for(unsigned int k = 0; k < K; k++){    
            cParam.w.push_back( pTheta[count] );
            count = count+1;
        }
        param.push_back(cParam);
        cParam.c.clear();
        cParam.b.clear();
        cParam.w.clear();
    }
}

void ncRBF::setParameters(vector<int> p_input_nodes, vector<int> p_output_nodes, vector<Node> p_hidden_nodes){
    // Not implemented
}

void ncRBF::clearParameters(){

    for(unsigned int i=0; i<param.size(); i++){
        param[i].c.clear();
        param[i].b.clear();
        param[i].w.clear();
    }
    param.clear();
    lin_param.clear();

}


vector<double> ncRBF::get_output(vector<double> input){

    // RBF
    vector<double> phi;
    double bf, num, den;
    for(unsigned int j = 0; j < N; j++){
        bf = 0;
        for(unsigned int i = 0; i < M; i++){
            num = (input[i] - param[j].c[i])*(input[i] - param[j].c[i]) ;
            den = (param[j].b[i]*param[j].b[i]);
            if(den < pow(10,-6)){
                den = pow(10,-6) ;
            }
            bf = bf + num / den ;
        }
        phi.push_back( exp(-bf) );
    }

    // output
    vector<double> y;
    double o;
    for(unsigned int k = 0; k < K; k++){
        o = lin_param[k];
        for(unsigned int i = 0; i < N; i++){
            o = o + param[i].w[k]*phi[i] ;
        }
        if(o>1){
            o=1.0;
        }
        if(o<0){
            o=0.0;
        }
        y.push_back(o);
    }

    return y;
}
