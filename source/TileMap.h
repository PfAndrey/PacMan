#pragma once

#include "Geometry.h"
#include <Vector>
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
		m_path_finder_need_update = true;
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
	void fillRect(int x1, int y1, int width, int height, T value)
	{
		for (int x = x1; x < x1 + width; ++x)
			for (int y = y1; y < y1 + height; ++y)
				setCell(x, y, value);
	}
	void loadFromString(std::map<char, T> dictionary, const std::string& str)
	{
		
		clear();
		
		assert(width()*height() == str.length());
		int i = 0;
		for (size_t y = 0; y < height(); ++y)
			for (size_t x = 0; x < width(); ++x)
				setCell(x, y, dictionary[str[i++]]);
		m_path_finder_need_update = true;
	}
	void loadFromString(std::function<T(char)> fabric, const std::string& str)
	{
		clear();
		assert(width()*height() == str.length());
		int i = 0;
		for (int y = 0; y < height(); ++y)
			for (int x = 0; x < width(); ++x)
				setCell(x, y, fabric(str[i++]));
		m_path_finder_need_update = true;
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


		m_path_finder_need_update = true;
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
	bool isEqualRect(int x, int y, int w, int h, const std::function<bool(const T&)>& is_allowed_cell) const
	{
		for (int Y = y; Y < y + h; ++Y)
			for (int X = x; X < x + w; ++X)
				if (!is_allowed_cell(getCell(X, Y)))
					return false;
		return true;
	}
	void createFindGraph(const T& allowedCellType, int unit_size = 1)
	{
		m_find_graph.clear();
		const int edge_value = 10;

		//create verticles
		for (int y = 0; y < m_height; ++y)
			for (int x = 0; x < m_width; ++x)
				if (isEqualRect(x, y, unit_size, unit_size, allowedCellType))
					m_find_graph.addVerticle({ x, y });

		//create edges
		for (int y = 0; y < m_height - 1; ++y)
			for (int x = 0; x < m_width - 1; ++x)
			{
				Verticle* base = m_find_graph.getVerticleByPos(x, y);
				if (base)
				{
					Verticle* right = m_find_graph.getVerticleByPos(x + 1, y);
					Verticle* bottom = m_find_graph.getVerticleByPos(x, y + 1);

					if (right)
						m_find_graph.addEdge(base, right, edge_value);
					if (bottom)
						m_find_graph.addEdge(base, bottom, edge_value);
				}
			}
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
	std::vector<std::pair<Vector,T>> getCells(const Rect& rect)
	{
		std::vector<std::pair<Vector, T>> cells;

		for (int x = rect.left(); x < rect.right(); ++x)
			for (int y = rect.top(); y < rect.bottom(); ++y)
				cells.push_back(std::make_pair<>({ x,y }, getCell(x, y)));
			

		return cells;
	}
	std::vector<Vector> findPath(const Vector& start_cell, const Vector& finish_cell, const std::function<bool (const T&)>& is_allowed_cell, int unit_size = 1, const Rect& claster_rect = Rect())
	{

		Vector start = floor(start_cell);
		Vector finish = floor(finish_cell);

		assert(is_allowed_cell(getCell(start)));
		assert(is_allowed_cell(getCell(finish)));

		Rect bounds;
		if (claster_rect == Rect())
			bounds = Rect(0, 0, m_width - unit_size, m_height - unit_size );
		else
			bounds = claster_rect;

		auto getHeuristicValue = [&finish](Vector v) {return abs(v.x - finish.x) + abs(v.y - finish.y); };

		struct Info
		{
			Vector parent;
			int value = 0;
			int base_value = 0;
			bool in_closed_list = false;
		};
 
		std::unordered_map<Vector, Info> info_list;
 
		auto cmp = [&info_list](const Vector& a, const Vector& b) { return info_list[a].value > info_list[b].value; };
		std::priority_queue<Vector, std::vector<Vector>, decltype(cmp)> open_list(cmp);
 
		const Vector deltas[] = {{ 1,0 }, { 0,1 }, { -1,0 }, { 0,-1 }};

		open_list.push(start);
 
		bool finded = false;
		Vector current_cell;
		while (!open_list.empty())
		{
			current_cell = open_list.top();
			open_list.pop();
			info_list[current_cell].in_closed_list = true;

			if (current_cell == finish)
				break;
			
			for (auto& delta : deltas)
			{
				const Vector neighbor_cell = current_cell + delta;
				if (bounds.isContain(neighbor_cell) &&
					isEqualRect(neighbor_cell.x, neighbor_cell.y, unit_size, unit_size, is_allowed_cell) && !info_list[neighbor_cell].in_closed_list)
				{
					Info& neighbor = info_list[neighbor_cell];
					neighbor.parent = current_cell;
					neighbor.base_value = info_list[current_cell].base_value + 10;
					neighbor.value = neighbor.base_value + getHeuristicValue(neighbor_cell);
					open_list.push(neighbor_cell);
				}
			}
		}

		std::vector<Vector> path;
		if (current_cell == finish)
		{
			while (current_cell != start)
			{
				path.push_back(current_cell);
				current_cell = info_list[current_cell].parent;
			}

			path.push_back(start);

			for (int i = 0; i < path.size() / 2; ++i)
				std::swap(path[i], path[path.size() - i - 1]);
 		
			std::vector<Vector> optimized_path;

			optimized_path.push_back(path.front());
			for (int i = 1; i < path.size() - 1; ++i)
			{
				if (path[i-1].x != path[i+1].x &&  path[i-1].y != path[i+1].y)
				optimized_path.push_back(path[i]);
			}
			optimized_path.push_back(path.back());
			return optimized_path;
			
		}
 
		return path;
	
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

	Vector traceLine(const Vector& start_cell, const Vector& direction, const std::function<bool(T)>& allowed_cell)
	{
		Vector curr_cell = floor(start_cell);
		assert(allowed_cell(getCell(curr_cell)));

		if (direction == Vector::zero)
			return curr_cell;

		while (inBounds(curr_cell) && allowed_cell(getCell(curr_cell)))
			curr_cell += direction;


		curr_cell += -direction;

		assert(allowed_cell(getCell(curr_cell)));
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
	void updatePathFinder(const T& allowed_tile_type = T(0), int unit_size = 1)
	{
		createFindGraph(allowed_tile_type, unit_size);
	}
	bool m_path_finder_need_update = true;
	T** m_map;
	int m_height,m_width;
	Graph m_find_graph;
};





