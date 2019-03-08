#include "PacManGame.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include "GhostStates.h"
#include <math.h>

//--------------------------------------------------------------------------------------------
void CPacManGameScene::addScore(int score)
{
	m_score += score;
	m_score_label->setString("Score: " + toString(m_score));
}

void CPacManGameScene::resetScore()
{
	m_score = 0;

}

void CPacManGameScene::enableActors(bool value)
{   
	auto ghosts = findObjectsByType<CGhost>();
	auto pacman = findObjectByType<CPacman>();

	for (auto& ghost : ghosts)
		if (value)
			ghost->enable();
		else
			ghost->disable();

	if (value)
		  pacman->enable();
		else
		  pacman->disable();
}

CPacManGameScene::CPacManGameScene()
{
	std::cout << Vector::left.angle() << std::endl;
	std::cout << Vector::right.angle() << std::endl;
	std::cout << Vector::up.angle() << std::endl;
	std::cout << Vector::down.angle() << std::endl;

	CPacManGame::instance()->eventManager().subscribe(this);

	m_wave_timer = new CTimer();
	m_pill_timer = new CTimer();
	m_born_timer = new CTimer();
	addObject(m_wave_timer);
	addObject(m_pill_timer);
	addObject(m_born_timer);

	m_walls = new CWalls(28, 31);
	m_walls->getMap()->loadFromFile({ { '*',EMapBrickTypes::full },
	{ '.',EMapBrickTypes::dot },  { 'P', EMapBrickTypes::pacman_spawn },{ ' ', EMapBrickTypes::empty },
	{ 'p',EMapBrickTypes::pill }, {'G', EMapBrickTypes::ghost_spawn},{ 'F', EMapBrickTypes::fruit },
	{ '1',EMapBrickTypes::door_lu },{ '2',EMapBrickTypes::door_ru },
	{ '3',EMapBrickTypes::door_ld },{ '4',EMapBrickTypes::door_rd } },
		"res/stage1.txt");

	addObject(m_walls);

	addObject(m_dots = new CDots(m_walls));
	m_dots->fill(m_walls);

	auto pills_cells = m_walls->getMap()->getCells(EMapBrickTypes::pill);
	m_fruit_cell = m_walls->getMap()->getCells(EMapBrickTypes::fruit)[0];


	for (auto& pill_cell : pills_cells)
	{
		auto pill = new CPill();
		pill->setPosition(m_walls->toPixelCoordinates(pill_cell));
		addObject(pill);
		m_pills.push_back(pill);
	}

	addObject(m_pacman = new CPacman(m_walls));

	char* ghost_names[]{ "Binky", "Pinky", "Inky", "Clyde" };
	sf::Color ghost_colors[] = { sf::Color(255,0,0),sf::Color(255,105,180), sf::Color(15, 255, 225), sf::Color(255, 140, 0) };
	CScatterState::Corner ghost_corners[] = { CScatterState::Corner::left_bottom, CScatterState::Corner::left_up, CScatterState::Corner::right_bottom, CScatterState::Corner::right_up };
	Vector ghost_house_door_cell(13.5, 12);

	for (int i = 0; i < 4; ++i)
	{
		CGhost* ghost = new CGhost(ghost_names[i], m_pacman, m_walls);
		ghost->setColor(ghost_colors[i]);
		m_ghosts[i] = ghost;
		addObject(ghost);

		m_ghost_states[GhostStates::scatter][ghost_names[i]] = new CScatterState(ghost_corners[i]);
		m_ghost_states[GhostStates::frightened][ghost_names[i]] = CFrightenedState::getInstance();
		m_ghost_states[GhostStates::souls][ghost_names[i]] = new CSoulState(ghost_house_door_cell);
		m_ghost_states[GhostStates::borning][ghost_names[i]] = new CBorningState(ghost_house_door_cell);
		m_ghost_states[GhostStates::in_ghost_house][ghost_names[i]] = new CInHouseState(Vector(13.5, 12));
	}
	
	CGhostState* st[] = { new CBinkyState(), new CPinkyState(), new CInkyState(m_ghosts[0]), new CClydeState() };
	for (int i = 0; i < 4; ++i)
	{
		m_ghost_states[GhostStates::chase][ghost_names[i]] = st[i];
		m_ghost_chase_states[i] = st[i];
	}

	m_pacman_spawn_position = m_walls->getMap()->getCells(EMapBrickTypes::pacman_spawn)[0]+Vector(0.5,0);
	m_walls->lining();

	m_fruit = new CFruit();
	m_fruit->setPosition(m_walls->toPixelCoordinates(m_fruit_cell+ Vector(0.5,0)));
	addObject(m_fruit);

	m_fruit_timer = new CTimer();
	addObject(m_fruit_timer);
	m_fruit->disable();
	m_fruit->hide();

// GUI
	m_big_text = new CButton();
	m_big_text->setFontName(*CPacManGame::instance()->fontManager().get("main_font"));
	m_big_text->setFontStyle(sf::Text::Bold);
	m_big_text->setFontSize(30);
	m_big_text->setFontColor(sf::Color(50,50,50));
	m_big_text->setPosition( m_walls->size().x / 2 + 15, m_walls->size().y / 2 + 42);
	addObject(m_big_text);

	m_flow_text = new CFlowText(*CPacManGame::instance()->fontManager().get("arial"));
	addObject(m_flow_text);

	m_score_label = new CLabel();
	m_score_label->setBounds(770, 40, 140, 30);
	m_score_label->setFontName(*CPacManGame::instance()->fontManager().get("score_font"));
	m_score_label->setFontSize(32);
	m_score_label->setTextAlign(CLabel::left);
	m_score_label->setFontColor(sf::Color(0, 119, 170));
	addObject(m_score_label);
	m_score_label->setString("Score: 0");


	CLabel* lives_label = new CLabel("Lives:");
	lives_label->setBounds(770, 100, 140, 30);
	lives_label->setFontName(*CPacManGame::instance()->fontManager().get("score_font"));
	lives_label->setFontSize(32);
	lives_label->setTextAlign(CLabel::left);
	lives_label->setFontColor(sf::Color(0, 119, 170));
	addObject(lives_label);

	m_life_bar = new CLifeBar({ 800,150 });
	addObject(m_life_bar);

	m_dots_label = new CLabel();
	m_dots_label->setBounds(770, 250, 140, 30);
	m_dots_label->setFontName(*CPacManGame::instance()->fontManager().get("score_font"));
	m_dots_label->setFontSize(28);
	m_dots_label->setTextAlign(CLabel::left);
	m_dots_label->setFontColor(sf::Color(0, 119, 170));
	addObject(m_dots_label);
	m_dots_label->setString("Dots:" + toString(m_dots->amount()) + "/" + toString(m_dots->maxDots()));
}

