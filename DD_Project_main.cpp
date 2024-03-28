#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <vector>
#include <algorithm> // for std::for_each
#include <stack>
using namespace std;

struct Component {
    vector<string>inputs;
    string outputExpression;
    int delayPs;
    string gateType;
    int numofinputs;
};

struct VariableValues {
    int timestamp;
    unordered_map<string, int> variables;
};

// Function to safely convert string to integer
int safe_stoi(const std::string& str) {
    if (str.empty()) {
        throw std::invalid_argument("Empty string");
    }
    for (char c : str) {
        if (!std::isdigit(c)) {
            throw std::invalid_argument("Non-integer character");
        }
    }
    return std::stoi(str);
}

// Function to trim whitespace from both ends of a string
string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == string::npos || end == string::npos) {
        return ""; // Empty or whitespace-only string
    }
    return str.substr(start, end - start + 1);
}


// Function to evaluate boolean expression


using namespace std;

bool evaluateExpression( string& expression,vector<bool>& inputs) {
    stack<bool> operands;
    stack<char> operators;
    for (int i = 0; i < expression.size(); i++) {
        size_t pos = expression.find("i" + to_string(i + 1));
        while (pos != string::npos) {
            string x;
            x=char(65+i);
            expression.replace(pos, 2, x);
            pos = expression.find("i" + to_string(i + 1), pos + 2);
        }
    }
    for (char c : expression) {
        int i=0;
        if (isalpha(c)) {
            operands.push(inputs[i]);
            i++;
        } else if (c == '(') {
            operators.push(c);
        } else if (c == ')') {
            while (!operators.empty() && operators.top() != '(') {
                char op = operators.top();
                operators.pop();
                if (op == '~') {
                    bool operand = operands.top();
                    operands.pop();
                    operands.push(!operand);
                } else if (op == '|') {
                    bool operand2 = operands.top();
                    operands.pop();
                    bool operand1 = operands.top();
                    operands.pop();
                    operands.push(operand1 || operand2);
                } else if (op == '&') {
                    bool operand2 = operands.top();
                    operands.pop();
                    bool operand1 = operands.top();
                    operands.pop();
                    operands.push(operand1 && operand2);
                }
            }
            if (!operators.empty()) {
                operators.pop(); // Remove '(' from stack
            }
        } else {
            operators.push(c);
        }
    }

    while (!operators.empty()) {
        char op = operators.top();
        operators.pop();
        if (op == '~') {
            bool operand = operands.top();
            operands.pop();
            operands.push(!operand);
        } else if (op == '|') {
            bool operand2 = operands.top();
            operands.pop();
            bool operand1 = operands.top();
            operands.pop();
            operands.push(operand1 || operand2);
        } else if (op == '&') {
            bool operand2 = operands.top();
            operands.pop();
            bool operand1 = operands.top();
            operands.pop();
            operands.push(operand1 && operand2);
        }
    }

    return operands.top();
}







// Function to read library file and populate components map
void readLibraryFile(const string& filePath, unordered_map<string, Component>& components,unordered_map<string, int> &delays) {
    string line;
    int i=0;
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening library file: " << filePath << endl;
        return;
    }
    while (getline(file, line)) {
        stringstream ss(line);
        string name, numInputsStr, outputExpr, delayStr,in1,in2,gateName;
        if (getline(ss, name, ',') && getline(ss, numInputsStr, ',') &&
            getline(ss, outputExpr, ',') && getline(ss, delayStr, ',')) {
            try {
                int delay = safe_stoi(delayStr);
                delays[name]=delay;
                gateName = "G" + std::to_string(i);
                components[gateName].inputs.push_back(in1);
                components[gateName].inputs.push_back(in2);
                components[gateName].numofinputs= safe_stoi(numInputsStr);
                components[gateName].outputExpression = outputExpr;
                components[gateName].delayPs=delays[name];
                components[gateName].gateType=name;

                i++;
            } catch (const std::invalid_argument& e) {
                cerr << "Invalid argument in line: " << line << endl;
            }
        }
    }
    file.close();
}

// Function to read circuit file and populate gates map

void readCircuitFile(const string& filePath, unordered_map<string, Component>& components,
                     unordered_map<string, string>& gates, vector<string>& inputs,unordered_map<string, int> delays,    unordered_map<string, unordered_set<string>> &dependencies
) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening circuit file: " << filePath << endl;
        return;
    }

    string line;
    bool readingInputs = false;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        if (line == "INPUTS:") {
            readingInputs = true;
            continue;
        } else if (line == "COMPONENTS:") {
            readingInputs = false;
            continue;
        }

        if (readingInputs) {
            // Store input variable
            inputs.push_back(line);
        } else {
            stringstream ss(line);

            string gateName, gateType, outputName, input1, input2;
            getline(ss, gateName, ',');
            getline(ss, gateType, ',');
            gateType = trim(gateType);
            if (gateType == "NOT") {
                // NOT gate only has one input
                getline(ss, outputName, ',');
                getline(ss, input1, ',');

            } else {
                getline(ss, outputName, ',');
                getline(ss, input1, ',');
                getline(ss, input2, ',');
            }

            gateName = trim(gateName);
            gateType = trim(gateType);
            outputName = trim(outputName);
            input1 = trim(input1);
            input2 = trim(input2);

            // Store the inputs for each gate in the components map

            components[gateName].inputs.push_back(input1);
            components[gateName].inputs.push_back(input2);
            components[gateName].delayPs = delays[gateType];
            components[gateName].gateType = gateType;


for(int i=0;i<components[gateName].numofinputs;i++) {
    // Check if gateType exists in components map

    dependencies[outputName].insert(components[gateName].inputs.at(i));
}

            if (components.find(gateName) != components.end()) {
                // Use gate's output name as key and gate's name as value
                gates[outputName] = gateName;
            } else {
                cerr << "Gate type '" << gateName << "' not found in components map." << endl;
            }}

    }
    file.close();
}


