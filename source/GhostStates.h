#ifndef GHOSTSTATES_H
#define GHOSTSTATES_H

#include "PacManGame.h"

class CBinkyState : public CGhostState
{
public:
	CBinkyState() : CGhostState(Type::BinkyChase) {};
	virtual void update(const CGhostStateContex& contex) override;
};

class CPinkyState : public CGhostState
{
public:
	CPinkyState() : CGhostState(Type::PinkyChase) {};
	virtual void update(const CGhostStateContex& contex) override;
};

class CInkyState : public CGhostState
{
public:
	CInkyState(CGhost* partner);
	virtual void update(const CGhostStateContex& contex) override;
	virtual void draw(const CGhostStateContex& state_contex, sf::RenderWindow* window) override;
private:
	CGhost* m_partner;
	Vector m_target_cell;
};

class CClydeState : public CGhostState
{
public:
	CClydeState() : CGhostState(Type::ClydeChase) {};
	virtual void update(const CGhostStateContex& contex) override;
};

class CFrightenedState : public CGhostState
{
public:
	static CFrightenedState* getInstance();
	virtual void update(const CGhostStateContex& contex) override;
	virtual void activate(const CGhostStateContex& contex) override;
	virtual void deactivate(const CGhostStateContex& contex) override;
	virtual void draw(const CGhostStateContex& contex, sf::RenderWindow* window);
	static void setFlashed(bool flashed);
private:
	CFrightenedState();
	static CFrightenedState* m_instance;
	std::map<std::string, sf::Color> m_old_color;
	std::map<std::string, float> m_time;
	sf::Color m_frightened_color = sf::Color(0, 0, 200);
	static bool m_flashed;
};

class CScatterState : public CGhostState
{
public:
	enum Corner { left_bottom, left_up, right_bottom, right_up };
	CScatterState(Corner corner);
	virtual void update(const CGhostStateContex& contex) override;
	virtual void activate(const CGhostStateContex& contex) override;
	virtual void deactivate(const CGhostStateContex& contex) override;
private:
	Corner m_corner;
	Vector m_corner_cell;
	int m_stage = 0;
};

class CSoulState : public CGhostState
{
public:
	CSoulState(Vector ghost_house_door);
	virtual void update(const CGhostStateContex& contex) override;
	virtual void activate(const CGhostStateContex& contex) override;
	virtual void draw(const CGhostStateContex&, sf::RenderWindow* window) override;
private:
	Vector m_ghost_house_door;
	int m_stage;
};

class CBorningState : public CGhostState
{
public:
	CBorningState(Vector ghost_house_door);
	virtual void update(const CGhostStateContex& contex) override;
	virtual void activate(const CGhostStateContex& contex) override;
private:
	Vector m_ghost_house_door;
	int m_stage;
};

class CInHouseState : public CGhostState
{
public:
	CInHouseState(Vector ghost_house_door);
	virtual void update(const CGhostStateContex& contex) override;
	virtual void activate(const CGhostStateContex& contex) override;
private:
	Vector m_ghost_house_door;
};

class CToyState : public CGhostState
{ 
public:
	enum class State {Binky, Pinki, Inky, Clyde, Frightened,Soul};
private:
	State m_state = State::Binky;
public:
	CToyState(State state);
	virtual void update(const CGhostStateContex& contex) override;
	virtual void draw(const CGhostStateContex& contex, sf::RenderWindow* window);
	virtual void activate(const CGhostStateContex& contex);
	virtual void  deactivate(const CGhostStateContex& contex);
};

#endif