void CPacManGameScene::reset()
{
	resetScore();
	m_lives = 3;
	
	m_inky_unlock = false;
	m_clyde_unlock = false;

	m_dots->reset();

	for (auto& pill : m_pills)
	{
		pill->show();
		pill->enable();
	}

	m_score_label->setString("Score: 0");
	m_lives = 3;
	m_life_bar->setValue(3);

	spawnGhosts();
	spawnPacman();
}

CPacManGameScene::~CPacManGameScene()
{
	CPacManGame::instance()->eventManager().unsubcribe(this);
}

void CPacManGameScene::goToMainMenu()
{
	CPacManGameScene* game_scene = CPacManGame::instance()->getRootObject()->findObjectByName<CPacManGameScene>("game_scene");
	CMainMenuScene* menu_scene =   CPacManGame::instance()->getRootObject()->findObjectByName<CMainMenuScene>("menu_scene");
	
	enableActors(true);
	game_scene->turnOff();
	menu_scene->reset();
	menu_scene->turnOn();
}

void CPacManGameScene::update(int delta_time)
 {
	CGameObject::update(delta_time);

	 if (!isEnabled() || !m_pacman->isEnabled())
		 return;

	 	 
	 // DOT EAT PROCESSING 
	 Vector player_claster = m_walls->toMapCoordinates(m_pacman->getPosition());
	 if (m_dots->eat(player_claster.x, player_claster.y))
	 {
		 CPacManGame::instance()->playSound("eat_dot");
		 addScore(1);
		 m_dots_label->setString("Dots:" + toString(m_dots->amount()) + "/" + toString(m_dots->maxDots()));
		 
		 if (m_dots->amount() == 0)
		 {
			 enableActors(false);
			 m_big_text->setString("You are win!");
			 m_wave_timer->clear();
			 m_wave_timer->add(sf::seconds(3), std::bind(&CPacManGameScene::goToMainMenu, this));
			 return;
		 }
		 if (m_dots->amount() == m_dots->maxDots() - 70 || m_dots->amount() == m_dots->maxDots() - 170)
		 {
			 m_fruit->enable();
			 m_fruit->show();
			 m_fruit_timer->clear();
			 m_fruit->setFlashed(true);
			 m_fruit_timer->add(sf::seconds(3),  [this]() {m_fruit->setFlashed(false); });
			 m_fruit_timer->add(sf::seconds(20), [this]() {m_fruit->disable(); m_fruit->hide(); });
			 m_fruit_timer->add(sf::seconds(17), [this]() {m_fruit->setFlashed(true); });
		 }
		 if (m_dots->amount() == m_dots->maxDots() - 30)
		 {
			m_inky_unlock = true;
			CGhost* inky = findObjectByName<CGhost>("Inky");
			if (inky->currentStateType() == CGhostState::Type::InHouse)
			  setGhostState(inky, GhostStates::borning);
		 }

		 if (m_dots->amount() == m_dots->maxDots() - 100)
		 {
			 m_clyde_unlock = true;
			 CGhost* clyde = findObjectByName<CGhost>("Clyde");
			 if (clyde->currentStateType() == CGhostState::Type::InHouse)
				 setGhostState(clyde, GhostStates::borning);
		 }

	 }


	 // FRUIT EAT PROCESSING 
	 if (m_walls->toMapCoordinates(m_fruit->getPosition()) == player_claster && m_fruit->isEnabled())
	 {
		 CPacManGame::instance()->playSound("ghost_eaten");
		 m_fruit->disable();
		 m_fruit->hide();
		 m_fruit_timer->clear();
		 m_flow_text->splash(m_pacman->getPosition(), "+1000");
		 addScore(1000);
	 }

	 //  MONSTER EAT PACMAN PROCESING      
	 for (auto& obj : m_ghosts)
	 {
		 Vector monster_claster = m_walls->toMapCoordinates(obj->getPosition());
		 if (monster_claster == player_claster && obj->currentStateType() != CGhostState::Frightened && obj->currentStateType() != CGhostState::Soul)
		 {
			 --m_lives;
			 if (m_lives > 0)
			 {
				 m_life_bar->setValue(m_lives);
				 CPacManGame::instance()->playSound("life_lost");
				 enableActors(false);
				 m_wave_timer->clear();
				 m_wave_timer->add(sf::seconds(3), [this]() {  spawnGhosts();  spawnPacman(); });
				 return;
			 }
			 else
			 {
				 enableActors(false);
				 CPacManGame::instance()->playSound("life_lost");
				 m_big_text->setString("Game over");
				 m_wave_timer->clear();
				 m_wave_timer->add(sf::seconds(3), std::bind(&CPacManGameScene::goToMainMenu, this));
			 }
			 break;
		 }
	 }

	 //  PACMAN EAT MONSTER PROCESING       
	 for (auto& obj : m_ghosts)
	 {
		 Vector monster_claster = m_walls->toMapCoordinates(obj->getPosition());
		 if (monster_claster == player_claster && obj->currentStateType() == CGhostState::Frightened)
		 {
			 CPacManGame::instance()->playSound("ghost_eaten");
			 m_flow_text->splash(m_pacman->getPosition(), "+200");
			 addScore(200);
			 setGhostState(obj,GhostStates::souls);
		 }
	 }

	 //  MONSTER BORN PROCESING       
	 for (auto& obj : m_ghosts)
	 {
		 if (obj->currentStateType() == CGhostState::Soul && !obj->isMoving())
		 {
			 obj->setState(m_ghost_states[GhostStates::in_ghost_house][obj->getName()]);
			 m_born_timer->add(sf::seconds(5), [this, obj]() {   setGhostState(obj, GhostStates::borning);	 CPacManGame::instance()->playSound("ghost_regenerate"); });
		 }

		 if (obj->currentStateType() == CGhostState::Borning && !obj->isMoving())
			 setGhostState(obj, GhostStates::scatter);
	 }

	 //PACMAN EAT PILL PROCESSING
	 CGameObject* pill = NULL;
	 foreachObject([&pill, this](CGameObject* object) {
		 if (object->isEnabled() && object->getName() == "Pill" && m_walls->toMapCoordinates(m_pacman->getPosition()) == m_walls->toMapCoordinates(object->getPosition()))
		 {
			 pill = object;
			 m_wave_timer->disable();
			 setGhostsToFrightenedState();
			 m_pill_timer->clear();
			 m_pill_timer->add(sf::seconds(7),  [this]() { CFrightenedState::setFlashed(true); });
			 m_pill_timer->add(sf::seconds(10), [this]() { m_wave_timer->enable();
			 if (getGhostsGlobalState() == GhostStates::scatter)
				 setGhostsToScatterState();
			 else if (getGhostsGlobalState() == GhostStates::chase)
				 setGhostsToChaseState(); });
		 }
	 });

	 if (pill)
	 {
		 pill->disable();
		 pill->hide();
	 }
 }

 void CPacManGameScene::events(const sf::Event& event)
 {
	 if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Num1)
		isEnabled()?disable():enable();
 }

 void CPacManGameScene::setGhostsState(GhostStates state)
 {
	 for (auto& ghost : m_ghosts)
		 ghost->setState(m_ghost_states[state][ghost->getName()]);
/*
	 if (state == GhostStates::borning)
		 std::cout << "set boring state";
	 else if (state == GhostStates::in_ghost_house)
		 std::cout << "set in_ghost_house state";
	 else if (state == GhostStates::scatter)
		 std::cout << "set scatter state";
	 else if (state == GhostStates::chase)
		 std::cout << "set chase state";
*/
 }

 void CPacManGameScene::setGhostState(CGhost* ghost, GhostStates state)
 {
	 ghost->setState(m_ghost_states[state][ghost->getName()]);
 }


 void CPacManGameScene::spawnGhosts()
 {
	 setGhostsState(GhostStates::in_ghost_house);
	 m_ghosts_global_state = GhostStates::scatter;
	 m_born_timer->clear();
	 m_fruit->disable(); 
	 m_fruit->hide();
	 m_fruit_timer->clear();
	 m_born_timer->clear();

	 enableActors(false);
	 m_big_text->setString("Get Ready!");
	 CPacManGame::instance()->playSound("beginning");
	 
	 int t = 0;
	 m_wave_timer->clear();
	 m_wave_timer->add(sf::seconds(t += 2), [this]()  { m_big_text->setString(""); enableActors(true); });
	 m_wave_timer->add(sf::seconds(t += 4), [this]() 
	     {
		    setGhostState(findObjectByName<CGhost>("Binky"), GhostStates::borning);
			setGhostState(findObjectByName<CGhost>("Pinky"), GhostStates::borning);
			if (m_inky_unlock)
			 setGhostState(findObjectByName<CGhost>("Inky"),  GhostStates::borning);
			if (m_clyde_unlock)
			 setGhostState(findObjectByName<CGhost>("Clyde"), GhostStates::borning);
	     });
	 m_wave_timer->add(sf::seconds(t += 7), [this]()  { setGhostsToChaseState(); });
	 m_wave_timer->add(sf::seconds(t += 20),[this]()  { setGhostsToScatterState(); });
	 m_wave_timer->add(sf::seconds(t += 7), [this]()  { setGhostsToChaseState(); });
	 m_wave_timer->add(sf::seconds(t += 20),[this]()  { setGhostsToScatterState(); });
	 m_wave_timer->add(sf::seconds(t += 5), [this]()  { setGhostsToChaseState(); });
	 m_wave_timer->add(sf::seconds(t += 20),[this]()  { setGhostsToScatterState(); });
	 m_wave_timer->add(sf::seconds(t += 5), [this]()  { setGhostsToChaseState(); });
 }

 void CPacManGameScene::spawnPacman()
 {
	 m_pacman->spawn(m_walls->toPixelCoordinates(m_pacman_spawn_position));
 }
 
 bool isChaseState(CGhostState::Type state_type)
 {
	 if (state_type == CGhostState::Type::BinkyChase ||
		 state_type == CGhostState::Type::InkyChase ||
		 state_type == CGhostState::Type::PinkyChase ||
		 state_type == CGhostState::Type::ClydeChase)
		 return true;
	 return false;
 }
 void CPacManGameScene::setGhostsToFrightenedState()
 {
	 CPacManGame::instance()->playSound("ghosts_frightened");

	 m_ghosts_global_state = GhostStates::frightened;
	 std::cout << "frighten mode" << std::endl;
	 for (auto& obj : m_ghosts)
		 if (obj->currentStateType() == CGhostState::Type::Scatter || 
			 isChaseState(obj->currentStateType()) ||
			 obj->currentStateType() == CGhostState::Type::Frightened)
			 setGhostState(obj, GhostStates::frightened);
 }

 void CPacManGameScene::setGhostsToScatterState()
 {
	 m_ghosts_global_state = GhostStates::scatter;
	 std::cout << "scatter mode" << std::endl;
	 for (auto& obj : m_ghosts)
		 if (isChaseState(obj->currentStateType()) || obj->currentStateType() == CGhostState::Type::Frightened)
			 setGhostState(obj, GhostStates::scatter);

 }
 
 void CPacManGameScene::setGhostsToChaseState()
 {
	 m_ghosts_global_state = GhostStates::chase;
	 std::cout << "chase mode" << std::endl;
	 for (auto& obj : m_ghosts)
		 if (obj->currentStateType() == CGhostState::Type::Scatter || obj->currentStateType() == CGhostState::Type::Frightened)
			 setGhostState(obj, GhostStates::chase);
 }

 CPacManGameScene::GhostStates CPacManGameScene::getGhostsGlobalState() const
 {
	 return m_ghosts_global_state;
 }

