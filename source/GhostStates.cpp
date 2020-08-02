#include "GhostStates.h"

sf::Vector2f toSfVec2f(const Vector& vec)
{
	return sf::Vector2f(vec.x, vec.y);
}

void CPinkyState::update(const CGhostStateContex& contex)
{
	if (!contex.ghost->isMoving())
	{
		Vector start_cell = contex.walls->toMapCoordinates(contex.ghost->getPosition());
		Vector target_cell = contex.walls->toMapCoordinates(contex.ghost->target()->getPosition());

		Vector pinky_target = contex.walls->getMap()->getCell(target_cell, contex.ghost->target()->getDirection(), 4);

		if (contex.walls->inBounds(pinky_target) && contex.walls->getMapCell(pinky_target) == EMapBrickTypes::empty)
			target_cell = pinky_target;

		//if closer than 4 cells -> straight catch 
		contex.ghost->moveToTarget(contex.walls->toPixelCoordinates(target_cell));
	}
}

//-------------------------------------------------------------------------------------------------

void CBinkyState::update(const CGhostStateContex& contex)
{
	if (!contex.ghost->isMoving()) //time to update moving direction
		contex.ghost->moveToTarget(contex.ghost->target()->getPosition());

}
//-------------------------------------------------------------------------------------------------
CInkyState::CInkyState(CGhost* partner) : CGhostState(Type::InkyChase)
{
	m_partner = partner;
}

void CInkyState::update(const CGhostStateContex& contex)
{
	if (!contex.ghost->isMoving())
	{
		Vector ghost_cell = contex.walls->toMapCoordinates(contex.ghost->getPosition());
		Vector pacman_cell = contex.walls->toMapCoordinates(contex.ghost->target()->getPosition());
		Vector partner_cell = contex.walls->toMapCoordinates(m_partner->getPosition());
		Vector pivot_cell = pacman_cell + 2 * contex.ghost->target()->getDirection();
		m_target_cell = pivot_cell - (partner_cell - pivot_cell);
		contex.ghost->moveToTarget(contex.walls->toPixelCoordinates(m_target_cell));
	}
}

void CInkyState::draw(const CGhostStateContex& state_contex, sf::RenderWindow* window)
{
	CGhostState::draw(state_contex, window);
	sf::CircleShape target(20);
	target.setFillColor(sf::Color::Red);
	target.setPosition(toSfVec2f(state_contex.walls->toPixelCoordinates(m_target_cell)));
}

//-------------------------------------------------------------------------------------------------

void CClydeState::update(const CGhostStateContex& contex)
{
	if (!contex.ghost->isMoving())
	{
		Vector ghost_cell = contex.walls->toMapCoordinates(contex.ghost->getPosition());
		Vector pacman_cell = contex.walls->toMapCoordinates(contex.ghost->target()->getPosition());
		Vector left_bottom_cell = Vector(1, contex.walls->getMap()->height() - 2);
		Vector m_target;

		if ((ghost_cell - pacman_cell).length() > 8)
			m_target = pacman_cell;
		else
			m_target = left_bottom_cell;

		contex.ghost->moveToTarget(contex.walls->toPixelCoordinates(m_target));
	}
}

//-------------------------------------------------------------------------------------------------

CFrightenedState::CFrightenedState() : CGhostState(Type::Frightened)
{
	m_flashed = false;
}

void CFrightenedState::update(const CGhostStateContex& contex)
{
	if (!contex.ghost->isMoving())
	{
		Vector ghost_cell = contex.walls->toMapCoordinates(contex.ghost->getPosition());

		Vector target_cell;
		do
		{
			target_cell = Vector(std::rand() % contex.walls->getMap()->width(), std::rand() % contex.walls->getMap()->height());
		} while (contex.walls->getMapCell(target_cell) != EMapBrickTypes::empty);

		contex.ghost->moveToTarget(contex.walls->toPixelCoordinates(target_cell));
	}

	if (m_flashed)
	{
		m_time[contex.ghost->getName()] += contex.delta_time;
		int i = int(m_time[contex.ghost->getName()] / 25) % 20;
		if (i > 10)
			contex.ghost->setColor(m_frightened_color);
		else
			contex.ghost->setColor(m_old_color[contex.ghost->getName()]);
	}
}

void CFrightenedState::setFlashed(bool flashed)
{
	m_flashed = flashed;
}

void CFrightenedState::activate(const CGhostStateContex& contex)
{
	m_old_color[contex.ghost->getName()] = contex.ghost->color();
	contex.ghost->setColor(m_frightened_color);
	contex.ghost->setSpeed(contex.ghost->SCARRED_SPEED);
	m_time[contex.ghost->getName()] = 0;
	setFlashed(false);
}

void CFrightenedState::deactivate(const CGhostStateContex& contex)
{
	contex.ghost->setColor(m_old_color[contex.ghost->getName()]);
	contex.ghost->setSpeed(contex.ghost->NORMAL_SPEED);
}

void CFrightenedState::draw(const CGhostStateContex& state_contex, sf::RenderWindow* window)
{

	CGhostState::draw(state_contex, window);
	state_contex.ghost->drawMouth(window);
}

CFrightenedState* CFrightenedState::getInstance()
{
	if (m_instance == nullptr)
		m_instance = new 	CFrightenedState();
	return m_instance;
}

CFrightenedState* CFrightenedState::m_instance = nullptr;
bool CFrightenedState::m_flashed = false;

//--------------------------------------------------------------------------------------------------

