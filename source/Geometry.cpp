#include "Geometry.h"
#include <cmath>

Vector rotateClockwise(const Vector& direction)
{
	return Vector(-direction.y, direction.x);
}


Vector rotateAnticlockwise(const Vector& direction)
{
	return Vector(direction.y, -direction.x);
}

int getLength(const std::vector<Verticle*>& path)
{
	int length = 0;
	for (int i = 1; i < path.size(); ++i)
		length += (path[i]->position - path[i - 1]->position).length();
	return length;
}

int getLength(const std::vector<Vector>& path)
{
	int length = 0;
	for (int i = 1; i < path.size(); ++i)
		length += (path[i] - path[i - 1]).length();
	return length;
}
 
Vector toVector(const std::string& str)
{
	 if (str == "Left" || str == "left") return Vector::left;
	 if (str == "Right" || str == "right") return Vector::right;
	 if (str == "Down" || str == "down") return Vector::down;
	 if (str == "Up" || str == "up") return Vector::up;
	return Vector::zero;
}

//-----------------------------------------------------------------------------

Vector::Vector(float x, float y) : x(x), y(y) {}
Vector::Vector()
{
	x = y = 0;
}
Vector Vector::operator + (const Vector& two) const
{
	return Vector(x + two.x, y + two.y);
}
Vector Vector::operator- (const Vector& two) const
{
	return Vector(x - two.x, y - two.y);
}
Vector Vector::operator* (const float& k) const
{
	return Vector(x*k, y*k);
}
void Vector::operator*= (const float& k)
{
	x *= k;
	y *= k;
}
void Vector::operator+= (const Vector& two)
{
	x += two.x;
	y += two.y;
}

void Vector::operator-= (const Vector& two)
{
	x -= two.x;
	y -= two.y;
}

Vector Vector::operator/ (const float& k) const
{
	return Vector(x / k, y / k);
}

Vector Vector::operator-() const
{
	return Vector(-x, -y);
}

bool Vector::operator == (const Vector& other) const
{
	return (other.x == x && other.y == y);
}
bool Vector::operator != (const Vector& other) const
{
	return (other.x != x || other.y != y);
}

float Vector::length() const
{
	return sqrt(x*x + y*y);
}
float Vector::angle() const
{
	float angle = atan2(x, y) / 3.1415 * 180;
	if (angle < 0)
		angle += 360;

	return angle;
}

Vector Vector::normalized() const
{
	auto l = length();
	return Vector(x / l, y / l);
}


Vector::Vector(const sf::Vector2f& vector)
{
	x = vector.x;
	y = vector.y;
}
Vector::operator sf::Vector2f() const
{
	return sf::Vector2f(x, y);
}

Vector round(const Vector& vector)
{
	return Vector(std::roundf(vector.x), std::roundf(vector.y));
}

Vector floor(const Vector& vector)
{
	return Vector(int(vector.x), int(vector.y));
}

Vector operator* (const float& k, const Vector& vector)
{
	return Vector(vector.x*k, vector.y*k);
}

 



bool Vector::operator < (const Vector& other) const
{
	return y * 10000 + x < other.y * 10000 + other.x;
}
 

const Vector Vector::right = {1,0};
const Vector Vector::left = {-1,0};
const Vector Vector::up = {0,-1};
const Vector Vector::down = {0,1};
const Vector Vector::zero = {0,0};

//----------------------------------------------------------------------------------

Rect::Rect()
{
	_left = _top = _width = _height = 0;
}
Rect::Rect(float left, float top, float width, float height) : _left(left), _top(top), _width(width), _height(height)
{

}

Rect::Rect(const Vector& top_left, const Vector& size)
{
	_left = top_left.x;
	_top = top_left.y;
	_width = size.x;
	_height = size.y;
}

float Rect::right() const
{
	return _left + _width;
}
float Rect::bottom() const
{
	return _top + _height;
}

Vector Rect::leftTop() const
{
	return{ _left,_top };
}

Vector Rect::rightTop() const
{
	return{ right(),_top };
}


Vector Rect::leftBottom() const
{
	return{ _left,bottom() };
}

Vector Rect::rightBottom() const
{
	return{ right(),bottom() };
}

Vector Rect::center() const
{
	return{ _left + _width / 2, _top + _height / 2 };
}
Vector Rect::size() const
{
	return{ _width,_height };
}

