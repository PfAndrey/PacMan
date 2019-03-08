#ifndef TILEMAP_H
#define TILEMAP_H

#include "Geometry.h"
#include <vector>
#include <functional>

const static Vector directions[] = { Vector::zero,Vector::left, Vector::up,Vector::down,Vector::right };

template<typename T>
class TileMap
{
public:
	TileMap(int width, int height)
	{
		m_width = width;
		m_height = height;
		m_map = new T*[width];
		for (int x = 0; x < width; ++x)
			m_map[x] = new T[height];
	}

	~TileMap()
	{
		for (int x = 0; x < m_width; ++x)
			delete m_map[x];
		delete m_map;
	}

	inline void setCell(int x, int y, T value)
	{
		assert(x < m_width && y < m_height && x >= 0 && y >= 0);
        m_map[x][y] = value;
	}

	inline const T& getCell(int x, int y) const
	{
		assert(x < m_width && y < m_height && x >= 0 && y >= 0);
		return m_map[x][y];
	}

	inline const T& getCell(const Vector& point) const
	{
		assert(point.x < m_width && point.y < m_height && point.x >= 0 && point.y >= 0);
		return m_map[(int)point.x][(int)point.y];
	}

	void clear(T value = T())
	{
		for (int x = 0; x < m_width; ++x)
			for (int y = 0; y < m_height; ++y)
				m_map[x][y] = value;
	}

	inline int width() const
	{
		return m_width;
	}

	inline int height() const
	{
		return m_height;
	}

	void loadFromFile(std::map<char, T> dictionary, const std::string& FilePath)
	{
		std::ifstream file;
		file.open(FilePath);
		if (!file.is_open())
			std::runtime_error("Can't load file: " + FilePath);
		std::string str;
		for (int y = 0; y < height(); ++y)
		{
			std::getline(file, str);
			assert(str.length() == m_width);
			for (int x = 0; x < m_width; ++x)
				m_map[x][y] =  dictionary[str[x]];
		}
	}

	bool inBounds(const Vector& cell) const
	{
		return cell.x >= 0 && cell.y >= 0 && cell.x < m_width && cell.y < m_height;
	}

    int getCellDegree(const Vector& cell, const T& cellType) const
    {
        if (getCell(cell) != cellType)
            return 0;

        int degree = 0;
        static const Vector deltas[]{ { 1,0 },{ -1,0 },{ 0,1 },{ 0,-1 } };

        for (auto& delta : deltas)
            if (inBounds(cell + delta) && getCell(cell + delta) == cellType)
                ++degree;

        return degree;
    }

    std::vector<Vector> getCells(T cell_type)
    {
        std::vector<Vector> cells;

        for (int x = 0; x < m_width; ++x)
            for (int y = 0; y < m_height; ++y)
                if (m_map[x][y] == cell_type)
                    cells.emplace_back(x, y);
        return 	cells;
    }

    Vector traceLine(const Vector& start_cell, const Vector& direction, const T& allowedCellType)
    {
        Vector curr_cell = floor(start_cell);
        assert(getCell(curr_cell) == allowedCellType);

        if (direction == Vector::zero)
            return curr_cell;

        while (inBounds(curr_cell) && getCell(curr_cell) == allowedCellType)
            curr_cell += direction;

        curr_cell += -direction;

        assert(getCell(curr_cell) == allowedCellType);
        return curr_cell;
    }

    Vector getCell(const Vector& start_cell, const Vector& direction, int length)
    {
        Vector cur_cell = start_cell;
        for (int i = 0; i < length; ++i)
            cur_cell +=  direction;
        return cur_cell;
    }

    std::vector<Vector> getNeighborNodes(const Vector& start_cell, const T& allowedCellType)
    {

        Vector curr_cell;
        std::vector<Vector> nodes;

        for (int i = 1; i < 5; ++i)
        {
            curr_cell = start_cell;

            while ((curr_cell == start_cell || getCellDegree(curr_cell, allowedCellType) < 3) && inBounds(curr_cell + directions[i]) && getCell(curr_cell + directions[i]) == allowedCellType)
                curr_cell += directions[i];

            if (curr_cell != start_cell && curr_cell.x != 0 && curr_cell.x != m_width - 1)
                nodes.push_back(curr_cell);
        }

        return nodes;
    }

	TileMap& operator=(const TileMap& other_map)
	{
		for (int x = 0; x < m_width; ++x)
			for (int y = 0; y < m_height; ++y)
				setCell(x, y, other_map.getCell(x, y));

		m_width = other_map.m_width;
		m_height = other_map.m_height;
		return *this;
	}

private:
	T** m_map;
	int m_height,m_width;
};

#endif TILEMAP_H





