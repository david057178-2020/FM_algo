#ifndef PARTITIONER_H
#define PARTITIONER_H

#include <fstream>
#include <vector>
#include <map>
#include "cell.h"
#include "net.h"
using namespace std;

class Partitioner
{
public:
    // constructor and destructor
    Partitioner(fstream& inFile) :
        _cutSize(0), _netNum(0), _cellNum(0), _maxPinNum(0), _bFactor(0),
        _accGain(0), _maxAccGain(0), _iterNum(0) {
        parseInput(inFile);
        _partSize[0] = 0;
        _partSize[1] = 0;
    }
    ~Partitioner() {
        clear();
    }

    // basic access methods
    int getCutSize() const          { return _cutSize; }
    int getNetNum() const           { return _netNum; }
    int getCellNum() const          { return _cellNum; }
    double getBFactor() const       { return _bFactor; }
    int getPartSize(int part) const { return _partSize[part]; }

    // modify method
    void parseInput(fstream& inFile);
    void partition();

    //my function
	void initParti();
	void setBasic();
	void setPartCountAndCutSize();
	void setInitG();
	void setMaxGainNode(Node* node) { _maxGainCell = node; }
	bool checkBalance(const int& size){
		const double& UB = ((1 + _bFactor) / 2) * _cellNum;
		const double& LB = ((1 - _bFactor) / 2) * _cellNum;		
		return (size >= LB && size <= UB);
	}
	void updateGain(Cell* cell);
	void changeAllGainOnNet(Net* net, const bool& b); //dec(0), inc(1)
	void changeOneGainOnNet(Net* net, const bool& b, const bool& part); //A(0), B(1)
	void printBList();
	void updateList(Cell* cell);
	void deleteNode(Node* node);
	void insertNode(Node* const node);	
	void initBList();
	Cell* pickBaseCell();

	// member functions about reporting
    void printSummary() const;
    void reportNet() const;
    void reportCell() const;
    void writeResult(fstream& outFile);

private:
    int                 _cutSize;       // cut size
    int                 _partSize[2];   // size (cell number) of partition A(0) and B(1)
    int                 _netNum;        // number of nets
    int                 _cellNum;       // number of cells
    int                 _maxPinNum;     // Pmax for building bucket list
    double              _bFactor;       // the balance factor to be met
    Node*               _maxGainCell;   // pointer to max gain cell
    vector<Net*>        _netArray;      // net array of the circuit
    vector<Cell*>       _cellArray;     // cell array of the circuit
    map<int, Node*>     _bList[2];      // bucket list of partition A(0) and B(1)
    map<string, int>    _netName2Id;    // mapping from net name to id
    map<string, int>    _cellName2Id;   // mapping from cell name to id

    int                 _accGain;       // accumulative gain
    int                 _maxAccGain;    // maximum accumulative gain
    int                 _moveNum;       // number of cell movements
    int                 _iterNum;       // number of iterations
    int                 _bestMoveNum;   // store best number of movements
    int                 _unlockNum[2];  // number of unlocked cells
    vector<int>         _moveStack;     // history of cell movement

	//my member
	int                 _sizeDiff;      //for decideing tie _maxAccGain
    
	// Clean up partitioner
    void clear();
};

#endif  // PARTITIONER_H
