// ESE-549 Project 1 Reference Solution
// Peter Milder


#include <iostream>
#include <fstream> 
#include <vector>
#include <time.h>
#include "parse_bench.tab.h"
#include "ClassCircuit.h"
#include "ClassGate.h"

using namespace std;

// Just for the parser
extern "C" int yyparse();
extern FILE *yyin; // Input file for parser

// Our circuit. We declare this external so the parser can use it more easily.
extern Circuit* myCircuit;

vector<char> constructInputLine(string line);


//----------------

// Prototypes for new functions I added. Function bodies are below.

void simGateValue(Gate* g);
char evalGate(vector<char> in, int c, int i);
char EvalXORGate(vector<char> in, int inv);
int LogicNot(int logicVal);

//-----------------


int main(int argc, char* argv[]) {

  // Check the command line input and usage
  if (argc != 4) {
    cout << "Usage: ./logicsim circuit_file input_vectors output_location" << endl;
    return 1;
  }

  // Parse the bench file and initialize the circuit. (Using C style for our parser.)
  FILE *benchFile = fopen(argv[1], "r");
  if (benchFile == NULL) {
    cout << "ERROR: Cannot read file " << argv[1] << " for input" << endl;
    return 1;
  }
  yyin=benchFile;
  yyparse();
  fclose(benchFile);

  myCircuit->setupCircuit(); 
  cout << endl;

  
  // Setup the input vector file 
  vector<vector<char> > inputValues;
  ifstream inputStream;
  inputStream.open(argv[2]);
  string inputLine;
  if (!inputStream.is_open()) {
    cout << "ERROR: Cannot read file " << argv[2] << " for input" << endl;
    return 1;
  }

  // Setup the output text file
  ofstream outputStream;
  outputStream.open(argv[3]);
  if (!outputStream.is_open()) {
    cout << "ERROR: Cannot open file " << argv[3] << " for output" << endl;
    return 1;
  }
  
  // Try to read a line of inputs from the file.
  while(getline(inputStream, inputLine)) {

    
    // clear logic values of my circuit
    myCircuit->clearGateValues();

    // set new logic values
    myCircuit->setPIValues(constructInputLine(inputLine));


    // I have written a recursive function called simGateValue(Gate* g);
    // which takes as input a gate, and simulates to find its value.
    // If the gate's input values are not known, it recurses on the
    // gates which drive the inputs of g to first determine their values.

    // We start from the POs of the circuit, and call simGateValue() on
    // each of them.
    
    vector<Gate*> circuitPOs = myCircuit->getPOGates();
    for (int i=0; i < circuitPOs.size(); i++) {
      simGateValue(circuitPOs[i]);
    }


    // ---------------------------------------------
    
    // Write the results we just simulated to the output file
    vector<Gate*> outputGates = myCircuit->getPOGates();
    for (int i=0; i < outputGates.size(); i++) {
      outputStream << outputGates[i]->printValue();
    }
    outputStream << endl;
    
  }
  
  // close the input and output streams
  outputStream.close();
  inputStream.close();

  return 0;
}


vector<char> constructInputLine(string line) {
  
  vector<char> inputVals;
  
  for (int i=0; i<line.size(); i++) {
    if (line[i] == '0') 
      inputVals.push_back(LOGIC_ZERO);
    
    else if (line[i] == '1') 
      inputVals.push_back(LOGIC_ONE);

    else if ((line[i] == 'X') || (line[i] == 'x'))
      inputVals.push_back(LOGIC_X);

    else if ((line[i] == 'D') || (line[i] == 'd'))
      inputVals.push_back(LOGIC_D);

    else if ((line[i] == 'B') || (line[i] == 'b'))
      inputVals.push_back(LOGIC_DBAR);
        
    else {
      cout << "ERROR: Do not recognize character " << line[i] << " in line " << i+1 << " of input vector file. Setting to X" << endl;
      inputVals.push_back(LOGIC_X);
    }
  }
  
  return inputVals;

}

/////////////////////////////////
// Functions from reference solution


// Recursive function to find and set the value on Gate* g.

