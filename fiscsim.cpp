// Adisak Sangiamputtakoon
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <bitset>
#include <algorithm>
#include <sstream>

using namespace std;
string hexToBi(string hexa);
void intArray(string biString, unsigned int binaryArray[8]);
void printArray(unsigned int binaryArray[8]);
string decoder(unsigned int binaryArray[8], bool disassembly, int &z, bitset<8> registers[4], int &PC);
int getRegister(unsigned int reg[2]);
bool fullAdder(bool b1, bool b2, bool &carry);
bitset<8> addReg(bitset<8> rn, bitset<8> rm);
bitset<8> andReg(bitset<8> rn, bitset<8> rm);
bitset<8> notReg(bitset<8> rn);
void bnz(int z, int &PC, int tarAdd);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cerr << "USAGE: fiscsim <object file> [cycles] [-l]" << endl;
        cerr << "-d : print disassembly listing with each cycle" << endl;
        cerr << "if cycles are unspecified the CPU will run for 20 cycles" << endl;
        return -1;
    }

    string fileName = argv[1];
    ifstream file(fileName);
    bool disassembly = false;
    int cycles = 20;

    // checks to see if the file exists
    if (!file.good())
    {
        cerr << "Failed to open file." << endl;
        throw invalid_argument("Failed to open file.");
        return -1;
    }

    string arg1, arg2;
    if (argc == 4)
    {
        arg1 = argv[2];
        arg2 = argv[3];
    }
    else if (argc == 3)
    {
        arg1 = argv[2];
    }
    // checks to see if there is an cycles and disassembly argument in the command line
    if (arg1.find('-') != string::npos)
    {
        disassembly = true;
        if (argc == 4)
        {
            cycles = stoi(arg2);
        }
    }
    else if (argc == 4 && arg2.find('-') != string::npos)
    {
        disassembly = true;
        cycles = stoi(arg1);
    }
    else if (argc == 3)
    {
        cycles = stoi(arg1);
    }
    vector<string> lines;
    vector<string> binaries;

    // reads the file
    string line;
    while (getline(file, line))
    {
        std::transform(line.begin(), line.end(), line.begin(), ::toupper);
        lines.push_back(line);
    }
    int length = lines.size();
    if (lines.at(0) != "V2.0 RAW")
    {
        cerr << "Not a Logism-type memory image file" << endl;
        cerr << "The header must be \"v2.0 raw\"" << endl;
        return -1;
    }
    int PC = 1;
    bitset<8> registers[4] = {};
    int z = 0;
    binaries.push_back("empty_line");
    for (int i = 1; i < length; i++)
    {
        string current = lines.at(i);
        string biString = hexToBi(current);
        binaries.push_back(biString);
    }

    int biSize = binaries.size();
    unsigned int binaryArray[8] = {0};
    int cycleNum = 1;
    string disAssem;
    while (cycleNum <= cycles)
    {
        string current = binaries.at(PC);
        intArray(current, binaryArray);
        disAssem = decoder(binaryArray, disassembly, z, registers, PC);
        cout << "Cycle:" << dec << cycleNum << " State:PC:";
        cout << setfill('0') << setw(2) << hex << uppercase << PC;
        cout << " Z:" << to_string(z);
        cout << " R0: ";
        cout << setfill('0') << setw(2) << hex << registers[0].to_ulong();
        cout << " R1: ";
        cout << setfill('0') << setw(2) << hex << registers[1].to_ulong();
        cout << " R2: ";
        cout << setfill('0') << setw(2) << hex << registers[2].to_ulong();
        cout << " R3: ";
        cout << setfill('0') << setw(2) << hex << registers[3].to_ulong();
        cout << endl;
        if (disassembly)
        {
            cout << disAssem << endl;
            cout << endl;
        }

        cycleNum++;
        PC++;
    }
}