// Function to read simulation file and populate simInputs vector
void readSimulationFile(const string& filePath, vector<VariableValues>& simInputs) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening simulation file: " << filePath << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string timeStr, variable, valueStr;
        if (getline(ss, timeStr, ',') && getline(ss, variable, ',') && getline(ss, valueStr, ',')) {
            try {
                int time = stoi(timeStr);
                int value = stoi(valueStr);

                // Check if there's already a VariableValues entry for this timestamp
                bool found = false;
                for (auto& vv : simInputs) {
                    if (vv.timestamp == time) {
                        vv.variables[variable] = value;
                        found = true;
                        break;
                    }
                }

                // If not found, create a new VariableValues entry
                if (!found) {
                    VariableValues vv;
                    vv.timestamp = time;
                    vv.variables[variable] = value;
                    simInputs.push_back(vv);
                }
            } catch (const std::invalid_argument& e) {
                cerr << "Invalid argument in line: " << line << endl;
            }
        }
    }
    file.close();
}

void generateSimulationOutput(const unordered_map<string, string>& gates,  unordered_map<string, Component> components,
                              vector<VariableValues>& simInputs, const string& outputPath, const vector<string>& inputs, unordered_map<string, unordered_set<string>> dependencies) {
    ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        cerr << "Error opening output file: " << outputPath << endl;
        return;
    }

    // Initialize input values to 0
    unordered_map<string, bool> lastInputValues;
    for (const auto& input : inputs) {
        lastInputValues[input] = 0;
    }

    // Vector to store the order of components
    vector<string> componentOrder;

    std::vector<std::string> keys;
    keys.reserve(gates.size());
    for (const auto& gate : gates) {
        keys.push_back(gate.first);

    }

// Iterate over keys vector in reverse and populate componentOrder in reverse
    std::for_each(keys.rbegin(), keys.rend(), [&componentOrder](const auto& key) {
        componentOrder.push_back(key);

    });
    // Loop through each simulation input
    for (const auto& simInput : simInputs) {
        int timestamp = simInput.timestamp;
        const unordered_map<string, int>& variables = simInput.variables;

        unordered_set<string> outputNames;

        vector<bool>inputsvalues;
inputsvalues.clear();
        // Update input values if provided in the simulation inputs
        for (const auto& input : variables) {

            const string& inputName = input.first;
            lastInputValues[inputName] = input.second;

        }

        // Output the values of inputs
        for (const auto& input : inputs) {

            int inputValue = lastInputValues[input];

            outputFile << timestamp << "," << input << "," << inputValue << endl;
        }

        int  time;

        for ( auto& componentName : componentOrder) {
            Component component;
             string outputName = componentName;
             string gateType = gates.at(componentName);
            auto it = components.find(gateType);
            if (it != components.end()) {
                 component = it->second;


            } else {
                // Handle the case where `gateType` is not found in components
                cerr << "Gate type '" << gateType << "' not found in components map." << endl;
            }

            // Get input values for the gate
            for(int i=0;i<component.inputs.size();i++){

                inputsvalues.push_back(lastInputValues[component.inputs.at(i)]);
            }



            // Evaluate gate's expression based on the last known input values
            bool result = evaluateExpression(component.outputExpression,inputsvalues);

            bool dependent =false;

            for(int i = 0; i < component.numofinputs; ++i) {
                if (dependencies[outputName].count(component.inputs[i])>0) {

                    // Calculate delay for the current component considering dependencies
                    dependent=true;

                }
            }
            if(dependent){
                timestamp += component.delayPs;

                outputFile << timestamp << "," << outputName << "," << result << endl;
            }
            else{
            time=timestamp+component.delayPs;
            outputFile << time << "," << outputName << "," << result << endl;}
            lastInputValues[outputName] = result;
            outputNames.insert(outputName);
// Output the value of the gate with delay


// Update the last input values with the calculated input


        } }



    outputFile.close();
}




int main() {

    string folderPath;
    string lib;
    string circ;
    string stim;
    string sim;
    vector<int> timestamps;

    cout<<"Enter library file path: ";
    getline(cin,lib);
    cout<<"Enter Circuit file path: ";
  getline(cin,circ);
    cout<<"Enter stim file path: ";
    getline(cin,stim);
    cout<<"Enter sim file path: ";
    getline(cin,sim);

    vector<string> inputs;
    unordered_map<string, Component> components;
    unordered_map<string, string> gates;
    vector<VariableValues> simInputs;
    unordered_map<string, int> delays;;
    unordered_map<string, unordered_set<string>> dependencies;

    readLibraryFile(lib, components,delays);
    readCircuitFile(circ, components, gates, inputs,delays,dependencies);
    readSimulationFile(stim, simInputs);

    generateSimulationOutput(gates, components, simInputs,   sim,inputs,dependencies);


    return 0;
}

