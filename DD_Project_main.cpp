#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <unordered_set>
#include <stack>
using namespace std;

struct Component {
    int numInputs;
    string outputExpression;
    int delayPs;
};
struct gateinputs{
    int in1,in2;
};
struct variablesnames{
    string  v1,v2;
};

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

string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == string::npos || end == string::npos) {
        return ""; // Empty or whitespace-only string
    }
    return str.substr(start, end - start + 1);
}

bool evaluateExpression(const string& expression, bool x, bool y) {
    stack<char> operators;
    stack<bool> operands;

    for (char c : expression) {
        if (isspace(c)) {
            continue;
        } else if (isalpha(c)) {
            operands.push((c == 'A') ? x : y);
        } else if (c == '(') {
            operators.push(c);
        } else if (c == ')') {
            while (!operators.empty() && operators.top() != '(') {
                char op = operators.top();
                operators.pop();
                bool operand2 = operands.top();
                operands.pop();
                bool operand1 = operands.top();
                operands.pop();

                if (op == '&') {
                    operands.push(operand1 && operand2);
                } else if (op == '|') {
                    operands.push(operand1 || operand2);
                } else if (op == '~') {
                    operands.push(!operand2);
                }
            }
            operators.pop();
        } else if (c == '&' || c == '|' || c == '~') {
            while (!operators.empty() && operators.top() != '(' &&
                   ((c == '~' && operators.top() == '~') ||
                    (c != '~' && (operators.top() == '&' || operators.top() == '|')))) {
                char op = operators.top();
                operators.pop();
                bool operand2 = operands.top();
                operands.pop();
                bool operand1 = operands.top();
                operands.pop();

                if (op == '&') {
                    operands.push(operand1 && operand2);
                } else if (op == '|') {
                    operands.push(operand1 || operand2);
                } else if (op == '~') {
                    operands.push(!operand2);
                }
            }
            operators.push(c);
        }
    }

    while (!operators.empty()) {
        char op = operators.top();
        operators.pop();
        bool operand2 = operands.top();
        operands.pop();
        bool operand1 = operands.top();
        operands.pop();

        if (op == '&') {
            operands.push(operand1 && operand2);
        } else if (op == '|') {
            operands.push(operand1 || operand2);
        } else if (op == '~') {
            operands.push(!operand2);
        }
    }

    return operands.top();
}

void readLibraryFile(const string& filePath, unordered_map<string, Component>& components,
                     const unordered_set<string>& inputs) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening library file: " << filePath << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string name, numInputsStr, outputExpr, delayStr;
        if (getline(ss, name, ',') && getline(ss, numInputsStr, ',') &&
            getline(ss, outputExpr, ',') && getline(ss, delayStr, ',')) {
            try {
                int numInputs = safe_stoi(numInputsStr);
                int delay = safe_stoi(delayStr);

                // Replace placeholders with actual input variables from the circuit
                for (const auto& input : inputs) {
                    size_t pos = outputExpr.find(input);
                    while (pos != string::npos) {
                        outputExpr.replace(pos, input.size(), input);
                        pos = outputExpr.find(input, pos + input.size());
                    }
                }

                components[name] = {numInputs, outputExpr, delay};
            } catch (const std::invalid_argument& e) {
                cerr << "Invalid argument in line: " << line << endl;
            }
        }
    }
    file.close();
}

void readCircuitFile(const string& filePath, const unordered_map<string, Component>& components,
                     unordered_map<string, string>& gates, unordered_set<string>& inputs) {
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
            inputs.insert(line);
        } else {
            stringstream ss(line);
            string gateName, gateType, outputName;
            getline(ss, gateName, ',');
            getline(ss, gateType, ',');
            getline(ss, outputName, ',');

            string inputsStr;
            while (getline(ss, inputsStr, ',')) {
                gateName = trim(gateName);
                gateType = trim(gateType);
                outputName = trim(outputName);
                inputsStr = trim(inputsStr);

                if (components.find(gateType) != components.end()) {
                    // Use input as key and output as value
                    gates[inputsStr] = outputName;
                } else {
                    cerr << "Gate type '" << gateType << "' not found in components map." << endl;
                }
            }
        }
    }
    file.close();
}

void readSimulationFile(const string& filePath, unordered_map<int, unordered_map<string, int>>& simInputs) {
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
                int time = safe_stoi(timeStr);
                int value = safe_stoi(valueStr);
                simInputs[time][variable] = value;
            } catch (const std::invalid_argument& e)
            {
                cerr << "Invalid argument in line: " << line << endl;
            }
        }
    }
    file.close();
}


void generateSimulationOutput(const unordered_map<string, string>& gates, unordered_map<string, int>& lastInputValues, const unordered_map<int, unordered_map<string, int>>& simInputs, const string& outputPath) {
    ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        cerr << "Error opening output file: " << outputPath << endl;
        return;
    }

    for (const auto& simInput : simInputs) {
        int timestamp = simInput.first;
        const auto& variables = simInput.second;

        // Write inputs and default values
        for (const auto& gate : gates) {
            const string& varName = gate.first;
            if (variables.find(varName) == variables.end()) {
                if (lastInputValues.find(varName) != lastInputValues.end()) {
                    // Use last known value of input if not provided in this simulation
                    outputFile << timestamp << "," << varName << "," << lastInputValues[varName] << endl;
                } else {
                    outputFile << timestamp << "," << varName << ",0" << endl; // Default value is 0
                }
            }
        }

        // Evaluate gates
        for (const auto& variable : variables) {
            const string& varName = variable.first;
            int varValue = variable.second;

            // Check if the variable is present in the gates map
            if (gates.find(varName) != gates.end()) {
                const string& expression = gates.at(varName);
                // Evaluate the gate expression
                bool result = evaluateExpression(expression, varValue, lastInputValues["B"]); // Assuming "secondVariable" is the name of the second input variable
                // Output the result
                outputFile << timestamp << "," << varName << "," << (result ? 1 : 0) << endl; // Writing 1 or 0 for boolean result
            } else {
                // If the variable is not found in the gates map, output an error message
                cerr << "Variable '" << varName << "' not found in gates map." << endl;
            }
        }


        // Update last known values of inputs
        for (const auto& input : variables) {
            if (lastInputValues.find(input.first) != lastInputValues.end()) {
                lastInputValues[input.first] = input.second;
            }
        }
    }

    outputFile.close();
}



int main() {
    string folderPath = "D:\\spring 24\\DD1 project\\Logic-Circuit-Simulator-DD1\\Test_Case_5"; // Update with the path to your folder containing input files
    unordered_set<string> inputs;
    unordered_map<string, Component> components;
    readLibraryFile(folderPath + "/T5.lib", components,inputs);

    unordered_map<string, string> gates;
    unordered_map<string, int> lastInputValues;

    readCircuitFile(folderPath + "/T5.cir", components, gates,inputs);
    unordered_map<int, unordered_map<string, int>> simInputs;
    readSimulationFile(folderPath + "/T5.stim", simInputs);

    generateSimulationOutput(gates, lastInputValues, simInputs, folderPath + "/T5.sim");

    return 0;
}
