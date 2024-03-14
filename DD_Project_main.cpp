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
using namespace std;

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

struct Component  { // describe the specifics of each gate
    int numInputs;
    string outputExpression;
    int delayPs;
};

bool evaluateExpression(const string& expression,bool x, bool y) {
    stack<char> operators;
    stack<bool> operands;

    for (char c : expression) {
        if (isspace(c)) {
            // Ignore whitespace
            continue;
        } else if (isalpha(c)) {
            // If the character is a variable, push its value onto the stack
            operands.push(x);
            operands.push(y);
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
    unordered_map<string, int> stim;
    vector<string> inputs; // The inputs of a test circuit
    int inputsc = -1; //number of inputs in a test circuit
    unordered_map<string, Component> components;
    int size = 5;// Store the library file, Key : Gate name , Value : gate's specifics
    string evaluatedExpr[size];
    int times[size];
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
            }
        }

        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string name, outputExpr;
            string snumInputs, sdelay;
            if (getline(ss, name, ',') &&
                getline(ss, snumInputs, ',') &&
                getline(ss, outputExpr, ',')
                && getline(ss, sdelay, ',')) {
                int numInputs = std::stoi(snumInputs);
                int delay = std::stoi(sdelay);
                components[name] = {numInputs, outputExpr, delay};

            }
        }
        //Printing the map for testing
        // for (auto it = components.begin(); it != components.end(); ++it) {

        // cout << "Gate: " << it->first << ", Specifics: " << it->second.numInputs<<'\t'<<it->second.outputExpression <<'\t'<< it->second.delayPs << endl;
        // }
    }
    file.close();




    for (const auto &entry : filesystem::directory_iterator(folderPath)) {
        string fileName = entry.path().filename().string();  // Extract filename from path
        if (fileName.find("stim") != std::string::npos) {    // Check if filename contains "stim"
            ifstream files(entry.path());  // Open the file
            if (!files.is_open()) {
                cerr << "Error opening file: " << fileName << endl;
                continue;
            }

            string line;
            int index = 0;
            while (getline(files, line)) {
                stringstream ss(line);
                string time1, name, value1;
                if (getline(ss, time1, ',') && getline(ss, name, ',') && getline(ss, value1, ',')) {
                    try {
                        times[index] = safe_stoi(time1);
                        int value = safe_stoi(value1);
                        stim[name] = value;
                        index++;
                    } catch (const std::invalid_argument& e) {
                        cerr << "Invalid argument in line: " << line << endl;
                    }
                }
            }
            files.close();
        }
    }



    ifstream fileC;
    string fileNameC;
    for (const auto &entry: filesystem::directory_iterator(folderPath)) {
        fileNameC = entry.path().string();
        if (fileNameC.find(".cir") != std::string::npos) {
            fileC.open(fileNameC);
            if (!fileC.is_open()) {
                cerr << "Error opening file: " << fileNameC << endl;
                continue;
            }
        }
        string lineC;
        getline(fileC, lineC);
        while (getline(fileC, lineC) && lineC != "COMPONENTS:") {

            inputsc++;               // count the inputs
        }

        while (getline(fileC, lineC)) {
            stringstream ss(lineC);

            string gate, inputs[inputsc], output;

            getline(ss, gate, ',');
            getline(ss, output, ',');
            for (int i = 0; i < inputsc; i++) {
                getline(ss, inputs[i], ',');
            }

         int indexc=0;
            // replacing the inputs from the library file with the inputs from the circuit file in the output expression of the gate read from the circuit file
    evaluatedExpr[indexc] = components[gate].outputExpression;
            for (int i = 0; i < components[gate].numInputs; i++) {
                size_t pos = evaluatedExpr[indexc].find("i" + to_string(i + 1));
                while (pos != string::npos) {
                    evaluatedExpr[indexc].replace(pos, 2, inputs[i]);
                    pos = evaluatedExpr[indexc].find("i" + to_string(i + 1), pos + 2);
                }
            }
            cout << "The expression is "<< evaluatedExpr[indexc] << endl;
            int j=0;

            cout <<"the result is "<<evaluateExpression(evaluatedExpr[indexc], stim[inputs[j]], stim[inputs[j+1]])<<endl;
j++;
            indexc++;

        }


    }
    fileC.close();


// Function to handle time stamps
    /*int variables[3];

    auto start = chrono::steady_clock::now();
    int t = 5000; // 5000 milliseconds total duration
    variables[0] = 0;
    variables[1] = 0;
    variables[2] = 0;


    while (t > 0) {
        auto end = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        if (elapsed == times[0]) { // 500 milliseconds
            variables[0] = 1;
            cout << "A= " << variables[0] << " at time: " << elapsed << " milliseconds" << endl;
        } else if (elapsed == times[1]) { // 800 milliseconds
            variables[1] = 1;
            cout << "B= " << variables[1] << " at time: " << elapsed << " milliseconds" << endl;
        } else if (elapsed == times[2]) { // 1000 milliseconds
            variables[2] = 1;
            cout << "C= " << variables[2] << " at time: " << elapsed << " milliseconds" << endl;
        } else if (elapsed == times[3]) { // 1300 milliseconds
            variables[0] = 1;
            variables[1] = 0;
            variables[2] = 1;
            cout << "At time: " << elapsed << " milliseconds, The values are: " << "A=" << variables[0] << " ,B="
                 << variables[1] << " ,C=" << variables[2] << endl;
        }
        t = t - 1;

        // Wait for 1 millisecond
        this_thread::sleep_for(chrono::milliseconds(1));

    }*/
}