#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <vector>
#include <utility> // for std::pair
#include <algorithm> // for std::for_each

using namespace std;

struct Component {
    string input1,input2;
    string outputExpression;
    int delayPs;
    string gateType;
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
bool evaluateExpression(bool input1, bool input2, const string& gate) {
    if (gate == "AND2") {
        return input1 && input2;
    } else if (gate == "OR2") {
        return input1 || input2;
    } else if (gate == "NAND2") {
        return !(input1 && input2);
    } else if (gate == "NOR2") {
        return !(input1 || input2);
    } else if (gate == "XOR2") {
        return (input1 || input2) && !(input1 && input2);
    } else if (gate == "NOT") {
        return !input1;
    } else {
        throw std::invalid_argument("Invalid gate type: " + gate);
    }
}

// Function to read library file and populate components map
void readLibraryFile(const string& filePath, unordered_map<string, Component>& components, vector<string>& inputs,unordered_map<string, int> &delays) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening library file: " << filePath << endl;
        return;
    }

    string line;
    int i=0;

    while (getline(file, line)) {
        stringstream ss(line);
        string name, numInputsStr, outputExpr, delayStr,in1,in2,gateName;
        if (getline(ss, name, ',') && getline(ss, numInputsStr, ',') &&
            getline(ss, outputExpr, ',') && getline(ss, delayStr, ',')) {
            try {
                int numInputs = safe_stoi(numInputsStr);
                int delay = safe_stoi(delayStr);
                delays[name]=delay;
               // cout<<delay;

gateName = "G" + std::to_string(i);

                components[gateName] = { in1,in2,outputExpr, delays[name],name};
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

            components[gateName] = {input1, input2, outputName, delays[gateType],gateType};
//cout<<components[gateName].input1<<endl;
            // Check if gateType exists in components map
            if (!input1.empty()) {
                dependencies[outputName].insert(input1);
            }
            if (!input2.empty()) {
                dependencies[outputName].insert(input2);
            }

            if (components.find(gateName) != components.end()) {
                // Use gate's output name as key and gate's type as value
                gates[outputName] = gateName;
            } else {
                cerr << "Gate type '" << gateName << "' not found in components map." << endl;
            }
        }
    }
    file.close();
    for (const auto& [output, inputs] : dependencies) {
        cout << "Component '" << output << "' depends on: ";
        for (const auto& input : inputs) {
            cout << input << " ";
        }
        cout << endl;
    }
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
    unordered_map<string, int> lastInputValues;
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



        // Update input values if provided in the simulation inputs
        for (const auto& input : variables) {
          //  cout<<input.first<<input.second;
            const string& inputName = input.first;
            lastInputValues[inputName] = input.second;

        }

        // Output the values of inputs
        for (const auto& input : inputs) {

            int inputValue = lastInputValues[input];

            outputFile << timestamp << "," << input << "," << inputValue << endl;
        }

        int  time;
        // Evaluate gate outputs in the order they appear in the circuit file
        for ( auto& componentName : componentOrder) {
            Component component;
             string outputName = componentName;
           //cout<<outputName<<endl;
             string gateType = gates.at(componentName);
            cout<<gateType;
            auto it = components.find(gateType);
            if (it != components.end()) {
                 component = it->second;
                cout<<component.gateType<<" ";
                 cout<<component.delayPs<<" ";
               //  cout<<component.outputExpression;
            } else {
                // Handle the case where `gateType` is not found in components
                cerr << "Gate type '" << gateType << "' not found in components map." << endl;
            }

            // Get input values for the gate
            // cout<<"in "<<component.input2<<" ";
            bool input1Value = lastInputValues[component.input1];

            bool input2Value = lastInputValues[component.input2];

            // Evaluate gate's expression based on the last known input values
            bool result = evaluateExpression(input1Value, input2Value, component.gateType);


            if (dependencies.count(component.input1) > 0 || dependencies.count(component.input2) > 0) {
                // Calculate delay for the current component considering dependencies
                timestamp += component.delayPs;


            // Calculate delay for the current component considering dependencies



            // Output the value of the gate with delay
            outputFile << timestamp << "," << outputName << "," << result << endl;

            // Update the last input values with the calculated input
            lastInputValues[outputName] = result;
        }
    else{   time=timestamp + component.delayPs;


        // Output the value of the gate with delay
        outputFile << time << "," << outputName << "," << result << endl;

        // Update the last input values with the calculated input
        lastInputValues[outputName] = result;
                outputNames.insert(component.outputExpression);
    } }}



    outputFile.close();
}


int main() {
    string folderPath = "D:\\spring 24\\DD1 project\\Logic-Circuit-Simulator-DD1\\Test_Case_1"; // Update with the path to your folder containing input files
    vector<string> inputs;
    unordered_map<string, Component> components;
    unordered_map<string, string> gates;
    vector<VariableValues> simInputs;
    unordered_map<string, int> delays;;
    unordered_map<string, unordered_set<string>> dependencies;

    readLibraryFile(folderPath + "/T1.lib", components, inputs,delays);
    readCircuitFile(folderPath + "/T1.cir", components, gates, inputs,delays,dependencies);
    readSimulationFile(folderPath + "/T1.stim", simInputs);

    generateSimulationOutput(gates, components, simInputs, folderPath + "/T1.sim",inputs,dependencies);

    return 0;
}

