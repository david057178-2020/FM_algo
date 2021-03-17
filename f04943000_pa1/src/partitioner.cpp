#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <map>
#include "cell.h"
#include "net.h"
#include "partitioner.h"
using namespace std;


void Partitioner::parseInput(fstream& inFile)
{
    string str;
    // Set balance factor
    inFile >> str;
    _bFactor = stod(str);

    // Set up whole circuit
    while (inFile >> str) {
        if (str == "NET") {
            string netName, cellName, tmpCellName = "";
            inFile >> netName;
            int netId = _netNum;
            _netArray.push_back(new Net(netName));
            _netName2Id[netName] = netId;
            while (inFile >> cellName) {
                if (cellName == ";") {
                    tmpCellName = "";
                    break;
                }
                else {
                    // a newly seen cell
                    if (_cellName2Id.count(cellName) == 0) {
                        int cellId = _cellNum;
                        _cellArray.push_back(new Cell(cellName, 0, cellId));
                        _cellName2Id[cellName] = cellId;
                        _cellArray[cellId]->addNet(netId);
                        _cellArray[cellId]->incPinNum();
                        _netArray[netId]->addCell(cellId);
                        ++_cellNum;
                        tmpCellName = cellName;
                    }
                    // an existed cell
                    else {
                        if (cellName != tmpCellName) {
                            assert(_cellName2Id.count(cellName) == 1);
                            int cellId = _cellName2Id[cellName];
                            _cellArray[cellId]->addNet(netId);
                            _cellArray[cellId]->incPinNum();
                            _netArray[netId]->addCell(cellId);
                            tmpCellName = cellName;
                        }
                    }
                }
            }
            ++_netNum;
        }
    }
    return;
}

void Partitioner::initParti()
{
	//move node and set part size and set max pin
	int halfNode = getCellNum() / 2;
	Cell* cell;
	for(size_t i = 0; i < halfNode; ++i){
		cell = _cellArray[i];
		cell->move();
	}
	setBSize(halfNode);

	//set max pin num
	int maxPin = 0;
	for(size_t i = 0, n = getCellNum(); i < n; ++i){
		cell = _cellArray[i];
		if(cell->getPinNum() > maxPin) maxPin = cell->getPinNum();
	}
	setMaxPinNum(maxPin); 
	//cout << "max pin num = " << maxPin << endl;

	//set part count and cut size
	int cutSize = 0;
	for(size_t i = 0, n = getNetNum(); i < n; ++i){
		Net* net = _netArray[i];
		vector<int>* cellList = net->getCellListPtr();
		for(size_t j = 0, m = cellList->size(); j < m; ++j){
			Cell* cell = _cellArray[(*cellList)[j]];
			net->incPartCount(cell->getPart());
		}
		if(net->getPartCount(0) > 0 && net->getPartCount(1) > 0) ++cutSize;
	}
	setCutSize(cutSize);
}