void simGateValue(Gate* g) {

  // If this gate has an already-set value, you are done.
  if (g->getValue() != LOGIC_UNSET)
    return;
  
  // Recursively call this function on this gate's predecessors to
  // ensure that their values are known.
  vector<Gate*> pred = g->get_gateInputs();
  for (int i=0; i<pred.size(); i++) {
    simGateValue(pred[i]);
  }
  
  // For convenience, create a vector of the values of this
  // gate's inputs.
  vector<char> inputVals;   
  for (int i=0; i<pred.size(); i++) {
    inputVals.push_back(pred[i]->getValue());      
  }

  char gateType = g->get_gateType();

  char gateValue;

  // Now, set the value of this gate based on its logical function and its input values
  switch(gateType) {   
  case GATE_NAND: { gateValue = evalGate(inputVals, 0, 1); break; }
  case GATE_NOR: { gateValue = evalGate(inputVals, 1, 1); break; }
  case GATE_AND: { gateValue = evalGate(inputVals, 0, 0); break; }
  case GATE_OR: { gateValue = evalGate(inputVals, 1, 0); break; }
  case GATE_BUFF: { gateValue = inputVals[0]; break; }
  case GATE_NOT: { gateValue = LogicNot(inputVals[0]); break; }
  case GATE_XOR: { gateValue = EvalXORGate(inputVals, 0); break; }
  case GATE_XNOR: { gateValue = EvalXORGate(inputVals, 1); break; }
  case GATE_PI: {gateValue = LOGIC_UNSET; break; }
  default: { cout << "ERROR: Do not know how to evaluate gate type " << gateType << endl; assert(false);}
  }    

  // After I have calculated this gate's value, set it on the data structure.
  g->setValue(gateValue);
  
}


//--------
// Evaluate a gate value (for non-XOR gates).
//    vector<int> in: logic values of the gate's inputs
//    int c: the controlling value of this gate (e.g., 0 for AND, 1 for OR)
//    int i: 1 if this gate is inverting (e.g., NAND), 0 if not inverting (e.g., AND)
char evalGate(vector<char> in, int c, int i) {

  // Are any of the inputs of this gate the controlling value?
  bool anyC = find(in.begin(), in.end(), c) != in.end();
  
  // Are any of the inputs of this gate unknown?
  bool anyUnknown = (find(in.begin(), in.end(), LOGIC_X) != in.end());

  // Are any inputs D or Dbar?
  int anyD    = find(in.begin(), in.end(), LOGIC_D)    != in.end();
  int anyDBar = find(in.begin(), in.end(), LOGIC_DBAR) != in.end();



  // if any input is c or we have both D and D', then return c^i
  if ((anyC) || (anyD && anyDBar))
    return (i) ? LogicNot(c) : c;

  // else if any input is unknown, return unknown
  else if (anyUnknown)
    return LOGIC_X;

  // else if any input is D, return D^i
  else if (anyD)
    return (i) ? LOGIC_DBAR : LOGIC_D;

  // else if any input is D', return D'^i
  else if (anyDBar)
    return (i) ? LOGIC_D : LOGIC_DBAR;

  // else return ~(c^i)
  else
    return LogicNot((i) ? LogicNot(c) : c);
}


// Evaluate an XOR or XNOR gate
//    vector<int> in: logic values of the gate's inputs
//    int inv: 1 if this gate is inverting (XNOR), 0 if not inverting (XOR)
char EvalXORGate(vector<char> in, int inv) {

  // if any unknowns, return unknown
  bool anyUnknown = (find(in.begin(), in.end(), LOGIC_X) != in.end());
  if (anyUnknown)
    return LOGIC_X;


  // Otherwise, let's count the numbers of ones and zeros for faulty and fault-free circuits.
  // It's not required for your project, but this will work with with XOR and XNOR with > 2 inputs.
  int onesFaultFree = 0;
  int onesFaulty = 0;

  for (int i=0; i<in.size(); i++) {
    switch(in[i]) {
    case LOGIC_ZERO: {break;}
    case LOGIC_ONE: {onesFaultFree++; onesFaulty++; break;}
    case LOGIC_D: {onesFaultFree++; break;}
    case LOGIC_DBAR: {onesFaulty++; break;}
    default: {cout << "ERROR: Do not know how to process logic value " << in[i] << " in Gate::EvalXORGate()" << endl; return LOGIC_X;}
    }
  }
  
  int XORVal;

  if ((onesFaultFree%2 == 0) && (onesFaulty%2 ==0))
    XORVal = LOGIC_ZERO;
  else if ((onesFaultFree%2 == 1) && (onesFaulty%2 ==1))
    XORVal = LOGIC_ONE;
  else if ((onesFaultFree%2 == 1) && (onesFaulty%2 ==0))
    XORVal = LOGIC_D;
  else
    XORVal = LOGIC_DBAR;

  return (inv) ? LogicNot(XORVal) : XORVal;

}


// A quick function to do a logical NOT operation on the LOGIC_* macros
int LogicNot(int logicVal) {
  if (logicVal == LOGIC_ONE)
    return LOGIC_ZERO;
  if (logicVal == LOGIC_ZERO)
    return LOGIC_ONE;
  if (logicVal == LOGIC_D)
    return LOGIC_DBAR;
  if (logicVal == LOGIC_DBAR)
    return LOGIC_D;
  if (logicVal == LOGIC_X)
    return LOGIC_X;
      
  cout << "ERROR: Do not know how to invert " << logicVal << " in LogicNot(int logicVal)" << endl;
  return LOGIC_UNSET;
}
