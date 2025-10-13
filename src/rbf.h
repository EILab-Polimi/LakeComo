/*
 * rbf.h
 *
 *  Created on: 31/oct/2013
 *      Author: MatteoG
 */


#ifndef RBF_H
#define RBF_H

#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include "param_function.h"

namespace std{

struct RBFparams{
    vector<double> c; // center (m-input variables)
    vector<double> b; // radius (m-input variables)
    vector<double> w; // weights (K-output)
} ;


class rbf : public param_function
{
public:
    rbf();
    virtual ~rbf();

    /**
      * constructor with parameters:
      *     pM = number of input
      *     pK = number of output
      *     pN = number of RBF
      **/
    rbf(unsigned int pM, unsigned int pK, unsigned int pN);

    /**
      * Clear policy parameters
      */
    void clearParameters();

    /**
      * RBF function (input and output are normalized/standardized)
      **/
    vector<double> get_output(vector<double> input);

    /**
     * RBFsetParameters(double* pTheta)
     *      pTheta = array of parameters (c,b,w)
     */
    void setParameters(double* pTheta);

    void setParameters(vector<int> p_input_nodes, vector<int> p_output_nodes, vector<Node> p_hidden_nodes);

protected:
    unsigned int N; // number of RBF
    unsigned int M; // number of input
    unsigned int K; // number of output

    vector<RBFparams> param;

};
}

#endif
