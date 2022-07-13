// Adisak Sangiamputtakoon
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <map>
#include <sstream>
#include <iomanip>
#include <bitset>
#include <algorithm>

using namespace std;
void stringSplitter(string currentLine, vector<string> &operands);
void printLines(vector<string> &lines, int length);
void purgeComments(vector<string> &lines, int length);
void firstPass(vector<string> &lines, int length, map<string, string> &symbolTable);
string threeArgs(vector<string> &arguments, map<string, string> &instructionsAndRegisters);
string twoArgs(vector<string> &arguments, map<string, string> &instructionsAndRegisters);
string oneArg(vector<string> &arguments, map<string, string> &symbolTable);
void secondPass(vector<string> &lines, int length, map<string, string> &symbolTable, vector<string> &binaryCode);
string biToHex(string binary);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cerr << "Wrong number of arguments." << endl;
        return -1;
    }
    map<string, string> symbolTable;

    vector<string> binaryCode;
    bool printList = false;

    string wfile = argv[2];
    ofstream writeFile;
    writeFile.open(wfile);

    int bytes[8] = {0};

    string fileName = argv[1];
    ifstream file;
    file.open(fileName);
    if (!file.good() || !writeFile.good())
    {
        cerr << "Failed to open file." << endl;
        throw invalid_argument("Failed to open file.");
        return -1;
    }
    vector<string> lines;

    string line;
    while (getline(file, line))
    {
        std::transform(line.begin(), line.end(), line.begin(), ::toupper);
        lines.push_back(line);
    }
    int length = lines.size();

    if (argc ==4)
    {
        string list = argv[2];
        printList = true;
    }

    //  printLines(lines);
    vector<string> arguments;
    purgeComments(lines, length);
    firstPass(lines, length, symbolTable);

    secondPass(lines, length, symbolTable, binaryCode);
    vector<string> hexCode;
    int bi = binaryCode.size();
    for (int i = 0; i < bi; i++)
    {
        string hexLine = biToHex(binaryCode.at(i));
        hexCode.push_back(hexLine);
    }

    writeFile << "v2.0 raw" << endl;
    for (int i = 0; i < length; i++)
    {
        if (i != length - 1)
        {
            writeFile << hexCode.at(i) << endl;
        }
        else
        {
            writeFile << hexCode.at(i);
        }
    }

    if (printList)
    {
         cout << "*** MACHINE PROGRAM ***" << endl;
        for (int i = 0; i < length; i++)
        {
            // cout << to_string(i) << hexCode.at(i);
            cout << setfill('0') << setw(2) << hex << uppercase << i;
            cout << ":" << hexCode.at(i) << lines.at(i);
            cout << endl;
        }
    }
  //  cout << "Here" << endl;
    // printLines(binaryCode, bi);

    writeFile.close();
    file.close();
}

void firstPass(vector<string> &lines, int length, map<string, string> &symbolTable)
{
    //     cout << "Here topia" << endl;;

    for (int i = 0; i < length; i++)
    {
        vector<string> arguments1;

        stringSplitter(lines.at(i), arguments1);
        string firstArg = arguments1.at(0);
        if (firstArg.find(':') != string::npos)
        {
            if (symbolTable.find(firstArg) == symbolTable.end())
            {
                string binary = std::bitset<6>(i).to_string();
                symbolTable.insert(pair<string, string>(firstArg, binary));
            }
            else
            {
                cerr << "Error: Too many labels." << endl;
            }
        }
    }
    //   cout << "Here topia";
}

void secondPass(vector<string> &lines, int length, map<string, string> &symbolTable, vector<string> &binaryCode)
{
    map<string, string> instructionsAndRegisters;
    string byte;

    instructionsAndRegisters.insert(pair<string, string>("ADD", "00"));
    instructionsAndRegisters.insert(pair<string, string>("AND", "01"));

    instructionsAndRegisters.insert(pair<string, string>("R0", "00"));
    instructionsAndRegisters.insert(pair<string, string>("R1", "01"));
    instructionsAndRegisters.insert(pair<string, string>("R2", "10"));
    instructionsAndRegisters.insert(pair<string, string>("R3", "11"));

    for (int i = 0; i < length; i++)
    {
        vector<string> arguments2;

        stringSplitter(lines.at(i), arguments2);
        int argLength = arguments2.size();
        string firstArg = arguments2.at(0);

        if (firstArg.find(':') != string::npos)
        {
            //  cout << "Here topia 2" << endl;

            arguments2.erase(arguments2.begin());
            argLength = arguments2.size();
            // cout << "arg length " << argLength << endl;
        }
        //   printLines(arguments2, argLength);
        //   cout << endl;

        if (argLength - 1 == 3)
        {
            byte = threeArgs(arguments2, instructionsAndRegisters);
        }
        else if (argLength - 1 == 2)
        {

            byte = twoArgs(arguments2, instructionsAndRegisters);
        }
        else
        {

            byte = oneArg(arguments2, symbolTable);
        }

        // cout << "Outside" << endl;
        //  cout << "Byte:" << byte;

        binaryCode.push_back(byte);
    }
}

void purgeComments(vector<string> &lines, int length)
{
    string currentLine;
    for (int i = 0; i < length; i++)
    {
        currentLine = lines.at(i);
        int index = currentLine.find(';');
        if (index != string::npos)
        {
            lines.at(i) = currentLine.substr(0, index);
        }
    }
}
void printLines(vector<string> &lines, int length)
{
    for (int i = 0; i < length; i++)
    {
        cout << lines.at(i) << endl;
    }
}
void stringSplitter(string currentLine, vector<string> &operands)
{
    istringstream ss(currentLine);
    string word;
    while (ss >> word)
    {
        operands.push_back(word);
    }
}

string threeArgs(vector<string> &arguments, map<string, string> &instructionsAndRegisters)
{
    string byte = "";
    byte += instructionsAndRegisters.find(arguments[0])->second;
    byte += instructionsAndRegisters.find(arguments[2])->second;
    byte += instructionsAndRegisters.find(arguments[3])->second;
    byte += instructionsAndRegisters.find(arguments[1])->second;

    return byte;
}

string twoArgs(vector<string> &arguments, map<string, string> &instructionsAndRegisters)
{
    string byte = "10";
    byte += instructionsAndRegisters.find(arguments[2])->second;
    byte += "00";
    byte += instructionsAndRegisters.find(arguments[1])->second;

    return byte;
}

string oneArg(vector<string> &arguments, map<string, string> &symbolTable)
{
    string byte = "11";
    byte += symbolTable.find(arguments[1] + ":")->second;
    return byte;
}

string biToHex(string binary)
{
    int binaryNum, hex = 0, mul = 1, chk = 1, rem, i = 0;
    char hexDecNum[2];
    binaryNum = stoi(binary);

    while (binaryNum != 0)
    {
        rem = binaryNum % 10;
        hex = hex + (rem * mul);
        if (chk % 4 == 0)
        {
            if (hex < 10)
                hexDecNum[i] = hex + 48;
            else
                hexDecNum[i] = hex + 55;
            mul = 1;
            hex = 0;
            chk = 1;
            i++;
        }
        else
        {
            mul = mul * 2;
            chk++;
        }
        binaryNum = binaryNum / 10;
    }
    if (chk != 1)
        hexDecNum[i] = hex + 48;
    if (chk == 1)
        i--;

    string hexa = string(hexDecNum);
    int length = hexa.length();
    if (length == 1)
    {
        hexa += "00";
    }
    reverse(hexa.begin(), hexa.end());

    string result = hexa.substr(1, 2);
    return result;
}