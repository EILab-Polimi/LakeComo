/*
 * rbf.cpp
 *
 *  Created on: 31/oct/2013
 *      Author: MatteoG
 */

#include "rbf.h"

using namespace std;

rbf::rbf()
{
}

rbf::~rbf(){
}


rbf::rbf(unsigned int pM, unsigned int pK, unsigned int pN){
    M = pM;
    K = pK;
    N = pN;
}

void rbf::setParameters(double* pTheta){

    vector<double> ws (K,0.0);
    RBFparams cParam;

    unsigned int idk = 2*M-1;
    for(unsigned int i = 0; i < N; i++){
        for(unsigned int k = 0; k < K; k++){
            idk = idk+1;
            ws[k] = ws[k] + pTheta[idk];
        }
        idk = idk+2*M;
    }

    unsigned int count = 0;
    for(unsigned int i = 0; i < N; i++){
        for(unsigned int j = 0; j < M; j++){
            cParam.c.push_back( pTheta[count] );
            cParam.b.push_back( pTheta[count+1] );
            count = count+2;
        }
        for(unsigned int k = 0; k < K; k++){
            if(ws[k] < pow(10,-6)){
                cParam.w.push_back( pTheta[count] );
            }else{
                cParam.w.push_back( pTheta[count]/ws[k] );
            }
            count = count+1;
        }
        param.push_back(cParam);
        cParam.c.clear();
        cParam.b.clear();
        cParam.w.clear();
    }
}

void rbf::setParameters(vector<int> p_input_nodes, vector<int> p_output_nodes, vector<Node> p_hidden_nodes){
    // Not implemented
}

void rbf::clearParameters(){

    for(unsigned int i=0; i<param.size(); i++){
        param[i].c.clear();
        param[i].b.clear();
        param[i].w.clear();
    }
    param.clear();

}


vector<double> rbf::get_output(vector<double> input){

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
        o = 0;
        for(unsigned int i = 0; i < N; i++){
            o = o + param[i].w[k]*phi[i] ;
        }
        y.push_back(o);
    }

    return y;
}
