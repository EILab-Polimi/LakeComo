/*
 * linRBF.h
 *
 *  Created on: 22/mar/2019 (update YuLi implementation in DistriLake)
 *      Author: MatteoG
 */


#ifndef linRBF_H
#define linRBF_H

#include <vector>
#include "param_function.h"
#include "rbf.h"

using namespace std;

struct linRBFparams{
    vector<double> c; // center (m-input variables)
    vector<double> b; // radius (m-input variables)
    double theta; // constant (N-RBF)
    vector<double> w; // weights (K-output)
} ;


class linRBF : public param_function{
public:
    linRBF();
    virtual ~linRBF();

    /**
      * constructor with parameters:
      *     pM = number of input
      *     pK = number of output
      *     pN = number of RBF
      **/
    linRBF(unsigned int pM, unsigned int pK, unsigned int pN);

    /**
      * Clear policy parameters
      */
    void clearParameters();

    
    /**
      * RBF policy (input and output are normalized)
      **/
    vector<double> get_output(vector<double> input);

    /**
     * RBFsetParameters(double* pTheta)
     *      pTheta = array of parameters (c,b,w)
     */
    void setParameters(double* pTheta);

protected:
    unsigned int N; // number of RBF
    unsigned int M; // number of input
    unsigned int K; // number of output

    vector<linRBFparams> param;

};

#endif
