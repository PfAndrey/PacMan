#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <queue>
#include <map>
#include <iostream>
#include "assert.h"

class Vector
{
public:
	float x, y;
	Vector(float x, float y);
	Vector(int x, int y) :x((float)x), y((float)y) {};
	Vector();
	Vector operator-() const;
	Vector operator+ (const Vector& two) const;
	Vector operator- (const Vector& two) const;
	Vector operator* (const float& k) const;
	Vector operator/ (const float& k) const;
	void operator+= (const Vector& two);
	void operator-= (const Vector& two);
	bool operator == (const Vector& other) const;
	bool operator != (const Vector& other) const;
	void operator*= (const float& k);
	float length() const;
	float angle() const;
	Vector normalized() const;
	static Vector moveTowards(const Vector& current, const Vector& target, float distance)
	{
		return (target - current).normalized()*distance + current;
	}
	bool operator < (const Vector& other) const;
	const static Vector right;
	const static Vector left;
	const static Vector up;
	const static Vector down;
	const static Vector zero;
	//-----------------------------------------------
	Vector(const sf::Vector2f& vector);
	operator sf::Vector2f() const;
};




std::ostream& operator << (std::ostream& str, const Vector& vector);
Vector toVector(const std::string& str);
Vector rotateClockwise(const Vector& direction);
Vector rotateAnticlockwise(const Vector& direction);

namespace std
{
	template<>
	struct hash<Vector>
	{
		size_t operator()(const Vector& v) const
		{
			return int(v.y) * 10000 + int(v.x);
		}
	};
};



class Rect
{
public:
	Rect();
	Rect(float left, float top, float width, float height);
	Rect(const Vector& top_left, const Vector& size);
	float top() const;
	float left() const;
	float right() const;
	float bottom() const;
	Vector leftTop() const;
	Vector rightTop() const;
	Vector leftBottom() const;
	Vector rightBottom() const;
	Vector center() const;
	Vector size() const;
	bool isContain(const Vector& point) const;
	bool isContain(const Rect& rect) const;
	bool isContainByX(const Vector& point) const;
	bool isContainByY(const Vector& point) const;
	bool isIntersect(const Rect& other) const;
	Rect getIntersection(const Rect& other) const;
	Rect bordered(float k) const;
	Rect operator* (float k) const;
	bool operator==(const Rect& other) const;
	float width() const;
	float height() const;
	void setWidth(int width);
	void setHeight(int height);
	void setTop(int top);
	void setBottom(int bottom);
	void setRight(int right);
	void setLeft(int left);
	void normalize();
	Rect moved(const Vector& diff) const;
private:
	float _left, _top, _width, _height;
};

 

Vector operator* (const float& k, const Vector& vector);
 
 
Vector round(const Vector& vector);
Vector floor(const Vector& vector);




class Verticle;


class Edge
{
public:
	Edge(Verticle* begin, Verticle* end, int value);
	int value;
	Verticle* beginVerticle();
	Verticle* endVerticle();
	void setBegin(Verticle* verticle);
	void setEnd(Verticle* verticle);
	bool isAdjacent(Verticle* verticle)
	{
		if (m_begin == verticle || m_end == verticle)
			return true;

		return false;
	}
private:
	Verticle* m_begin;
	Verticle* m_end;
};

class Verticle
{
public:
	Vector position;
	void connectEdge(Edge* edge);
	void disconnectEdge(Edge* edge);
	std::vector<Edge*>::const_iterator edges_begin() const;
	std::vector<Edge*>::const_iterator edges_end() const;
	std::vector<Verticle*> getIncidentVerticles();
	bool isAdjacent(Edge* edge);
	void clearEdges();
	std::vector<Edge*> getEdgesList() const;
private:
	std::vector<Edge*> m_edges;
};

class Graph
{
public:
	void clear();
	Verticle* addVerticle(const Vector& pos);
	Verticle* getVerticleByPos(const Vector& pos);
	Verticle* getVerticleByPos(float x, float y);
	Edge* getEdge(Verticle* one, Verticle* two);
	void removeEdge(Edge* e);
	void removeVerticle(Verticle* v);
	Edge* addEdge(Verticle* begin, Verticle* end, int value);
	void addEdge(Edge* edge);
	std::vector<Verticle*> findPath(Verticle* start, Verticle* finish);

private:
	std::unordered_map<Vector, Verticle*> pos_to_verticles;
	std::vector<Verticle*> m_verticles;
	std::vector<Edge*> m_edges;

};


int getLength(const std::vector<Verticle*>& path);

int getLength(const std::vector<Vector>& path);


#endif 