//----------------------------------------------------------------------------------------------

     CMainMenuScene::CMainMenuScene()
     {
		 CPacManGame::instance()->eventManager().subscribe(this);

		 m_logo = new CLabel();
		 m_logo->setSprite(sf::Sprite(*CPacManGame::instance()->textureManager().get("texture"), sf::IntRect(5, 148, 240, 50)));
		 m_logo->setBounds(240, 120, 480, 100);
		 addObject(m_logo);

		 m_ghost_name = new CLabel();
		 m_ghost_name->setBounds(300, 280, 360, 75);
		 m_ghost_name->setFontSize(42);
		 m_ghost_name->setFontColor(sf::Color::Black);
		 m_ghost_name->setFontName(*CPacManGame::instance()->fontManager().get("menu_font"));
		 addObject(m_ghost_name);
		 CTimer* timer;
		 addObject(new CPacman());
		 addObject(new CPill());
		 addObject(timer = new CTimer());


		 static const char* captions[] = { "New game","Controls", "Exit" };
		 for (int i = 0; i < 3; ++i)
		 {
			 CButton* button = new CButton();
			 button->setBounds(400, 400 + i*70, 170, 40);
			 button->setString(captions[i]);
			 button->setFontName(*CPacManGame::instance()->fontManager().get("menu_font"));
			 button->setFontColor(sf::Color::Black);

			 button->setFontSize(26);
			 addObject(button);
			 m_buttons[i] = button;
		 }
		 m_buttons[2]->onClick([]() { exit(0); });

		 auto root = CPacManGame::instance()->getRootObject();

		 m_buttons[0]->onClick([timer, root]() {root->findObjectByName("game_scene")->turnOn();
		                                        root->findObjectByName<CPacManGameScene>("game_scene")->reset();
											    root->findObjectByName("menu_scene")->turnOff();
										        timer->clear();
		 });

		 m_buttons[0]->setFocus(true);

	
		 for (int i = 0; i < 6; ++i)
			 m_ghost_states[i] = new CToyState((CToyState::State)i);

		 static const char* names[] = { "Binky","Pinky","Inky","Clyde" };
		 for (int i = 0; i < 4; ++i)
			 addObject(m_ghosts[i] = new CGhost(names[i], NULL, NULL));
			

	

		 reset();
	 }
	 
	 CMainMenuScene::~CMainMenuScene()
	 {
		 CPacManGame::instance()->eventManager().unsubcribe(this);
	 }

	 void CMainMenuScene::reset()
	 {
		 m_ghost_name->setString("");
		 CTimer* timer = findObjectByName<CTimer>("Timer");


		 auto pacman = findObjectByName<CPacman>("Player");
 
 		 pacman->setMovingPath({ { -50,300 },{ 650,300 },{ -50,300 } });
	 
		 static const int ghosts_amount = 4;

		 for (int i = 0; i < ghosts_amount; ++i)
		 {
			 m_ghosts[i]->setState(m_ghost_states[i]);
	 		 m_ghosts[i]->setMovingPath({ { -130 - i * 80, 300 }, { 550 - i * 80,300 } });
		 }

		 CPill* pill = findObjectByName<CPill>("Pill");
		 pill->setPosition(650, 300);
		 
		 timer->add(sf::seconds(4.6), [this, pill]()
		 {  
			pill->setPosition({ -100, 300 });   
		    for (int i = 0; i < ghosts_amount; ++i)
		    {
			 m_ghosts[i]->setState(m_ghost_states[(int)CToyState::State::Frightened]);
 			 m_ghosts[i]->setMovingPath({ { 550 - i * 80,300 } ,{  -130 - i * 80 , 300 } });
		    }
		 });

		 for (int i = 0; i < ghosts_amount; ++i)
			 timer->add(sf::seconds(6 + i), [this, i]() {m_ghosts[i]->setState(m_ghost_states[(int)CToyState::State::Soul]);  });


		 timer->add(sf::seconds(12.5), [this] {
			 for (int i = 0; i < ghosts_amount; ++i)
				 m_ghosts[i]->setState(m_ghost_states[i]); });


		 for (int i = 0; i < ghosts_amount; ++i)
		 {
			 float dt = i * 7;
 			 timer->add(sf::seconds(12.5 + dt), [this,i] { m_ghosts[i]->setSpeed(0.4); m_ghosts[i]->setMovingPath({ { -50,300 },{ 650,300 } });  });
			 timer->add(sf::seconds(14.3 + dt), [this,i] { m_ghost_name->setString(m_ghosts[i]->getName()); m_ghost_name->setFontColor(m_ghosts[i]->color()); });
 			 timer->add(sf::seconds(16.5 + dt), [this,i] { m_ghost_name->setString(""); m_ghosts[i]->setMovingPath({ { 650,300 }, { 1100,300 } }); });
		 }


		 timer->add(sf::seconds(40), [this, timer] {    reset(); }); //repeat

		 }
 
	 void CMainMenuScene::events(const sf::Event& event)
	 {
		 if (!isEnabled())
			 return;

		 if (event.type == sf::Event::KeyPressed)
		 {
			 if (event.key.code == sf::Keyboard::Down)
			 {
				 if (m_buttons[0]->hasFocus())
				 {
					 m_buttons[0]->setFocus(false);
					 m_buttons[1]->setFocus(true);
				 } else
				 if (m_buttons[1]->hasFocus())
				 {
					 m_buttons[1]->setFocus(false);
					 m_buttons[2]->setFocus(true);
				 }
			 }
			 else if (event.key.code == sf::Keyboard::Up)
				 {
					 if (m_buttons[1]->hasFocus())
					 {
						 m_buttons[1]->setFocus(false);
						 m_buttons[0]->setFocus(true);
					 }
					 else if (m_buttons[2]->hasFocus())
					{
							 m_buttons[2]->setFocus(false);
							 m_buttons[1]->setFocus(true);
					}
				 }
			 else if (event.key.code == sf::Keyboard::Space || event.key.code == sf::Keyboard::Return)
			 {
				 for(auto& but : m_buttons)
					 if (but->hasFocus())
					 {
						 but->click();
						 break;
					 }
			 }

			 }
	 }

	
