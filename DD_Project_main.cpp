#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <stack>
#include <chrono>
#include <thread>
#include <sstream>
#include <cctype>

using namespace std;
namespace fs = std::filesystem;

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

// Structure to hold gate components
struct Component {
    int numInputs;
    string outputExpression;
    int delayPs;
};

// Structure to hold input values
struct Input {
    string in;
    int value;
};

// Function to evaluate logical expression
bool evaluateExpression(const string& expression, bool x, bool y) {
    stack<char> operators;
    stack<bool> operands;

    for (char c : expression) {
        if (isspace(c)) {
            // Ignore whitespace
            continue;
        } else if (isalpha(c)) {
            // If the character is a variable, push its value onto the stack
            operands.push((c == 'x') ? x : y);
        } else if (c == '(') {
            // Push opening parenthesis onto the operator stack
            operators.push(c);
        } else if (c == ')') {
            // Evaluate until the corresponding opening parenthesis
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
            // Remove the opening parenthesis
            operators.pop();
        } else if (c == '&' || c == '|' || c == '~') {
            // Operator encountered
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
            // Push the current operator onto the stack
            operators.push(c);
        }
    }

    // Evaluate remaining operators
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

// Function to simulate the circuit
// Function to simulate the circuit
void simulate(const unordered_map<string, Component>& components, const string& folderPath) {
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        string fileName = entry.path().filename().string();
        if (fileName.find(".cir") != string::npos) {
            ifstream fileC(entry.path());
            if (!fileC.is_open()) {
                cerr << "Error opening file: " << fileName << endl;
                continue;
            }

            int inputsc = 0; // Number of inputs in a test circuit
            string lineC;
            while (getline(fileC, lineC) && lineC != "COMPONENTS:") {
                inputsc++; // Count the inputs
            }

            while (getline(fileC, lineC)) {
                stringstream ss(lineC);
                string gate, output;
                vector<string> inputs(inputsc);

                getline(ss, gate, ',');
                getline(ss, output, ',');
                for (int i = 0; i < inputsc; i++) {
                    getline(ss, inputs[i], ',');
                }

                if (components.find(gate) != components.end()) { // Check if the key exists
                    string evaluatedExpr = components.at(gate).outputExpression;
                    for (int i = 0; i < components.at(gate).numInputs; i++) {
                        size_t pos = evaluatedExpr.find("i" + to_string(i + 1));
                        while (pos != string::npos) {
                            evaluatedExpr.replace(pos, 2, inputs[i]);
                            pos = evaluatedExpr.find("i" + to_string(i + 1), pos + 2);
                        }
                    }

                    cout << "Output of gate " << gate << " is " << evaluateExpression(evaluatedExpr, true, false) << endl;
                } else {
                    cerr << "Gate '" << gate << "' not found in components map." << endl;
                }
            }

            fileC.close();
        }
    }
}
int main() {
    unordered_map<string, Component> components;
    string folderPath;

    cout << "Enter folder path containing test cases: ";
    getline(cin, folderPath);

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        string fileName = entry.path().filename().string();
        if (fileName.find(".lib") != string::npos) {
            ifstream file(entry.path());
            if (!file.is_open()) {
                cerr << "Error opening file: " << fileName << endl;
                continue;
            }

            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string name, outputExpr, snumInputs, sdelay;
                if (getline(ss, name, ',') && getline(ss, snumInputs, ',') && getline(ss, outputExpr, ',')
                    && getline(ss, sdelay, ',')) {
                    int numInputs = safe_stoi(snumInputs);
                    int delay = safe_stoi(sdelay);
                    components[name] = {numInputs, outputExpr, delay};
                }
            }
            file.close();
        }
    }

    simulate(components, folderPath);

    return 0;
}
