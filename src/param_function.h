/*
 * param_function.h
 *
 *  Created on: 31/oct/2013
 *      Author: MatteoG
 */

#ifndef PARAMFUNCTION_H
#define PARAMFUNCTION_H

#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include "utils.h"

namespace std{

struct pFunction_param{
    int tPolicy;            // class of policy { 1=RBF; 2=ANN(not implemented yet); 3=pwLinear }
    int policyInput;        // number of policy input
    int policyOutput;        // number of policy output
    int policyStr;        // policy architecture (e.g., number of RBF, number of linear segments)
    vector<double> mIn,mOut,MIn,MOut; // min-max policy input and output
    vector<double> muIn,muOut,stdIn,stdOut; // mean-std policy input and output
};


class param_function
{
public:
    param_function();
    virtual ~param_function();

    /**
     * Get function input/output
     */
    unsigned int getInputNumber();
    unsigned int getOutputNumber();

    /**
      * Set function parameters
      **/
    virtual void setParameters(double* pTheta) = 0;

    /**
      * Set function parameters
      **/
    virtual void setParameters(vector<int> p_input_nodes, vector<int> p_output_nodes, vector<Node> p_hidden_nodes) = 0;

    /**
      * Clear function parameters
      */
    virtual void clearParameters() = 0;

    /**
      * Parametrized function with standardized/normalized values
      **/
    vector<double> get_StdOutput(vector<double> pInput);
    vector<double> get_NormOutput(vector<double> pInput);

    /**
      * generic parametrized function (input and output are normalized/standardized)
      **/
    virtual vector<double> get_output(vector<double> input) = 0;

    /**
     * Set/Get min and max input/output for normalization
     */
    void setMaxInput(vector<double> pV);
    void setMaxOutput(vector<double> pV);
    void setMinInput(vector<double> pV);
    void setMinOutput(vector<double> pV);
    //vector<double> getMaxInput();
    //vector<double> getMaxOutput();
    //vector<double> getMinInput();
    //vector<double> getMinOutput();

    /**
     * Set/Get mean and std input/output for standardization
     */
    void setMeanInput(vector<double> pV);
    void setMeanOutput(vector<double> pV);
    void setStdInput(vector<double> pV);
    void setStdOutput(vector<double> pV);
    //vector<double> getMeanInput();
    //vector<double> getMeanOutput();
    //vector<double> getStdInput();
    //vector<double> getStdOutput();


protected:
    unsigned int M; // number of input
    unsigned int K; // number of output
    unsigned int N; // policy architecture

    // function input/output normalization
    vector<double> input_max;
    vector<double> output_max;
    vector<double> input_min;
    vector<double> output_min;
    // function input/output standardization
    vector<double> input_mean;
    vector<double> output_mean;
    vector<double> input_std;
    vector<double> output_std;

};
}

#endif
