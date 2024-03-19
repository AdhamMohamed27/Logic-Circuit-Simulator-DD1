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
            inputs.insert(line); // Store input variable
        } else {
            stringstream ss(line);
            string gateName, gateType, outputName;
            getline(ss, gateName, ',');
            getline(ss, gateType, ',');
            getline(ss, outputName, ',');

            string inputs;
            while (getline(ss, inputs, ',')) {
                gateName = trim(gateName);
                gateType = trim(gateType);
                outputName = trim(outputName);

                if (components.find(gateType) != components.end()) {
                    gates[outputName] = gateName;
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
            } catch (const std::invalid_argument& e) {
                cerr << "Invalid argument in line: " << line << endl;
            }
        }
    }
    file.close();
}

void generateSimulationOutput(const unordered_map<string, string>& gates, const unordered_map<int, unordered_map<string, int>>& simInputs, const string& outputPath) {
    ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        cerr << "Error opening output file: " << outputPath << endl;
        return;
    }

    for (const auto& simInput : simInputs) {
        int timestamp = simInput.first;
        const auto& variables = simInput.second;

        for (const auto& gate : gates) {
            const string& varName = gate.first;
            if (variables.find(varName) == variables.end()) {
                outputFile << timestamp << "," << varName << ",0" << endl; // Default value is 0
            }
        }

        for (const auto& variable : variables) {
            const string& varName = variable.first;
            int varValue = variable.second;

            if (gates.find(varName) != gates.end()) {
                const string& expression = gates.at(varName);
                bool result = evaluateExpression(expression, varValue, varValue);
                outputFile << timestamp << "," << varName << "," << result << endl;
            } else {
                cerr << "Variable '" << varName << "' not found in gates map." << endl;
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

    readCircuitFile(folderPath + "/T5.cir", components, gates,inputs);

    unordered_map<int, unordered_map<string, int>> simInputs;
    readSimulationFile(folderPath + "/T5.stim", simInputs);

    generateSimulationOutput(gates, simInputs, folderPath + "/T5.sim");

    return 0;
}