CScatterState::CScatterState(Corner corner) : CGhostState(Type::Scatter)
{
	m_corner = corner;
}

void CScatterState::update(const CGhostStateContex& contex)
{
	Vector ghost_cell = contex.walls->toMapCoordinates(contex.ghost->getPosition());

	if (!contex.ghost->isMoving())
		contex.ghost->moveToTarget(contex.walls->toPixelCoordinates(m_corner_cell));
}

void CScatterState::activate(const CGhostStateContex& contex)
{
	m_stage = 0;
	Vector map_size(contex.walls->getMap()->width(), contex.walls->getMap()->height());

	switch (m_corner)
	{
	case(Corner::left_up):
	{
		m_corner_cell = Vector(1, 1);
		break;
	}
	case(Corner::right_up):
	{
		m_corner_cell = Vector((int)map_size.x - 2, 1);
		break;
	}
	case(Corner::left_bottom):
	{
		m_corner_cell = Vector(1, (int)map_size.y - 2);
		break;
	}
	case(Corner::right_bottom):
	{
		m_corner_cell = Vector((int)map_size.x - 2, (int)map_size.y - 2);
		break;
	}
	}
}

void CScatterState::deactivate(const CGhostStateContex& contex)
{

}

//--------------------------------------------------------------------------------------------------

CSoulState::CSoulState(Vector ghost_house_door) : CGhostState(Type::Soul)
{
	m_ghost_house_door = ghost_house_door;
}

void CSoulState::update(const CGhostStateContex& contex)
{
	Vector ghost_cell = contex.walls->toMapCoordinates(contex.ghost->getPosition());

	switch (m_stage)
	{
		case(0):
		{
			if (!contex.ghost->isMoving())
			{
				contex.ghost->moveToTarget(contex.walls->toPixelCoordinates(m_ghost_house_door - Vector(0, 1)));
			}

			if (round(ghost_cell) == round(m_ghost_house_door) - Vector(0, 1))
			{
				contex.ghost->stop();
				m_stage = 1;
			}

			break;
		}
		case(1):
		{
			if (!contex.ghost->isMoving())
			{
				contex.ghost->moveToTarget(contex.walls->toPixelCoordinates(m_ghost_house_door + Vector(0, 2)));
				m_stage = 2;
			}
			break;
		}

	}
}

void CSoulState::activate(const CGhostStateContex& contex)
{
	m_stage = 0;
}

void CSoulState::draw(const CGhostStateContex& contex, sf::RenderWindow* window)
{
	contex.ghost->drawEyes(window);
}

//--------------------------------------------------------------------------------------------------

CBorningState::CBorningState(Vector ghost_house_door) : CGhostState(Type::Borning)
{
	m_ghost_house_door = ghost_house_door;
}

void CBorningState::update(const CGhostStateContex& contex)
{

}

void CBorningState::activate(const CGhostStateContex& contex)
{
	std::vector<Vector> path =
	{
	  contex.ghost->getPosition(),
	  contex.walls->toPixelCoordinates(m_ghost_house_door + 1 * Vector::down),	  
	  contex.walls->toPixelCoordinates(m_ghost_house_door + 2 * Vector::up)
	};

	contex.ghost->setMovingPath(path);
}

//--------------------------------------------------------------------------------------------------

CInHouseState::CInHouseState(Vector ghost_house_door) : CGhostState(Type::InHouse)
{
	m_ghost_house_door = ghost_house_door;
}

void CInHouseState::update(const CGhostStateContex& contex)
{

	if (!contex.ghost->isMoving())
	{
		Vector next_cell = contex.walls->toMapCoordinates(contex.ghost->getPosition()) + Vector::left;
		bool can_turn = !(!contex.walls->inBounds(next_cell) || contex.walls->getMapCell(next_cell) != EMapBrickTypes::empty);

		Vector direction = can_turn ? Vector::left : Vector::right;

		Vector player_cell = contex.walls->toMapCoordinates(contex.ghost->getPosition());
		Vector finish_cell = contex.walls->getMap()->traceLine(player_cell, direction, EMapBrickTypes::empty);

		contex.ghost->setMovingPath(contex.walls->toPixelCoordinates({ player_cell,finish_cell }));
	};

}

void CInHouseState::activate(const CGhostStateContex& contex)
{
	contex.ghost->stop();
	contex.ghost->setPosition(contex.walls->toPixelCoordinates(m_ghost_house_door + Vector(0, 2)) + Vector(10 * (rand() % 4), 0));
}

//--------------------------------------------------------------------------------------------------

CToyState::CToyState(State state) :CGhostState(Toy)
{
	m_state = state;
}

void CToyState::update(const CGhostStateContex& contex)
{

}

void CToyState::draw(const CGhostStateContex& contex, sf::RenderWindow* window)
{
	if (m_state != State::Soul)
		contex.ghost->drawBody(window);
	contex.ghost->drawEyes(window);

	if (m_state == State::Frightened)
		contex.ghost->drawMouth(window);
}

void CToyState::activate(const CGhostStateContex& contex)
{
	static sf::Color ghost_colors[] = { sf::Color(255,0,0),sf::Color(255,105,180), sf::Color(15, 255, 225), sf::Color(255, 140, 0) };

	if (m_state < State::Frightened)
		contex.ghost->setColor(ghost_colors[(int)m_state]);
	else
	{
		contex.ghost->setColor(sf::Color(0, 0, 200));
		contex.ghost->setSpeed(0.075);
	}
}

void CToyState::deactivate(const CGhostStateContex& contex)
{
	contex.ghost->setSpeed(0.15);
}
