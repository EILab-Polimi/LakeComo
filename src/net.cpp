/*
 * net.cpp
 *
 *  Created on: 02/aug/2024
 *      Author: DavideS
 */

#include "net.h"

using namespace std;
using func_act = function<float(float)>;
using func_agg = function<float(vector<float>)>;

net::net()
{
}

net::~net(){
}


net::net(unsigned int pM, unsigned int pK){
    M = pM;
    K = pK;
    values.clear();
    for(unsigned int i = 0; i < M+K; i++){
        values[i] = 0.0f;
    }
}

float identity_function(float x) {
    return x;
}

float linear_function(float x) {
    return x;
}

float linear_piece_function(float x) {
    return x < 0 ? 0 : x > 1 ? 1 : x;
}

float linear_piece_symmetric_function(float x) {
    return x < -1 ? -1 : x > 1 ? 1 : x;
}

float threshold_function(float x) {
    return x < 0 ? 0 : 1;
}

float threshold_symmetric_function(float x) {
    return x < 0 ? -1 : 1;
}

float sigmoid_function(float x) {
    return 1/(1+exp(-x));
}

float sigmoid_symmetric_function(float x) {
    return 2/(1+exp(-2*x))-1;
}

float gaussian_function(float x) {
    return exp(-x*x);
}

float gaussian_symmetric_function(float x) {
    return 2*exp(-x*x)-1;
}

float elliot_function(float x) {
    return 0.5f * x / (1 + abs(x)) + 0.5f;
}

float softsign_function(float x) {
    return x/(1+abs(x));
}

float sin_function(float x) {
    return sin(x)/2+0.5f;
}

float cos_function(float x) {
    return cos(x)/2+0.5f;
}

float sin_symmetric_function(float x) {
    return sin(x);
}

float cos_symmetric_function(float x) {
    return cos(x);
}

float relu_function(float x) {
    return x < 0 ? 0 : x;
}

float linear_piece_rect_function(float x) {
    return x < 0 ? 0 : x;
}

float linear_piece_rect_leaky_function(float x) {
    return x < 0 ? 0.01f*x : x;
}

float h_tangent_function(float x) {
    return 0.5f*tanh(0.5f*x)+0.5f;
}

float h_tangent_symmetric_function(float x) {
    return tanh(x);
}


map<string, func_act> activationMap = {
        {"IDENTITY", identity_function},
        {"LINEAR", linear_function},
        {"LINEAR_PIECE", linear_piece_function},
        {"LINEAR_PIECE_SYMMETRIC", linear_piece_symmetric_function},
        {"THRESHOLD", threshold_function},
        {"THRESHOLD_SYMMETRIC", threshold_symmetric_function},
        {"SIGMOID", sigmoid_function},
        {"SIGMOID_SYMMETRIC", sigmoid_symmetric_function},
        {"GAUSSIAN", gaussian_function},
        {"GAUSSIAN_SYMMETRIC", gaussian_symmetric_function},
        {"ELLIOT", elliot_function},
        {"SOFTSIGN", softsign_function},
        {"SIN", sin_function},
        {"COS", cos_function},
        {"SIN_SYMMETRIC", sin_symmetric_function},
        {"COS_SYMMETRIC", cos_symmetric_function},
        {"RELU", relu_function},
        {"LINEAR_PIECE_RECT", linear_piece_rect_function},
        {"LINEAR_PIECE_RECT_LEAKY", linear_piece_rect_leaky_function},
        {"H_TANGENT", h_tangent_function},
        {"H_TANGENT_SYMMETRIC", h_tangent_symmetric_function}

    };


// Activation functions based on the FANN library https://github.com/libfann/fann
float act_function(string f, float x, float bias, float response) {
    if (f == "RBF")
        return exp(-(x-bias)*(x-bias)/(2*response*response));  // radial basis function
    else
        x = bias + response * x;  // ridge function
    // compute f(x) for functions (ridge functions) in the form y = bias + response * f(x)
    return activationMap[f](x);
}


float sum_function(vector<float> v) {
    return accumulate(v.begin(), v.end(), 0.0f);
}

float min_function(vector<float> v) {
    return *min_element(v.begin(), v.end());
}

float max_function(vector<float> v) {
    return *max_element(v.begin(), v.end());
}

float mean_function(vector<float> v) {
    return accumulate(v.begin(), v.end(), 0.0f) / v.size();
}

float median_function(vector<float> v) {
    sort(v.begin(), v.end());
    if (v.size() % 2 == 0) {
        return v[v.size() / 2] + v[(v.size() / 2) - 1] / 2;
    } else {
        return v[v.size() / 2];
    }
}

float product_function(vector<float> v) {
    return accumulate(v.begin(), v.end(), 1.0f, multiplies<float>());
}

map<string, func_agg> aggregationMap = {
        {"SUM", sum_function},
        {"MIN", min_function},
        {"MAX", max_function},
        {"MEAN", mean_function},
        {"MEDIAN", median_function},
        {"PRODUCT", product_function}
    };

float agg_function(string f, vector<float> v) {
    return aggregationMap[f](v);
}

void net::setParameters(double* pTheta){
    // Not implemented
}

void net::setParameters(vector<int> p_input_nodes, vector<int> p_output_nodes, vector<Node> p_hidden_nodes){
    input_nodes = p_input_nodes;
    M = input_nodes.size();
    output_nodes = p_output_nodes;
    K = output_nodes.size();
    hidden_nodes = p_hidden_nodes;
    N = hidden_nodes.size();
    values.clear();
    for(unsigned int i = 0; i < M+K; i++){
        values[i] = 0.0;
    }
}

void net::clearParameters(){
    input_nodes.clear();
    output_nodes.clear();
    hidden_nodes.clear();
    values.clear();
}


vector<double> net::get_output(vector<double> input){
    if (input.size() != M) {
        cout << "Expected " << M << " inputs, got " << input.size() << endl;
    }
    for(unsigned int i = 0; i < M; i++) {
        values[input_nodes[i]] = input[i];
    }

    for (Node node : hidden_nodes) {
        vector<float> node_inputs;
        if (node.act_s == "RBF") {
            // RBF as in EMODPS, with:
            //      sigma different for every node but constant for the same node (sigma different for every link in EMODPS)
            //      centers different for every link as in EMODPS
            float tmp;
            for (auto const& link : node.links) { // link.second is used as RBF center; node.response as only sigma
                node_inputs.push_back((values[link.first] - link.second) * (values[link.first] - link.second));
            }
//            node_inputs.push_back(node.bias);
            tmp = node.response * node.response;
            tmp = accumulate(node_inputs.begin(), node_inputs.end(), 0.0f) / (tmp < 1e-6 ? 1e-6 : tmp);
            values[node.id] = exp(-tmp);
        } else if (node.act_s == "RBF_old") {  // First implementation of RBF
            float tmp, num, den;
            for (auto const& link : node.links) {
                tmp = values[link.first] * link.second;
                num = (tmp-node.bias)*(tmp-node.bias);
                den = node.response*node.response;
                node_inputs.push_back(num / (den < 1e-6 ? 1e-6 : den));
            }
            tmp = agg_function(node.agg_s, node_inputs);
            values[node.id] = exp(-tmp);
        } else {
            for (auto const& link : node.links) {
                node_inputs.push_back(values[link.first] * link.second);
            }
            float s = agg_function(node.agg_s, node_inputs);
            values[node.id] = act_function(node.act_s, s, node.bias, node.response);
        }
    }

    input.clear();
    for (int i : output_nodes) {
        input.push_back(values[i]);
    }

    return input;
}