std::ostream& operator << (std::ostream& str, const Vector& vector)
{
	str << "[" << vector.x << "," << vector.y << "]";
	return str;
}


bool Rect::isContain(const Vector& point) const
{
	if (point.x >= _left && point.y >= _top &&
		point.x < right() && point.y < bottom())
		return true;
	return false;
}

bool Rect::isContain(const Rect& other_rect) const
{
	if (other_rect._left >= _left && other_rect.right() <= right() &&
		other_rect._top >= _top && other_rect.bottom() <= bottom())
		return true;
	return false;
}


bool Rect::isContainByX(const Vector& point) const
{
	if (point.x >= _left  && point.x < right())
		return true;
	return false;
}
bool Rect::isContainByY(const Vector& point) const
{
	if (point.y >= _top && point.y < bottom())
		return true;
	return false;
}


bool Rect::isIntersect(const Rect& other) const
{
	//bool x_intersect = (_left <  other._left && right() <  other._left) || (_left >  other.right() && right() > other.right());
	//bool y_intersect = (_top < other._top && bottom() < other._top)     || (_top > other.bottom() && bottom() >  other.bottom());
	//return !x_intersect && !y_intersect;
	
	
	return     abs(2*(_left - other._left) + _width  - other._width ) < abs(_width  + other._width )
	        && abs(2*(_top - other._top) + _height   - other._height ) < abs(_height + other._height );	
}


Rect Rect::getIntersection(const Rect& other) const
{
	Rect new_rect;
	new_rect._left = std::max(_left, other._left);
	new_rect._top = std::max(_top, other._top);
	new_rect._height = std::min(bottom(), other.bottom()) - new_rect._top;
	new_rect._width = std::min(right(), other.right()) - new_rect._left;

	return new_rect;
}


Rect Rect::bordered(float k) const
{
	return{ _left - k, _top - k, _width + 2 * k, _height + 2 * k };
}

Rect Rect::operator* (float k) const
{
	return{ _left*k,_top*k,_width*k,_height*k };
}
float Rect::top() const
{
	return _top;
}
float Rect::left() const
{
	return _left;
}

bool Rect::operator==(const Rect& other) const
{
	if (_left == other._left && _top == other._top &&
		_width == other._width && _height == other._height)
		return true;
	return false;
}

float Rect::width() const
{
	return _width;
}

float Rect::height() const
{
	return _height;
}

void Rect::setWidth(int width)
{
	_width = width;
}

void Rect::setHeight(int height)
{
	_height = height;
}

void Rect::setTop(int top)
{
	_height += _top - top;
	_top = top;
}
void Rect::setBottom(int bottom)
{
	_height = bottom - _top;
}
void Rect::setRight(int right)
{
	_width =   right - _left;
}
void Rect::setLeft(int left)
{
	_width += _left - left;
	_left = left;
}
void Rect::normalize()
{
	if (_width < 0)
	{
		_left += _width;
		_width = -_width;
	}
	if (_height < 0)
	{
		_top += _height;
		_height = -_height;
	}
}

Rect Rect::moved(const Vector& diff) const
{
	return Rect(_left + diff.x, _top + diff.x, _width, _height);
}

//----------------------------------------------------------------------------------
 Edge::Edge(Verticle* begin, Verticle* end, int _value)
 {
	value = _value;
	setBegin(begin);
	setEnd(end);
 }
	Verticle* Edge::beginVerticle()
	{
		return m_begin;
	}
	Verticle* Edge::endVerticle()
	{
		return m_end;
	}
	void Edge::setBegin(Verticle* verticle)
	{
		m_begin = verticle;
	}
	void Edge::setEnd(Verticle* verticle)
	{
		Edge::m_end = verticle;
	}

//----------------------------------------------------------------------------------


	void Verticle::connectEdge(Edge* edge)
	{
		m_edges.push_back(edge);
	}
	void Verticle::disconnectEdge(Edge* edge)
	{
		auto it = std::find(m_edges.begin(), m_edges.end(), edge);
		if (it != m_edges.end())
			m_edges.erase(it);
	}
	std::vector<Edge*>::const_iterator Verticle::edges_begin() const
	{
		return m_edges.cbegin();
	}
	std::vector<Edge*>::const_iterator Verticle::edges_end() const
	{
		return m_edges.cend();
	}
	std::vector<Verticle*> Verticle::getIncidentVerticles()
	{
		std::vector<Verticle*> result;

		for (auto& e : m_edges)
			if (e->beginVerticle() == this)
				result.push_back(e->endVerticle());
			else
				result.push_back(e->beginVerticle());
		return result;
	}
	bool Verticle::isAdjacent(Edge* edge)
	{
		for (auto& e : m_edges)
			if (e == edge)
				return true;
		return false;
	}
	void Verticle::clearEdges()
	{

	}
	std::vector<Edge*> Verticle::getEdgesList() const
	{
		return m_edges;
	}

