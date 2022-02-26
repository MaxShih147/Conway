#include "Conway.h"


ConwayPattern octagon_2(
	"Octagon 2", 5,
	{ { 0, 3 }, { 0, 4 }, { 1, 2 }, { 1, 5 }, { 2, 1 }, { 2, 6 }, { 3, 0 }, { 3, 7 },
	{ 4, 0 }, { 4, 7 }, { 5, 1 }, { 5, 6 }, { 6, 2 }, { 6, 5 }, { 7, 3 }, { 7, 4 } }
);


Cell::Cell(Position __pos)
	: _pos(__pos), _age(0)/*, _state(CellState::cs_living)*/
{
}

void Cell::Update()
{
	++_age;
}

int Cell::GetAge()
{
	return _age;
}


//CellState Cell::GetState()
//{
//	return _state;
//}

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
		Create(cell/*, _startPattern[i]*/);
	}
}

void Conway::Update()
{
	++_day;

	// ! Maybe it is a stupid way...
	std::vector<std::vector<bool>> _gird(_width, std::vector<bool>(_height, false));

	for (auto i = 0; i < _width; ++i)
	{
		for (auto j = 0; j < _height; ++j)
		{
			_gird[i][j] =
				_field[i][j] != nullptr /*&&
				_field[i][j]->GetState() == CellState::cs_living*/;
		}
	}

	auto GetLivingNeighborCount = [=](int _x, int _y)
	{
		int result = 0;
		for (auto i : { -1, 0, 1 })
		{
			int x = _x + i;
			if (x < 0 || x >= _width)
			{
				continue;
			}
			for (auto j : { -1, 0, 1 })
			{
				int y = _y + j;
				if (y < 0 || y >= _height)
				{
					continue;
				}

				if (!(i == 0 && j == 0) && _gird[x][y])
				{
					++result;
				}
			}
		}

		return result;
	};

	for (auto i = 0; i < _width; ++i)
	{
		for (auto j = 0; j < _height; ++j)
		{
			auto n = GetLivingNeighborCount(i, j);

			if (n < 2) 
			{
				// Check underpopulation
				Kill(_field[i][j]);
			}
			else if (n > 3)
			{
				// Check overpopulation
				Kill(_field[i][j]);
			}
			else if (n == 3)
			{
				// Check reproduction
				if (_field[i][j] == nullptr)
				{
					std::shared_ptr<Cell> cell = std::make_shared<Cell>(Position(i, j));
					Create(cell);
				}
				else
				{
					_field[i][j]->Update();
				}
			}
			else
			{
				// keep your last state
				if (_field[i][j] != nullptr)
				{
					_field[i][j]->Update();
				}
			}
		}
	}
}

void Conway::Create(std::shared_ptr<Cell> _cell/*, Position _pos*/)
{
	int x = std::get<0>(_cell->_pos);
	int y = std::get<1>(_cell->_pos);
	if (x >= 0 && y >= 0 && x < _width && y < _height)
	{
		_field[x][y] = _cell;
	}
}

void Conway::Kill(std::shared_ptr<Cell>& _cell)
{
	if (_cell != nullptr)
	{
		_cell.reset();
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

ConwayPattern::ConwayPattern(std::string __name, int __period, std::vector<Position> __pattern)
	: _name(__name), _period(__period), _pattern(__pattern)
{
}