//---------------------------------------------------------------------------------------------- 
CPacManGame* CPacManGame::s_instance = NULL;
	

CPacManGame* CPacManGame::instance()
{
	if (s_instance == NULL)
		s_instance = new CPacManGame();
	return s_instance;
}

CPacManGame::CPacManGame() : CGame("PacMan", {1000,850})
 {
	textureManager().loadFromFile("texture", "res/sprites.png");
	fontManager().loadFromFile("arial", "C:/Windows/Fonts/Calibri.ttf");
	fontManager().loadFromFile("menu_font", "res/menu_font.ttf");
	fontManager().loadFromFile("main_font", "res/main_font.ttf");
	fontManager().loadFromFile("score_font", "res/score_font.ttf");

	soundManager().loadFromFile("beginning", "res/begininng_sound.wav");
	soundManager().loadFromFile("eat_dot", "res/eat_dot_sound.wav");
	soundManager().loadFromFile("ghosts_frightened", "res/ghosts_frightened_sound.wav");
	soundManager().loadFromFile("ghost_eaten", "res/ghost_eaten_sound.wav");
	soundManager().loadFromFile("life_lost", "res/life_lost_sound.wav");
	soundManager().loadFromFile("ghost_regenerate", "res/ghost_regenerate_sound.wav");

	textureManager().get("texture")->setSmooth(true);
 }


