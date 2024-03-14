#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <queue>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <stack>
using namespace std;

struct Component  { // describe the specifics of each gate
    int numInputs;
    string outputExpression;
    int delayPs;
};
bool evaluateExpression(const string& expression) {
    stack<char> operators;
    stack<bool> operands;

    for (char c : expression) {
        if (isspace(c)) {
            // Ignore whitespace
            continue;
        } else if (isalpha(c)) {
            // If the character is a variable, push its value onto the stack
            operands.push(true);
            // For simplicity, assuming all variables are true
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
int main() {
    unordered_map<char, bool> variable;
    vector<string>inputs; // The inputs of a test circuit
    int inputsc=-1; //number of inputs in a test circuit
    unordered_map<string, Component> components; // Store the library file, Key : Gate name , Value : gate's specifics
    string folderPath, outputFilePath;
    cout << "Enter folder path containing test cases: ";
    getline(cin, folderPath);
    ifstream file;
    string fileName;
    for (const auto &entry: filesystem::directory_iterator(folderPath)) {
        fileName = entry.path().string();
        if (fileName.find(".lib") != std::string::npos) {
            file.open(fileName);
            if (!file.is_open()) {
                cerr << "Error opening file: " << fileName << endl;
                continue;
            }}

        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string name, outputExpr;
            string snumInputs, sdelay;
            if (getline(ss, name, ',') &&
            getline(ss, snumInputs, ',')&&
                getline(ss, outputExpr, ',')
                &&getline(ss, sdelay, ',')) {
                int numInputs = std::stoi(snumInputs);
                int delay = std::stoi(sdelay);
                components[name] = {numInputs, outputExpr, delay};

        }}
        //Printing the map for testing
       // for (auto it = components.begin(); it != components.end(); ++it) {

     // cout << "Gate: " << it->first << ", Specifics: " << it->second.numInputs<<'\t'<<it->second.outputExpression <<'\t'<< it->second.delayPs << endl;
       // }
    } file.close();


    ifstream fileC;
    string fileNameC;
    for (const auto &entry: filesystem::directory_iterator(folderPath)) {
        fileNameC = entry.path().string();
        if (fileNameC.find(".cir") != std::string::npos) {
            fileC.open(fileNameC);
            if (!fileC.is_open()) {
                cerr << "Error opening file: " << fileNameC << endl;
                continue;
            }}
        string lineC;
        getline(fileC, lineC);
        while (getline(fileC, lineC)&&lineC!="COMPONENTS:") {

            inputsc++;               // count the inputs
        }

        while (getline(fileC, lineC)){
            stringstream ss(lineC);

            string gate,inputs[inputsc],output;

           getline(ss, gate, ',');
            getline(ss, output, ',');
           for(int i=0;i<inputsc;i++){
               getline(ss, inputs[i], ',');}

            // replacing the inputs from the library file with the inputs from the circuit file in the output expression of the gate read from the circuit file
           string evaluatedExpr=components[gate].outputExpression;
                   for (int i=0;i<components[gate].numInputs;i++){
                    size_t pos = evaluatedExpr.find("i"+ to_string(i+1));
                    while (pos != string::npos) {
                        evaluatedExpr.replace(pos, 2, inputs[i]);
                        pos = evaluatedExpr.find("i" + to_string(i + 1), pos + 2);
                    }}
                        cout << evaluatedExpr << endl;
            cout<<evaluateExpression(evaluatedExpr);





        }



        }fileC.close();}

// Function to evaluate the Boolean expression


