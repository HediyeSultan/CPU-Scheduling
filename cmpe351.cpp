/* This is code developed by <Hediye Sultan Bozkurt> */
#include "cmpe351.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream> 
#include <climits> 
#include <iomanip>

using namespace std;

struct Process {
    int burstTime;
    int priority;
    int arrivalTime;
    int queueID;
    int waitingTime;
    int processID; 
    bool isCompleted;
};

struct Node {
    Process data;
    Node* next;
};

Node* createNode(int b, int p, int a, int q, int pid) {
    Node* newNode = new Node();
    newNode->data.burstTime = b;
    newNode->data.priority = p;
    newNode->data.arrivalTime = a;
    newNode->data.queueID = q;
    newNode->data.processID = pid;
    newNode->data.waitingTime = 0;
    newNode->data.isCompleted = false;
    newNode->next = nullptr;
    return newNode;
}

void append(Node*& head, int b, int p, int a, int q, int pid) {
    Node* newNode = createNode(b, p, a, q, pid);
    if (head == nullptr) {
        head = newNode;
    } else {
        Node* temp = head;
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
}

void readFile(const string& filename, Node*& head) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error:File (" << filename << ") didnt open!" << endl;
        exit(1);
    }

    string line;
    int pidCounter = 0; 

    while (getline(file, line)) {
        if (line.empty()) continue; 

        if (!line.empty() && line[line.length()-1] == '\r') {
            line.erase(line.length()-1);
        }

        stringstream ss(line);
        string segment;
        int values[4];
        int i = 0;
        while (getline(ss, segment, ':') && i < 4) {
            try {
                values[i] = stoi(segment);
            } catch (...) {
                values[i] = 0;
            }
            i++;
        }

        if (i == 4) {
            append(head, values[0], values[1], values[2], values[3], pidCounter++);
        }
    }
    file.close();
} 

void resetCompletionStatus(Node* head) {
    Node* temp = head;
    while (temp != nullptr) {
        temp->data.isCompleted = false;
        temp = temp->next;
    }
}

int countProcesses(Node* head, int queueID) {
    int count = 0;
    Node* temp = head;
    while (temp != nullptr) {
        if (temp->data.queueID == queueID) count++;
        temp = temp->next;
    }
    return count;
}
void calculateFCFS(Node* head, int targetQueueID, ofstream& outFile) {
    int currentTime = 0;
    double totalWait = 0;
    int count = 0;
    string outputLine = to_string(targetQueueID) + ":1";
    Node* temp = head;
    bool foundAny = false;

    while (temp != nullptr) {
        if (temp->data.queueID == targetQueueID) {
            foundAny = true;
            if (currentTime < temp->data.arrivalTime) {
                currentTime = temp->data.arrivalTime;
            }
            int wait = currentTime - temp->data.arrivalTime;
            if (wait < 0) wait = 0;

            totalWait += wait;
            currentTime += temp->data.burstTime;
            count++;
            outputLine += ":" + to_string(wait);
        }
        temp = temp->next;
    }

    if (foundAny) {
        double awt = (count > 0) ? (totalWait / count) : 0.0;
        outFile << outputLine << ":" << fixed << setprecision(2) << awt << endl;
        cout << outputLine << ":" << fixed << setprecision(2) << awt << endl;
    }
}

void calculateSJF(Node* head, int targetQueueID, ofstream& outFile) {
    resetCompletionStatus(head); 

    int currentTime = 0;
    int completedCount = 0;
    int totalProcesses = countProcesses(head, targetQueueID);

    if (totalProcesses == 0) return;

    while (completedCount < totalProcesses) {
        Node* shortestNode = nullptr;
        int minBurst = INT_MAX;
        
        Node* temp = head;
        bool foundCandidate = false;

        while (temp != nullptr) {
            if (temp->data.queueID == targetQueueID && !temp->data.isCompleted && temp->data.arrivalTime <= currentTime) {
                if (temp->data.burstTime < minBurst) {
                    minBurst = temp->data.burstTime;
                    shortestNode = temp;
                    foundCandidate = true;
                }
            }
            temp = temp->next;
        }

        if (foundCandidate && shortestNode != nullptr) {
            shortestNode->data.waitingTime = currentTime - shortestNode->data.arrivalTime;
            currentTime += shortestNode->data.burstTime;
            shortestNode->data.isCompleted = true;
            completedCount++;
        } else {
            currentTime++;
        }
    }
    string outputLine = to_string(targetQueueID) + ":2"; 
    double totalWait = 0;
    
    Node* temp = head;
    while (temp != nullptr) {
        if (temp->data.queueID == targetQueueID) {
            outputLine += ":" + to_string(temp->data.waitingTime);
            totalWait += temp->data.waitingTime;
        }
        temp = temp->next;
    }

    double awt = (totalProcesses > 0) ? (totalWait / totalProcesses) : 0.0;
    outFile << outputLine << ":" << fixed << setprecision(2) << awt << endl;
    cout << outputLine << ":" << fixed << setprecision(2) << awt << endl;
}

void calculatePriority(Node* head, int targetQueueID, ofstream& outFile) {
    resetCompletionStatus(head); 
    
    int currentTime = 0;
    int completedCount = 0;
    int totalProcesses = countProcesses(head, targetQueueID);

    if (totalProcesses == 0) return;

    while (completedCount < totalProcesses) {
        Node* selectedNode = nullptr;
        int minPriority = INT_MAX;
        Node* temp = head;
        bool foundCandidate = false;

        while (temp != nullptr) {
            if (temp->data.queueID == targetQueueID && !temp->data.isCompleted && temp->data.arrivalTime <= currentTime) {
                if (temp->data.priority < minPriority) {
                    minPriority = temp->data.priority;
                    selectedNode = temp;
                    foundCandidate = true;
                }
            }
            temp = temp->next;
        }

        if (foundCandidate && selectedNode != nullptr) {
            selectedNode->data.waitingTime = currentTime - selectedNode->data.arrivalTime;
            currentTime += selectedNode->data.burstTime;
            selectedNode->data.isCompleted = true;
            completedCount++;
        } else {
            currentTime++;
        }
    }

    string outputLine = to_string(targetQueueID) + ":3"; 
    double totalWait = 0;
    
    Node* temp = head;
    while (temp != nullptr) {
        if (temp->data.queueID == targetQueueID) {
            outputLine += ":" + to_string(temp->data.waitingTime);
            totalWait += temp->data.waitingTime;
        }
        temp = temp->next;
    }

    double awt = (totalProcesses > 0) ? (totalWait / totalProcesses) : 0.0;
    outFile << outputLine << ":" << fixed << setprecision(2) << awt << endl;
    cout << outputLine << ":" << fixed << setprecision(2) << awt << endl;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "Usage: ./cmpe351 input.txt output.txt" << endl;
        return 1;
    }

    string inputFileName = argv[1];
    string outputFileName = argv[2];

    Node* processList = nullptr;
    readFile(inputFileName, processList);

    ofstream outFile(outputFileName);
    if (!outFile.is_open()) {
        cerr << "File didnt execute!" << endl;
        return 1;
    }
    calculateFCFS(processList, 0, outFile);
    calculateSJF(processList, 0, outFile);
    calculatePriority(processList, 0, outFile);


    calculateFCFS(processList, 1, outFile);
    calculateSJF(processList, 1, outFile);
    calculatePriority(processList, 1, outFile);

    outFile.close();
    cout << "Executed. Results are written in" << outputFileName << "." << endl;

    return 0;
}