CPacManGame::~CPacManGame()
{

}

void CPacManGame::init()
{
	m_game_scene = new CPacManGameScene();
	m_main_menu_scene = new CMainMenuScene();
	m_game_scene->setName("game_scene");
	m_main_menu_scene->setName("menu_scene");
	m_game_scene->turnOff();
	getRootObject()->addObject(m_game_scene);
	getRootObject()->addObject(m_main_menu_scene);
	setClearColor(sf::Color::White);
}
	
//---------------------------------------------------------------------------------------------------------

CButton::CButton()
{
	CPacManGame::instance()->eventManager().subscribe(this);
	m_focus = false;
	setOutlineColor(sf::Color(200, 200, 220));
	setOutlineThickness(2);
	setName("Button");
}

CButton::~CButton()
{
	CPacManGame::instance()->eventManager().unsubcribe(this);
}

void CButton::update(int delta_time)
{

}

void CButton::click()
{
	if (m_call_back)
		m_call_back();
}

void CButton::setFocus(bool value)
{
	m_focus = value;
	setOutlineThickness(m_focus?5:2);
}

bool CButton::hasFocus() const
{
	return m_focus;
}

void CButton::draw(sf::RenderWindow* window)
{
	CLabel::draw(window);
		
}

void CButton::onClick(const std::function<void()>& call_back)
{
	m_call_back = call_back;
}

void CButton::onMouseEnter()
{
	setFillColor(sf::Color(200,200,255));
}

void CButton::onMouseLeave()
{
	setFillColor(sf::Color::White);
}

void CButton::events(const sf::Event& event) 
{
	if (event.type == sf::Event::MouseMoved)	
		if (m_is_on_cursor ^ contains(Vector( event.mouseMove.x, event.mouseMove.y )))
			(m_is_on_cursor = !m_is_on_cursor) ? onMouseEnter() : onMouseLeave();

	if (event.type == sf::Event::MouseButtonPressed && m_is_on_cursor)
		if (m_call_back)
			m_call_back();
}
//---------------------------------------------------------------------------------------------------------

CLifeBar::CLifeBar(const Vector& pos)
{
	setPosition(pos);
	sf::Texture* texture = CPacManGame::instance()->textureManager().get("texture");
	m_sprite.setTexture(*texture);
	m_sprite.setTextureRect({ 48,32,48,48 });
}

void CLifeBar::draw(sf::RenderWindow* window) 
{
	for (int i = 0; i < m_value; ++i)
	{
		m_sprite.setPosition( getPosition() + Vector(50, 0)*i) ;
		window->draw(m_sprite);
	}

}

void CLifeBar::setValue(int value)
{
	m_value = value;
}

//------------------------------------------------------------------------------------------------
CPacman::CPacman(CWalls* walls)
{
	setName("Pacman");
	m_walls = walls;
	init();

}

CPacman::CPacman()
{
	m_walls = NULL;
	init();
}

void CPacman::init()
{
	setName("Player");
	setDirection(Vector::right);
	sf::Texture* texture = CPacManGame::instance()->textureManager().get("texture");
	m_animator.create("right", *texture, { 0,32 }, {48,48},4,1, 0.03, AnimType::forward_backward_cycle);
	m_animator.create("left", *texture, { 48,32 }, { -48,48 }, 4, 1, 0.03, AnimType::forward_backward_cycle);
	m_animator.create("down", *texture, { 0,32 }, { 48,48 }, 4, 1, 0.03, AnimType::forward_backward_cycle);
	m_animator.create("up", *texture, { 0,32 }, { 48,48 }, 4, 1, 0.03, AnimType::forward_backward_cycle);
	m_animator.get("up")->setRotation(270);
	m_animator.get("down")->setRotation(90);
	m_waypoint_system = new WaypointSystem();
	addObject(m_waypoint_system);
}	


Rect CPacman::getBounds() const  
{
	return Rect(getPosition() + Vector(3,3), Vector(48, 48) - Vector(27,27));
}
	
CPacman::~CPacman()
{

}

void CPacman::update(int delta_time)
{
	CGameObject::update(delta_time);

	if (getDirection() == Vector::right)      m_animator.play("right");
	else if (getDirection() == Vector::left)  m_animator.play("left");
	else if (getDirection() == Vector::up)    m_animator.play("up");
	else if (getDirection() == Vector::down)  m_animator.play("down");
	m_animator.update(delta_time);

	if (delta_time == 0 || !m_walls)
		return;

	//Controller component for player
	static std::map<sf::Keyboard::Key,Vector> keys = {{sf::Keyboard::Up,  Vector::up},
									    {sf::Keyboard::Down, Vector::down},
									    {sf::Keyboard::Right,Vector::right},
									    {sf::Keyboard::Left, Vector::left} };
			
	Vector input_direction;
	for (auto& key : keys)
		if (sf::Keyboard::isKeyPressed(key.first))
		{
			input_direction = key.second;
			break;
		}
	            
			
	Vector next_cell = m_walls->toMapCoordinates(getPosition()) + input_direction;
	bool can_turn = !(!m_walls->inBounds(next_cell) || m_walls->getMapCell(next_cell) != EMapBrickTypes::empty);

	if (input_direction != Vector::zero && input_direction != getDirection() && can_turn)
	{
		Vector player_cell = m_walls->toMapCoordinates(getPosition());
		Vector finish_cell = m_walls->getMap()->traceLine(player_cell, input_direction, EMapBrickTypes::empty);
		m_waypoint_system->addPath(m_walls->toPixelCoordinates({ player_cell,finish_cell }), NORMAL_SPEED);
	}

	Vector player_pos = m_walls->toMapCoordinates(getPosition());

	//Teleports
	if ((getDirection() == Vector::left) && getPosition().x < 5)
	{
		setPosition(m_walls->toPixelCoordinates({ m_walls->getMap()->width() - 1.f, player_pos.y }));
		Vector player_cell = m_walls->toMapCoordinates(getPosition());
		Vector finish_cell = m_walls->getMap()->traceLine(player_cell, Vector::left, EMapBrickTypes::empty);
		m_waypoint_system->addPath( m_walls->toPixelCoordinates({ player_cell,finish_cell }), NORMAL_SPEED);
	}

	if ((getDirection() == Vector::right) && getPosition().x > (m_walls->size().x - 30))
	{
		setPosition(m_walls->toPixelCoordinates({ 0.f, player_pos.y }));
		Vector player_cell = m_walls->toMapCoordinates(getPosition());
		Vector finish_cell = m_walls->getMap()->traceLine(player_cell, Vector::right, EMapBrickTypes::empty);
		m_waypoint_system->addPath(m_walls->toPixelCoordinates({ player_cell,finish_cell }), NORMAL_SPEED);
	}
 

}
	 
