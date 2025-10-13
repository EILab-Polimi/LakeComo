//============================================================================
// Name        : main_lakecomo.cpp
// Author      : MatteoG, DavideS
// Version     : 
// Copyright   : Your copyright notice
//============================================================================


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <fstream>
#include "../src/model_lakecomo_nemodps.h"
#include "../src/moeaframework.h"

#include <time.h>

using namespace std;


int main(int argc, char* argv[]) {

    // user input file
    string input_file = argv[1];

    // Lake Como Problem
    model_lakecomo LakeComoProblem(input_file);
    int nvars = LakeComoProblem.getNvar();
    int nobjs = LakeComoProblem.getNobj();
    double vars[nvars];
    double objs[nobjs];


    // Dummy input, output and hidden nodes for NEMODPS with 3 inputs, 1 output and 1 hidden node
    vector<int> input_nodes = {-1, -2, -3, -4, -5, -6};
    vector<int> output_nodes = {0};

    Node hidden_node = Node(); // A single hidden node with id=1, receiving from all 6 inputs except the fourth
    hidden_node.id = 1;
    hidden_node.links.push_back(make_pair(-1, 0.5)); // link from input node -1 with weight 0.5
    hidden_node.links.push_back(make_pair(-2, 0.3)); // link from input node -2 with weight 0.3
    hidden_node.links.push_back(make_pair(-3, 0.2)); // link from input node -3 with weight 0.2
    hidden_node.links.push_back(make_pair(-5, 0.4)); // link from input node -5 with weight 0.4
    hidden_node.links.push_back(make_pair(-6, 0.6)); // link from input node -6 with weight 0.6

    Node output_node = Node(); // A single output node with id=0, receiving from the hidden node
    output_node.id = 0;
    output_node.links.push_back(make_pair(1, 1.0)); // link from hidden node 1 with weight 1.0

    vector<Node> hidden_nodes = {hidden_node, output_node}; // hidden and output nodes


    // optimization
    clock_t start, end;
    start = clock();

    MOEA_Init(nobjs, 0);
    while (MOEA_Next_solution() == MOEA_SUCCESS) {
        MOEA_Read_doubles(nvars, vars);
//        LakeComoProblem.evaluate(vars, objs);
        LakeComoProblem.evaluate_with_nemodps(vars, objs, input_nodes, output_nodes, hidden_nodes);
        MOEA_Write(objs, NULL);
    }
    end = clock();
    cout << "time elapsed: " << ((end - start)/double(CLOCKS_PER_SEC)) << " seconds" << endl;

    return EXIT_SUCCESS;

}
