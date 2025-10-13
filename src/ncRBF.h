/*
 * ncRBF.h
 *
 *  Created on: 13/feb/2015
 *      Author: MatteoG
 */


#ifndef ncRBF_H
#define ncRBF_H

#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include "param_function.h"
#include "rbf.h"

namespace std{


class ncRBF : public param_function
{
public:
    ncRBF();
    virtual ~ncRBF();

    /**
      * constructor with parameters:
      *     pM = number of input
      *     pK = number of output
      *     pN = number of RBF
      **/
    ncRBF(unsigned int pM, unsigned int pK, unsigned int pN);

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
     *      pTheta = array of parameters (a;c,b,w)
     */
    void setParameters(double* pTheta);

    void setParameters(vector<int> p_input_nodes, vector<int> p_output_nodes, vector<Node> p_hidden_nodes);

protected:
    unsigned int N; // number of RBF
    unsigned int M; // number of input
    unsigned int K; // number of output

    vector<RBFparams> param;
    vector<double> lin_param;

};
}

#endif
