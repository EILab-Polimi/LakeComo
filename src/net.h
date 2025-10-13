/*
 * net.h
 *
 *  Created on: 02/aug/2024
 *      Author: DavideS
 */


#ifndef NET_H
#define NET_H

#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <map>
#include <numeric>
#include <algorithm>
#include "param_function.h"

using namespace std;

class net : public param_function
{
public:
    net();
    virtual ~net();

    /**
      * constructor with parameters:
      *     pM = number of input
      *     pK = number of output
      **/
    net(unsigned int pM, unsigned int pK);

    /**
      * Clear policy parameters
      */
    void clearParameters();

    /**
      * net function (input and output are normalized/standardized)
      **/
    vector<double> get_output(vector<double> input);

    /**
     * RBFsetParameters(double* pTheta)
     *      pTheta = array of parameters (a;c,b,w)
     */
    void setParameters(double* pTheta);

    /**
     * NEMODPSParameters(vector<Node> pNode)
     *      pNode = vector of nodes
     */
    void setParameters(vector<int> p_input_nodes, vector<int> p_output_nodes, vector<Node> p_hidden_nodes);

protected:
    unsigned int M; // number of input
    unsigned int K; // number of output
    vector<int> input_nodes;
    vector<int> output_nodes;
    vector<Node> hidden_nodes;
    map<int, float> values;

};

#endif //NET_H
