#pragma once

#include <vector>
#include <string>
#include <memory>

typedef std::pair<int, int> Position;

//enum class CellState
//{
//	cs_dead = 0,
//	cs_living = 1
//};

class Cell
{
public:
	Cell(Position __pos);
public:
	void Update();
	int GetAge();
	int GetIndex();
	//CellState GetState();
public:
	Position _pos;
private:
	int _age;
	//CellState _state;
};

class Conway
{
public:
	Conway(
		int __underpopulation = 2, int __overpopulation = 3, int __reproduction = 3,
		int __width = 30, int __height = 30
	);
public:
	// ! Run Section
	void Start(std::vector<Position> _startPattern);
	void Update(); // Start a new day.

public:
	// ! Field Section
	//void SetSize(int __width = 100, int __height = 100);
	void Create(std::shared_ptr<Cell> _cell);
	void Kill(std::shared_ptr<Cell>& _cell);

public:
	// ! Utility Section
	int GetWidth();
	int GetHeight();
	int GetDay();

	// ! Maybe dangerous
	const std::vector<std::vector<std::shared_ptr<Cell>>>& GetField();

private:
	int _day;
	std::vector<std::vector<std::shared_ptr<Cell>>> _field;
private:
	// ! Rules Section
	//	Any live cell with fewer than two live neighbours dies, as if by underpopulation.
	//	Any live cell with two or three live neighbours lives on to the next generation. 
	//	Any live cell with more than three live neighbours dies, as if by overpopulation.
	//	Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
	int _underpopulation; // defalut = 2
	int _overpopulation;  // default = 3
	int _reproduction;    // default = 3
private:
	int _width;
	int _height;
};

// ! For test...
class ConwayPattern
{
public:
	ConwayPattern(std::string __name, int __period, std::vector<Position> __pattern);
public:
	std::string _name;
	int _period;
	std::vector<Position> _pattern;
};