void CPacman::draw(sf::RenderWindow* window)
{
	m_animator.setPosition(getPosition() + Vector(-10,-10));
	m_animator.draw(window);
}

void CPacman::spawn(const Vector& position)
{
	m_waypoint_system->stop();
	setPosition(position);
	setDirection(Vector::right);
}
void CPacman::setMovingPath(const std::vector<Vector>& path)
{
	m_waypoint_system->addPath(path, NORMAL_SPEED);
}

//-------------------------------------------------------------------------------------------------
		
CPill::CPill()
{
	setName("Pill");
	sf::Texture* texture = CPacManGame::instance()->textureManager().get("texture");
	m_sprite_sheet.load(*texture, { {192,0,32,32} });
	m_rot_offset = { m_sprite_sheet[0].getLocalBounds().width / 2,
						m_sprite_sheet[0].getLocalBounds().height / 2 };
	m_sprite_sheet[0].setOrigin(m_rot_offset.x, m_rot_offset.y);		
}

void CPill::draw(sf::RenderWindow* window)
{
	m_sprite_sheet.setPosition(getPosition() + m_rot_offset);
	window->draw(m_sprite_sheet[0]);
}

void CPill::update(int delta_time)
{
	float angle = m_sprite_sheet[0].getRotation() + delta_time / 10;
	m_sprite_sheet[0].setRotation(angle);
}

//-------------------------------------------------------------------------------------------------
CFruit::CFruit()
{
	setName("Fruit");
	sf::Texture* texture = CPacManGame::instance()->textureManager().get("texture");
	m_sprite_sheet.load(*texture, { { 149,84,40,40 } });
	m_rot_offset = { m_sprite_sheet[0].getLocalBounds().width / 2,
		m_sprite_sheet[0].getLocalBounds().height / 2 };
	m_sprite_sheet[0].setOrigin(m_rot_offset.x, m_rot_offset.y);
}

void CFruit::draw(sf::RenderWindow* window)
{
	m_sprite_sheet.setPosition(getPosition() + m_rot_offset);
	window->draw(m_sprite_sheet[0]);
}

void CFruit::update(int delta_time)
{
	if (m_flashed)
	{
		m_time += delta_time;

		if (int(m_time / 300) % 2)
			show();
		else
			hide();
	}
}

void CFruit::setFlashed(bool value)
{
	m_time = 0;
	m_flashed = value;
	if (!m_flashed)
		show();
}

//-------------------------------------------------------------------------------------------------

CGhost::CGhost(const std::string& name,CGameObject* target, CWalls* walls)
{
	setSpeed(NORMAL_SPEED);
	setName(name);
	m_target = target;
	m_walls = walls;

	sf::Texture* texture = CPacManGame::instance()->textureManager().get("texture");

	m_sprite_sheet.load(*texture, { {0, 80, 48, 40} , //body
			                        {60, 83, 26, 11}, { 60, 94, 26, 11 }, { 60, 105, 26, 11 },{ 60, 116, 26, 11}, //eyes
			                        {10, 120, 27, 5}, //mouth
									{99,84,40,10}, {99,95,40,10}, {99,106,40,10}, {99,117,40,10} }); //legs
			
	for (int i = 1; i < 5; ++i)
	{
		m_sprite_sheet[i].setOrigin(-11, -13);
		m_sprite_sheet[i].setColor(sf::Color::White);
	}

	m_sprite_sheet[5].setOrigin(-10, -28);

	for (int i = 6; i < 10; ++i)
		m_sprite_sheet[i].setOrigin(-4, -34);


	m_waypoint_system = new WaypointSystem();
	addObject(m_waypoint_system);
}


CGhostState::Type CGhost::currentStateType()
{
	return m_ghost_state->type();
}

void CGhost::update(int delta_time)
{
	CGameObject::update(delta_time);
	m_time += delta_time;
	if (m_ghost_state)
		m_ghost_state->update({ delta_time,this,m_walls });
};

void CGhost::drawBody(sf::RenderWindow* window)
{
	m_sprite_sheet.setPosition(getPosition() - Vector(10, 10));
	window->draw(m_sprite_sheet[0]);
	int k = int(m_time / 200) % 4 + 6;	
	window->draw(m_sprite_sheet[k]);
}

void CGhost::drawEyes(sf::RenderWindow* window)
{
	m_sprite_sheet.setPosition(getPosition() - Vector(10, 10));
	float angle = getDirection().angle();
	if (angle < 0) angle += 360;
	if (angle > 360) angle -= 360;
	int index = round(angle / 90) +1;
	window->draw(m_sprite_sheet[index]);
	if (getDirection() == Vector::zero)
		window->draw(m_sprite_sheet[1]);
}

void CGhost::drawMouth(sf::RenderWindow* window)
{
	m_sprite_sheet.setPosition(getPosition() - Vector(10, 10));
	window->draw(m_sprite_sheet[5]);
}

void CGhost::draw(sf::RenderWindow* window)
{
	if (m_ghost_state)
		m_ghost_state->draw({0,this,m_walls }, window);
	else
	{
		drawBody(window);
		drawEyes(window);
	}
}

void CGhost::setTarget(CGameObject* target)
{
	m_target = target;
}

CGameObject* CGhost::target()
{
	return m_target;
}

void CGhost::setColor(sf::Color color)
{
	m_sprite_sheet[0].setColor(color);
	for (int i = 6; i < 10; ++i)
		m_sprite_sheet[i].setColor(color);
}

sf::Color CGhost::color() const
{
	return (const_cast<CGhost*>(this))->m_sprite_sheet[0].getColor();
}

void CGhost::setState(CGhostState* state)
{
	if (m_ghost_state != NULL)
		m_ghost_state->deactivate({ 0,this,m_walls });
	m_ghost_state = state;
	m_ghost_state->activate({ 0,this,m_walls });
}