//----------------------------------------------------------------------------------

	void Graph::clear()
	{
		for (auto v : m_verticles)
			delete v;
		for (auto e : m_edges)
			delete e;
		m_edges.clear();
		m_verticles.clear();
		pos_to_verticles.clear();
	}
	
	Verticle* Graph::addVerticle(const Vector& pos)
	{
		assert(!pos_to_verticles[pos]); //already exist
		Verticle* verticle = new Verticle();
		verticle->position = pos;
		m_verticles.push_back(verticle);

		pos_to_verticles[pos] = verticle;
		return verticle;
	}

	Verticle* Graph::getVerticleByPos(const Vector& pos)
	{
		return pos_to_verticles[pos];
	}

	Verticle* Graph::getVerticleByPos(float x, float y)
	{
		return getVerticleByPos({ x,y });
	}

	void Graph::removeEdge(Edge* e)
	{
		assert(e);
		auto it = std::find(m_edges.begin(), m_edges.end(), e);

		if (it != m_edges.end())
		{
			if (e->beginVerticle()) e->beginVerticle()->disconnectEdge(e);
			if (e->endVerticle()) e->endVerticle()->disconnectEdge(e);
			delete *it;
			m_edges.erase(it);
		}
		
	}

	void Graph::removeVerticle(Verticle* v)
	{
		assert(v);
		auto it = std::find(m_verticles.begin(), m_verticles.end(), v);

		if (it != m_verticles.end())
		{
			auto edges_list = v->getEdgesList();
			for (auto& edge : edges_list)
				removeEdge(edge);
			pos_to_verticles[(*it)->position] = NULL;
			delete *it;
			m_verticles.erase(it);
		}
	}

	void Graph::addEdge(Edge* edge)
	{
		edge->beginVerticle()->connectEdge(edge);
		edge->endVerticle()->connectEdge(edge);
		m_edges.push_back(edge);
	}

	Edge* Graph::addEdge(Verticle* begin, Verticle* end, int value)
	{
		Edge* edge = new Edge(begin,end,value);
		addEdge(edge);
		return edge;
	}

	std::vector<Verticle*> Graph::findPath(Verticle* start, Verticle* finish)
	{
		assert(start && finish);

		auto getHeuristicValue = [&finish](Verticle* v) {return abs(v->position.x - finish->position.x) + abs(v->position.y - finish->position.y); };

		std::unordered_map<Verticle*, bool> close_list;
		std::unordered_map<Verticle*, Verticle*> parent_list;
		std::unordered_map<Verticle*, int> value_list;


		auto cmp = [&value_list](Verticle* a, Verticle* b) { return value_list[a] > value_list[b]; };

		std::priority_queue<Verticle*, std::vector<Verticle*>, decltype(cmp)> open_list(cmp);


		open_list.push(start);
		Verticle* current_verticle = NULL;
		while (!open_list.empty())
		{
			current_verticle = open_list.top();
			int current_value = value_list[current_verticle];

			if (current_verticle == finish)
				break;

			open_list.pop();
			//close_list[current_verticle] = true;

			assert(current_verticle);
			auto verticles = current_verticle->getIncidentVerticles();
			for (auto& v : verticles)
				if (!close_list[v])
				{
					parent_list[v] = current_verticle;
					close_list[v] = true;
					int neighbor_value = getEdge(current_verticle, v)->value;
					value_list[v] = (current_value + neighbor_value) + getHeuristicValue(v);
					open_list.push(v);
				}
		}

		std::vector<Verticle*> path;
		if (current_verticle == finish)
			while (current_verticle != start)
			{
				path.push_back(current_verticle);
				current_verticle = parent_list[current_verticle];
			}
		path.push_back(start);

		return path;
	}


	Edge* Graph::getEdge(Verticle* one, Verticle* two)
	{
		for (auto e = one->edges_begin(); e != one->edges_end(); ++e)
			if ((*e)->isAdjacent(two))
				return *e;
		return NULL;

	}