// takes a hex string and returns a binary string
string hexToBi(string hexa)
{
    stringstream ss;
    ss << hex << hexa;
    unsigned n;
    ss >> n;
    bitset<8> b(n);
    string result = b.to_string();
    return result;
}

// Turns the binary string into an array
void intArray(string biString, unsigned int binaryArray[8])
{
    for (int i = 0; i < 8; i++)
    {
        binaryArray[i] = (int)biString.at(i) - '0';
    }
}
// takes a binary array of the instruction memory and decodes the IM and returns the disambled code
string decoder(unsigned int binaryArray[8], bool disassembly, int &z, bitset<8> registers[4], int &PC)
{
    unsigned int instruction[2] = {binaryArray[0], binaryArray[1]};
    string disString = "Disassembly: ";
    vector<string> regString;
    // register string for printing
    regString.push_back("r0");
    regString.push_back("r1");
    regString.push_back("r2");
    regString.push_back("r3");

    unsigned int rd[2] = {binaryArray[6], binaryArray[7]};
    unsigned int rn[2] = {binaryArray[2], binaryArray[3]};
    unsigned int rm[2] = {binaryArray[4], binaryArray[5]};
    string tar = "";
    for (int i = 2; i < 8; i++)
    {
        tar += to_string(binaryArray[i]);
    }
    bitset<6> targetAddress(tar);
    int tarAdd = (int)(targetAddress.to_ulong());
    int rdNum = getRegister(rd);
    int rnNum = getRegister(rn);
    int rmNum = getRegister(rm);
    if (instruction[0] == 0)
    {
        if (instruction[1] == 0)
        {
            // add
            registers[rdNum] = addReg(registers[rnNum], registers[rmNum]);

            disString += "add ";
        }
        else
        {
            // and
            registers[rdNum] = andReg(registers[rnNum], registers[rmNum]);

            disString += "and ";
        }

        disString += regString.at(rdNum) + " " + regString.at(rnNum) + " " + regString.at(rmNum);
    }
    if (instruction[0] == 1)
    {
        if (instruction[1] == 0)
        {
            // not
            registers[rdNum] = notReg(registers[rnNum]);
            disString += "not ";
            disString += regString.at(rdNum) + " " + regString.at(rnNum);
        }
        else
        {
            // bnz
            bnz(z, PC, tarAdd);
            disString += "bnz " + to_string(tarAdd);
        }
    }
    if (rdNum == 3 && registers[3] == 0)
    {
        z = 1;
    }
    else
    {
        z = 0;
    }

    return disString;
}
// returns the correct address of the register
int getRegister(unsigned int reg[2])
{
    if (reg[0] == 0)
    {
        if (reg[1] == 0)
        {
            // r0
            return 0;
        }
        else
        {
            // r1
            return 1;
        }
    }
    if (reg[0] == 1)
    {
        if (reg[1] == 0)
        {
            // r2
            return 2;
        }
        else
        {
            // r3
            return 3;
        }
    }
    return 0;
}

// adds two registers
bitset<8> addReg(bitset<8> rn, bitset<8> rm)
{
    // add
    bool carry = false;
    bitset<8> rd;

    for (int i = 0; i < 8; i++)
    {
        rd[i] = fullAdder(rn[i], rm[i], carry);
    }
    return rd;
}

// ands two registers
bitset<8> andReg(bitset<8> rn, bitset<8> rm)
{
    bitset<8> rd;
    rd = rn & rm;
    return rd;
}

// returns the complement of a register
bitset<8> notReg(bitset<8> rn)
{
    bitset<8> rd;
    rd = ~rn; // not
    return rd;
}

// changes the PC counter if the z flag is false
void bnz(int z, int &PC, int tarAdd)
{
    if (z == 0)
    {
        PC = tarAdd;
    }
}

// helper function for the addReg function
bool fullAdder(bool b1, bool b2, bool &carry)
{
    bool sum = (b1 ^ b2) ^ carry;
    carry = (b1 && b2) || (b1 && carry) || (b2 && carry);
    return sum;
}