Vector CGhost::getTargetPos() const
{
	return m_target_pos;
}
 
bool CGhost::isMoving() const
{
	return m_waypoint_system->isMoving();
}

void CGhost::moveToTarget(const Vector& target_pos)
{
	Vector object_cell = m_walls->toMapCoordinates(getPosition());
	Vector target_cell = m_walls->toMapCoordinates(target_pos);

	std::vector<Vector> nodes = m_walls->getMap()->getNeighborNodes(object_cell, EMapBrickTypes::empty);

	Vector next_node;
	float min_dis = 100000;
	for (auto& node : nodes)
	{
		if (m_walls->toMapCoordinates(m_target_pos) == node) //avoid back move
			continue;
		float dis =  (node - target_cell).length();
		if (min_dis >= dis)
		{
			min_dis = dis;
			next_node = node;
		}
	}
	m_target_pos = m_walls->toPixelCoordinates(object_cell) ;

	m_waypoint_system->addPath(m_walls->toPixelCoordinates({ object_cell,next_node }), getSpeed());
}

void CGhost::setMovingPath(const std::vector<Vector>& path)
{
	m_waypoint_system->addPath(path, getSpeed());
}

void CGhost::stop()
{
	m_waypoint_system->stop();
}

void CGhost::setSpeed(float speed)
{
	m_speed = speed;
}

float CGhost::getSpeed() const
{
	return m_speed;
}

//-------------------------------------------------------------------------------------------------

CGhostState::CGhostState(Type type) : m_type(type)
{

}

CGhostState::Type CGhostState::type() const
{
	return m_type;
}

void CGhostState::draw(const CGhostStateContex& contex, sf::RenderWindow* window)
{
	contex.ghost->drawBody(window);
	contex.ghost->drawEyes(window);
}

//-------------------------------------------------------------------------------------------------

CWalls::CWalls(int width, int height)
{
	sf::Texture* texture = CPacManGame::instance()->textureManager().get("texture");
	m_map = new TileMap<EMapBrickTypes>(width, height);
	m_map->clear(EMapBrickTypes::empty);

	texture->setSmooth(true);
	m_sprite_sheet.load(*texture, { { 0, 0, 32, 32 },{ 32, 0, 32, 32 },{ 64, 0, 32, 32 },{ 96, 0, -32, 32 },
	{ 64, 0, 32, 32 },{ 64, 0, 32, 32 },
	{ 96, 0, 32, 32 },{ 96 + 32, 0, -32, 32 },{ 96, 32, 32, -32 },{ 96 + 32, 32, -32, -32 },
	{ 128, 0, 32, 32 },{ 128 + 32, 0, -32, 32 },{ 128, 32, 32, -32 },{ 128 + 32, 32, -32, -32 },
	{ 0, 0, 32, 32 },{ 32, 0, -32, 32 },{ 0, 32, 32, -32 },{ 32, 32, -32, -32 } });

	m_sprite_sheet[4].rotate(90);
	m_sprite_sheet[4].setOrigin(0, 32);
	m_sprite_sheet[5].rotate(-90);
	m_sprite_sheet[5].setOrigin(32, 0);

	m_sprite_sheet.scale(CLASTER_SIZE / 32.f, CLASTER_SIZE / 32.f);
}

void CWalls::update(int delta_time)
{

}

void CWalls::draw(sf::RenderWindow* window)
{
	Vector off_set;

	sf::RectangleShape retangle(sf::Vector2f(m_map->width()*CLASTER_SIZE, m_map->height()*CLASTER_SIZE));
	retangle.setPosition(0, 0);
	retangle.setFillColor(sf::Color(255, 255, 255));
	window->draw(retangle);

	for (int x = 0; x < m_map->width(); ++x)
		for (int y = 0; y < m_map->height(); ++y)
		{
			int num = (int)m_map->getCell(x, y);
			if (num > EMapBrickTypes::brick_min && num < EMapBrickTypes::brick_max)
			{
				auto& sprite = m_sprite_sheet[num - EMapBrickTypes::brick_min];
				sprite.setPosition(off_set + Vector(x, y)*CLASTER_SIZE);
				window->draw(sprite);
			}
		}
}

void CWalls::lining()
{
	auto* map = m_map;

   for (int x = 0; x < map->width(); ++x)
		for (int y = 0; y < map->height(); ++y)
			if (map->getCell(x, y) > EMapBrickTypes::brick_max)
				map->setCell(x, y, EMapBrickTypes::empty);
  			
	for (int x = 0; x < map->width(); ++x)
		for (int y = 0; y < map->height(); ++y)
		{
			if (map->getCell(x, y) >= EMapBrickTypes::in_corn_left_down)
				continue;
								
					if (x < map->width() - 1 && map->getCell(x, y) != EMapBrickTypes::empty && map->getCell(x + 1, y) == EMapBrickTypes::empty)
						map->setCell(x, y, EMapBrickTypes::left);

					else if (x > 0 && map->getCell(x, y) != EMapBrickTypes::empty && map->getCell(x - 1, y) == EMapBrickTypes::empty)
						map->setCell(x, y, EMapBrickTypes::right);

					else if (y < map->height() - 1 && map->getCell(x, y) != EMapBrickTypes::empty && map->getCell(x, y + 1) == EMapBrickTypes::empty)
						map->setCell(x, y, EMapBrickTypes::up);

					else if (y > 0 && map->getCell(x, y) != EMapBrickTypes::empty && map->getCell(x, y - 1) == EMapBrickTypes::empty)
						map->setCell(x, y, EMapBrickTypes::down);

					if (x + 1 < map->width() && y + 1 < map->height() &&
						map->getCell(x, y + 1) == EMapBrickTypes::full &&
						map->getCell(x + 1, y + 1) == EMapBrickTypes::empty &&
						map->getCell(x + 1, y) == EMapBrickTypes::full)
						map->setCell(x, y, EMapBrickTypes::in_corn_right_down);

					if (x - 1 >= 0 && y + 1 < map->height() &&
						map->getCell(x, y + 1) != EMapBrickTypes::empty &&
						map->getCell(x - 1, y + 1) == EMapBrickTypes::empty &&
						map->getCell(x - 1, y) != EMapBrickTypes::empty)
						map->setCell(x, y, EMapBrickTypes::in_corn_left_down);

					if (y - 1 >= 0 && x + 1 < map->width() &&
						map->getCell(x, y - 1) != EMapBrickTypes::empty &&
						map->getCell(x + 1, y - 1) == EMapBrickTypes::empty &&
						map->getCell(x + 1, y) != EMapBrickTypes::empty)
						map->setCell(x, y, EMapBrickTypes::in_corn_right_top);

					if (y - 1 >= 0 && x - 1 >= 0 &&
						map->getCell(x, y - 1) != EMapBrickTypes::empty &&
						map->getCell(x - 1, y - 1) == EMapBrickTypes::empty &&
						map->getCell(x - 1, y) != EMapBrickTypes::empty)
						map->setCell(x, y, EMapBrickTypes::in_corn_left_top);

					if (y == 0 || x == 0 || y == map->height() - 1 || x == map->width() - 1 ||
						map->getCell(x, y) == EMapBrickTypes::empty)
						continue;

					if (map->getCell(x, y + 1) == EMapBrickTypes::empty &&
						map->getCell(x + 1, y + 1) == EMapBrickTypes::empty &&
						map->getCell(x + 1, y) == EMapBrickTypes::empty)
						map->setCell(x, y, EMapBrickTypes::out_corn_left_top);

					if (map->getCell(x, y + 1) == EMapBrickTypes::empty &&
						map->getCell(x - 1, y + 1) == EMapBrickTypes::empty &&
						map->getCell(x - 1, y) == EMapBrickTypes::empty)
						map->setCell(x, y, EMapBrickTypes::out_corn_right_top);

					if (map->getCell(x, y - 1) == EMapBrickTypes::empty &&
						map->getCell(x + 1, y - 1) == EMapBrickTypes::empty &&
						map->getCell(x + 1, y) == EMapBrickTypes::empty)
						map->setCell(x, y, EMapBrickTypes::out_corn_left_down);

					if (map->getCell(x, y - 1) == EMapBrickTypes::empty &&
						map->getCell(x - 1, y - 1) == EMapBrickTypes::empty &&
						map->getCell(x - 1, y) == EMapBrickTypes::empty)
						map->setCell(x, y, EMapBrickTypes::out_corn_right_down);
				}
}

