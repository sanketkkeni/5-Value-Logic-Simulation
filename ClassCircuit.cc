
 /** \class Circuit
 * \brief A circuit contains primary inputs and outputs, and a number of interconnected gates.
 *
 * The API for interacting with a Circuit includes functions to access the gates in the
 * system. Functions you may use in your project include \a getPIGates() and \a getPOGates(), which
 * return the inputs and outputs of the circuit. Also useful are \a setPIValues(), which the reference
 * code uses to set the values of the inputs of the circuit, and \a clearGateValues() which it uses
 * to clear the logical values after each simulation iteration.
 *
 * An important thing to understand: we will use a special Gate type called a PI to represent
 * the circuit's primary inputs. This isn't really a logic gate, it's just a way of 
 * making it easy to access the inputs. So, when you call \a getPIGates() you will get a vector of
 * pointers to the special PI gates.
 * 
 * We don't use a special structure for the primary outputs (POs); instead we simply maintain a 
 * list of which gates drive the output values. So when you call \a getPOGates() you will get
 * a vector of pointers to the normal gates that drive the outputs.
 * 
 * Lastly, note that there are a number of functions here that are only used when the initial 
 * representation of the circuit is constructed. (This is done for you by the yyparse() function 
 * (see main.cc). These functions are ones that you will never have to manipulate yourself, and 
 * they are marked with a note that indicate this.
 */

#include "ClassCircuit.h"

/** \brief Construct a new circuit */
Circuit::Circuit() {}

/** \brief Add a new gate to the circuit
 *  \param name a string providing the output name for the gate
 *  \param ID a unique ID number for the gate (primarily used while parsing the input source file)
 *  \param gt the type of gate, using the GATE_* marcos defined in ClassGate.h
 *  \note This function should only need to be run by the parser.
 */
void Circuit::newGate(string name, int ID, int gt) {
    Gate* g = new Gate(name, ID, gt);
    gates.push_back(g);

    if (gt == GATE_PI)
      inputGates.push_back(g);
}

/** \brief Get pointer to gate \a i from the circuit
 *  \param i gate number
 *  \return Pointer to Gate \a i
 */
Gate* Circuit::getGate(int i) {
  if (i >= gates.size()) {
    cout << "ERROR: Requested gate out of bounds" << endl;
    assert(false);
  }
  return gates[i];
}

/** \brief Record the name of a primary output of this circuit.
 *  \param n Name of the output signal
 *  \note This function should only need to be run by the parser.
 */
void Circuit::addOutputName(string n) {
  outputNames.push_back(n);
}

/** \brief Print the circuit
 */
void Circuit::printAllGates() {
  cout << "Inputs: ";
  for (int i=0; i<inputGates.size(); i++)
    cout << inputGates[i]->get_outputName() << " ";
  cout << endl;

  cout << "Outputs: ";
  for (int i=0; i<outputGates.size(); i++)
    cout << outputGates[i]->get_outputName() << " ";
  cout << endl;
  
  for (int i=0; i<gates.size(); i++) {
    gates[i]->printGateInfo();
  }

}  

/** \brief Returns a pointer to the Gate in this circuit with output name \a name.
 *  \param name A string containing the name of the gate requested
 *  \return Pointer to the Gate with output given by \a name
 */
Gate* Circuit::findGateByName(string name) {
  for (int i=0; i<gates.size(); i++) {
    if (gates[i]->get_outputName().compare(name) == 0)
      return gates[i];
  }
  
  cout << "ERROR: Cannot find gate with name " << name << endl;
  assert(false);
}

/** \brief Sets up the circuit data structures after parsing is complete.
 *  Run this once after parsing, before using the data structure.
 *  The handout \a main.cc code already does this; you do not need to add it yourself.
 */
void Circuit::setupCircuit() {

  // set-up the vector of output gates based on their pre-stored names
  for (int i=0; i<outputNames.size(); i++) {
    outputGates.push_back(findGateByName(outputNames[i]));
  }

  // set input and output pointers of each gate
  for (int i=0; i<gates.size(); i++) {
    Gate* g = gates[i];
    vector<string> names = g->get_gateInputNames();
    for (int j=0; j<names.size(); j++) {
      string n = names[j];
      Gate* inGate = findGateByName(n);
      inGate->set_gateOutput(g);
      g->set_gateInput(inGate);
    }
  }
  
}

/** \brief Initializes the values of the PIs of the circuit.
 *  \param inputVals the desired input values (using LOGIC_* macros).
 */
void Circuit::setPIValues(vector<char> inputVals) {
  if (inputVals.size() != inputGates.size()) {
    cout << "ERROR: Incorrect number of input values: " << inputVals.size() << " vs " << inputGates.size() << endl;
    assert(false);
  }

  for (int i=0; i<inputVals.size(); i++)
    inputGates[i]->setValue(inputVals[i]);    

}

/** \brief Returns the output values on each PO of this circuit.
 */
vector<int> Circuit::getPOValues() { 
  vector<int> outVals;
  for (int i=0; i < outputGates.size(); i++) 
    outVals.push_back(outputGates[i]->getValue());

  return outVals;
}

/** \brief Get the number of PIs of the circuit.
 *  \return The number of PIs of the circuit. */
int Circuit::getNumberPIs() { return inputGates.size(); }

/** \brief Get the number of POs of the circuit.
 *  \return The number of POs of the circuit. */
int Circuit::getNumberPOs() { return outputGates.size(); }

/** \brief Get the number of gates of the circuit.
 *  \return The number of gates of the circuit.
 */
int Circuit::getNumberGates() { return gates.size(); }

/** \brief Clears the value of each gate in the circuit (to LOGIC_UNSET) */
void Circuit::clearGateValues() {
  for (int i=0; i<gates.size(); i++)
    gates[i]->setValue(LOGIC_UNSET);
}

/** \brief Returns the PI (input) gates. (The PIs of the circuit).
    \return a \a vector<Gate*> of the circuit's PIs */
vector<Gate*> Circuit::getPIGates() { return inputGates; }

/** \brief Returns the PO (output) gates. (The gates which drive the POs of the circuit.)
    \return a \a vector<Gate*> of the circuit's POs */
vector<Gate*> Circuit::getPOGates() { return outputGates; }