void Partitioner::setInitG()
{
	int n = getCellNum();
	int maxG = 0;
	Cell* maxGainCell;
	Cell* cell;
	bool part;
	int gain;
	Node* node;
	for(size_t i = 0; i < n; ++i){
		cell = _cellArray[i];
		part = cell->getPart();
        vector<int>* netList = cell->getNetListPtr();
		//compute gain of each cell
		for (size_t j = 0, m = netList->size(); j < m; ++j) {
			Net* net = _netArray[(*netList)[j]];
			if(net->getPartCount(part) == 1){
				cell->incGain();
			}
			else if(net->getPartCount(!part) == 0){
				cell->decGain();
			}
		}
		
		gain = cell->getGain();
		//record max gain cell
		if(gain > maxG) {
			maxG = gain;
			maxGainCell = cell;
		}

		//record bList
		map<int, Node*>::iterator it = _bList[part].find(gain);
		if(it != _bList[part].end()){
			node = it->second;
			//cout << "in (part, gain) = (" << part << ", " << gain << "), ";
			//cout << "find node " << _cellArray[node->getId()]->getName() << endl; 
			while(node->getNext() != NULL){
				node = node->getNext();
				//cout << "---> node " << _cellArray[node->getId()]->getName() << endl;
			}
			node->setNext(cell->getNode());
			cell->getNode()->setPrev(node);
		}
		else{
			_bList[part][gain] = cell->getNode();
		}
	}
	setMaxGainCell(maxGainCell);
	
	cout << endl << "A list is:" << endl;
	for(map<int, Node*>::iterator it = _bList[0].begin(); it != _bList[0].end(); ++it){
		cout << "gain = " << it->first << endl;
		node = it->second;
		cout << "cell =  " << _cellArray[node->getId()]->getName() << endl;
		while(node->getNext() != NULL){
			node = node->getNext();
            cout << "---> node " << _cellArray[node->getId()]->getName() << endl;
        }
	}

	cout << endl << "B list is:" << endl;
    for(map<int, Node*>::iterator it = _bList[1].begin(); it != _bList[1].end(); ++it){
        cout << "gain = " << it->first << endl;
        node = it->second;
        cout << "cell =  " << _cellArray[node->getId()]->getName() << endl;
        while(node->getNext() != NULL){
            node = node->getNext();
            cout << "---> node " << _cellArray[node->getId()]->getName() << endl;
        }
    }
	
}
/*
void Partitioner::moveCell()
{
	Node* node = _maxGainCell;
	Cell* cell = _cellArray[node->getId()];
	bool F = cell->getPart();
	bool T = !F;
	cell->move();
	cell->lock();

	vector<int>* netList = cell->getNetListPtr();
    //for each net on base cell
    for (size_t j = 0, m = netList->size(); j < m; ++j) {
        Net* net = _netArray[(*netList)[j]];
		//before move
		if(net->_partCount[T] == 0){
			//++gain for all free cell on n
			vector<int>* cellList = net->getCellListPtr();
		}	
    }



}
*/

void Partitioner::partition()
{
	//TODO
	//initial partition
	initParti();
	
	//set initial gain
	setInitG();


/*
	//iteratively until no improve
	do{
		//for loop until all lock
		for(_iterNum = 0; _iterNum <_cellNum; ++_iterNum){
			//move the node with max gain
			Node* node = _maxGainCell;
			Cell* cell = _cellArray[node->getId()];
			cell->move();


			//update gain, size, lock
		
		}
		//find max subseq

			
		
	} while(_maxAccGain > 0);
*/
}

void Partitioner::printSummary() const
{
    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cutsize: " << _cutSize << endl;
    cout << " Total cell number: " << _cellNum << endl;
    cout << " Total net number:  " << _netNum << endl;
    cout << " Cell Number of partition A: " << _partSize[0] << endl;
    cout << " Cell Number of partition B: " << _partSize[1] << endl;
    cout << "=================================================" << endl;
    cout << endl;
    return;
}

void Partitioner::reportNet() const
{
    cout << "Number of nets: " << _netNum << endl;
    for (size_t i = 0, end_i = _netArray.size(); i < end_i; ++i) {
        cout << setw(8) << _netArray[i]->getName() << ": ";
        vector<int>* cellList = _netArray[i]->getCellListPtr();
        for (size_t j = 0, end_j = cellList->size(); j < end_j; ++j) {
            cout << setw(8) << _cellArray[(*cellList)[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::reportCell() const
{
    cout << "Number of cells: " << _cellNum << endl;
    for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i) {
        cout << setw(8) << _cellArray[i]->getName() << ": ";
        vector<int>* netList = _cellArray[i]->getNetListPtr();
        for (size_t j = 0, end_j = netList->size(); j < end_j; ++j) {
            cout << setw(8) << _netArray[(*netList)[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::writeResult(fstream& outFile)
{
    stringstream buff;
    buff << _cutSize;
    outFile << "Cutsize = " << buff.str() << '\n';
    buff.str("");
    buff << _partSize[0];
    outFile << "G1 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 0) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    buff.str("");
    buff << _partSize[1];
    outFile << "G2 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 1) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    return;
}

void Partitioner::clear()
{
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        delete _cellArray[i];
    }
    for (size_t i = 0, end = _netArray.size(); i < end; ++i) {
        delete _netArray[i];
    }
    return;
}