CWalls::~CWalls()
{
	delete m_map;
}
			
Vector CWalls::toMapCoordinates(const Vector& global_pos)
{
    return{ (int)round(global_pos.x / CLASTER_SIZE), (int)round(global_pos.y / CLASTER_SIZE) };
}

Vector CWalls::toPixelCoordinates(const Vector& local_pos)
{
	return local_pos * CLASTER_SIZE;
}

std::vector<Vector> CWalls::toPixelCoordinates(std::vector<Vector>&& path)
{
	for (auto& v : path)
		v *= CLASTER_SIZE;
	return path;
}

EMapBrickTypes CWalls::getMapCell(const Vector& vector) const
{
	return m_map->getCell(vector.x, vector.y);
}

Vector CWalls::alignToMap(const Vector& position) const
{
    return { (int)CLASTER_SIZE * round(position.x / CLASTER_SIZE), CLASTER_SIZE * round(position.y/CLASTER_SIZE) };
}

EMapBrickTypes CWalls::getMapCell(int x, int y) const
{
	return m_map->getCell(x, y);
}

Vector CWalls::size() const
{
	return Vector(m_map->width(), m_map->height())*CLASTER_SIZE;
}

TileMap<EMapBrickTypes>* CWalls::getMap()
{
	return m_map;
}

bool CWalls::inBounds(const Vector& vec) const
{
	return m_map->inBounds(vec);
}

bool CWalls::isCollide(Rect& rect, EMapBrickTypes allowed_cell_type)
{
	Vector lt = rect.leftTop() / CLASTER_SIZE;
	Vector rb = rect.rightBottom() / CLASTER_SIZE;

	for (int x = lt.x; x < rb.x; ++x)
		for (int y = lt.y; y < rb.y; ++y)
			if (m_map->getCell(x, y) != allowed_cell_type)
				if (Rect(x*CLASTER_SIZE, y*CLASTER_SIZE, CLASTER_SIZE, CLASTER_SIZE).isIntersect(rect))
					return true;
	return false;
}
 
//-----------------------------------------------------------------------------

CDots::CDots(CWalls* walls)
{
	m_walls = walls;
	m_dots_map = new  TileMap<bool>(walls->getMap()->width(), walls->getMap()->height());
	m_saved_map = new  TileMap<bool>(walls->getMap()->width(), walls->getMap()->height());

	//fill(walls);
	m_shape.setRadius(4);
	m_shape.setOrigin(sf::Vector2f(3, 3));
	m_shape.setFillColor(sf::Color(0, 162, 232));
	
}

CDots::~CDots()
{
	delete m_dots_map;
}

void CDots::update(int delta_time)
{
			
}

void CDots::draw(sf::RenderWindow* window)
{
	for (int x = 0; x < m_dots_map->width(); ++x)
	for (int y = 0; y < m_dots_map->height(); ++y)
	if (m_dots_map->getCell(x, y) == true)
	{
		m_shape.setPosition(m_walls->toPixelCoordinates(Vector(x+0.5f, y+0.5f)));
		window->draw(m_shape);
	}
}

bool CDots::eat(int x, int y)
{
	bool a = m_dots_map->getCell(x, y);
	if (a)
	{
		m_dots_counter--;
		m_dots_map->setCell(x, y, false);
	}
	return a;
}

void CDots::fill(CWalls* walls)
{
	m_dots_counter = 0;
	for (int x = 0; x < m_dots_map->width(); ++x)
	for (int y = 0; y < m_dots_map->height(); ++y)
		if (walls->getMapCell(x, y) == EMapBrickTypes::dot)
		{
			m_saved_map->setCell(x, y, true);
			m_dots_counter++;
		}
	m_max_dots = m_dots_counter;
}

int CDots::maxDots() const
{
	return m_max_dots;
}  

void CDots::reset()
{
	m_dots_map->operator=(*m_saved_map);
	m_dots_counter = m_max_dots;
}

int CDots::amount() const
{
	return m_dots_counter;
}

void CDots::clearRect(int x1, int y1, int w, int h)
{
	for (int x = x1; x < x1 + w; ++x)
		for (int y = y1; y < y1 + h; ++y)
			if (m_dots_map->getCell(x, y) == true)
			{
				m_dots_map->setCell(x, y, false); 
				m_dots_counter--;
			}
}

//-----------------------------------------------------------------------------

