#include <vector>

#include "Conway.h"

Cell::Cell(Position __pos)
	: _pos(__pos), _age(0), _state(CellState::cs_living)
{
}

CellState Cell::GetState()
{
	return _state;
}

Conway::Conway(
	int __underpopulation, int __overpopulation, int __reproduction,
	int __width, int __height
)
	: _underpopulation(__underpopulation), _overpopulation(__overpopulation), _reproduction(__reproduction),
	  _day(0), _width(__width), _height(__height)
{
	_field.resize(_width, std::vector<std::shared_ptr<Cell>>(_height, nullptr));
}

void Conway::Start(std::vector<Position> _startPattern)
{
	for (auto i = 0; i < _startPattern.size(); ++i)
	{
		std::shared_ptr<Cell> cell = std::make_shared<Cell>(_startPattern[i]);		
		UpdateOccupancy(cell, _startPattern[i]);
	}
}

void Conway::Update()
{
	++_day;





}

void Conway::UpdateOccupancy(std::shared_ptr<Cell> _cell, Position _pos)
{
	int x = std::get<0>(_pos);
	int y = std::get<1>(_pos);
	if (x >= 0 && y >= 0 && x < _width && y < _height)
	{
		_field[std::get<0>(_pos)][std::get<1>(_pos)] = _cell;
	}
}

int Conway::GetWidth()
{
	return _width;
}

int Conway::GetHeight()
{
	return _height;
}

int Conway::GetDay()
{
	return _day;
}

const std::vector<std::vector<std::shared_ptr<Cell>>>& Conway::GetField()
{
	return _field;
}
