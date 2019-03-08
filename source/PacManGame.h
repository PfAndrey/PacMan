#ifndef PACMANGAME_H
#define PACMANGAME_H

#include <vector>
#include <array>
#include "assert.h"
#include <memory>
#include <iostream>
#include "GameEngine.h"

class CPacman;
class CWalls;
class CDots;
class CGhost;
class CGhostState;
class CPacManGameScene;
class CMainMenuScene;
class CButton;
class CFlowText;
class CLabel;
class CLifeBar;
class CGhostState;
class CPill;
class CFruit;

enum  EMapBrickTypes {
	empty=0, brick_min, full, left, right, up, down,
	out_corn_left_top, out_corn_right_top, out_corn_left_down, out_corn_right_down,
	in_corn_left_top, in_corn_right_top, in_corn_left_down, in_corn_right_down,
	door_lu, door_ru, door_ld, door_rd, brick_max, ghost_spawn, pacman_spawn,pill, fruit, dot
};

class CPacManGame : public CGame
{
private:
	CPacManGame();
	CPacManGameScene* m_game_scene;
	CMainMenuScene* m_main_menu_scene;
	void init() override;
	static CPacManGame* s_instance;
public:
	~CPacManGame();
    static CPacManGame* instance();
 
};

class CPacManGameScene : public CGameObject
{
public:
	CPacManGameScene();
	~CPacManGameScene();
	virtual void update(int delta_time) override;
	virtual void events(const sf::Event& event) override;
	void reset();
private:
	void addScore(int);
	void resetScore();
	void enableActors(bool value);
	void goToMainMenu();
	CFruit* m_fruit;
	Vector m_pacman_spawn_position;
	Vector m_fruit_cell;
	enum Ghosts { Binky, Pinky, Inky, Clyde };
	enum GhostStates { chase, scatter, frightened, souls, borning, in_ghost_house } m_ghosts_global_state;
	std::array<CGhost*, 4> m_ghosts;
	std::array<std::map<std::string, CGhostState*>, 6> m_ghost_states;
	CGhostState* m_ghost_chase_states[4];
	CLabel* m_score_label;
	CLabel* m_dots_label;
	std::vector<CPill*> m_pills;
	int m_score;
	int m_lives;
	CButton* m_big_text;
	CTimer* m_wave_timer, *m_pill_timer, *m_born_timer, *m_fruit_timer;
	CPacman* m_pacman;
	CWalls* m_walls;
	CDots* m_dots;
	sf::Sound m_sound;
	CFlowText* m_flow_text;
	CLifeBar* m_life_bar;
	bool m_inky_unlock;
	bool m_clyde_unlock;
	void setGhostsToFrightenedState();
	void setGhostsToScatterState();
	void setGhostsToChaseState();
	GhostStates getGhostsGlobalState() const;
	void setGhostsState(GhostStates state);
	void setGhostState(CGhost* ghots, GhostStates state);
	void spawnGhosts();
	void spawnPacman();
};

class CMainMenuScene : public CGameObject
{
public: 
	CMainMenuScene();
	~CMainMenuScene();
	void reset();
	virtual void events(const sf::Event& event) override;
private:
	CGhostState* m_ghost_states[6];
	CGhost* m_ghosts[4];
	CButton* m_buttons[3];
	CLabel* m_logo;
	CLabel* m_ghost_name;
	float m_time = 0;
};

class CButton : public CLabel
{
 public:
	 CButton();
	 ~CButton();
	 virtual void update(int delta_time) override;
	 virtual void events(const sf::Event& event) override;
	 void onClick(const std::function<void()>& call_back);
	 void setFocus(bool value);
	 bool hasFocus() const;
	 void click();
protected:
	 void onMouseEnter();
	 void onMouseLeave();
 private:
	 virtual void draw(sf::RenderWindow* window) override;
	 bool m_focus;
	 bool m_is_on_cursor = false;
	 std::function<void()> m_call_back = NULL;
};

class CLifeBar : public CGameObject
{
public:
	CLifeBar(const Vector& pos);
	void draw(sf::RenderWindow* window) override;
	void setValue(int value);
private:
	sf::Sprite m_sprite;
	int m_value = 3;
};

class CWalls : public CGameObject
{
public:
	CWalls(int width, int height);
	~CWalls();
	virtual void update(int delta_time) override;
	virtual void draw(sf::RenderWindow* window) override;
	void lining();
	EMapBrickTypes getMapCell(int x, int y) const;
	EMapBrickTypes getMapCell(const Vector& vector) const;
	Vector size() const;
	TileMap<EMapBrickTypes>* getMap();
	Vector toMapCoordinates(const Vector& global_pos);
	Vector toPixelCoordinates(const Vector& global_pos);
	Vector alignToMap(const Vector& position) const;
	std::vector<Vector> toPixelCoordinates(std::vector<Vector>&& path);
	bool inBounds(const Vector& vec) const;
	bool isCollide(Rect& rect, EMapBrickTypes allowed_cell_type);
private:
	CSpriteSheet m_sprite_sheet;
	TileMap<EMapBrickTypes>* m_map;
	const int CLASTER_SIZE = 27;
};

class CPacman : public CGameObject
{
public:
	CPacman();
	CPacman(CWalls* walls);
	~CPacman();
	virtual void update(int delta_time) override;
	virtual void draw(sf::RenderWindow* window) override;
	virtual Rect getBounds() const override;
	void spawn(const Vector& position);
	void setMovingPath(const std::vector<Vector>& path);
private:
	const float NORMAL_SPEED = 0.15f;
	void init();
	Animator m_animator;
	CWalls* m_walls;
	WaypointSystem* m_waypoint_system = NULL;
};

class CDots : public CGameObject
{
public:
	CDots(CWalls* walls);
	~CDots();
	virtual void update(int delta_time) override;
	virtual void draw(sf::RenderWindow* window) override;
	bool eat(int x, int y);
	void fill(CWalls* walls);
	void reset();
	int amount() const;
	int maxDots() const;
private:
	CWalls* m_walls;
	sf::CircleShape m_shape;
	TileMap<bool>* m_dots_map;
	TileMap<bool>* m_saved_map;
	int m_max_dots;
	int m_dots_counter;
	float m_claster_size;
};

class CPill : public CGameObject
{
public:
	CPill();
	virtual void draw(sf::RenderWindow* window) override;
	virtual void update(int delta_time) override;
private:
	CSpriteSheet m_sprite_sheet;
	Vector m_rot_offset;
};

class CFruit : public CGameObject
{
public:
	CFruit();
	virtual void draw(sf::RenderWindow* window) override;
	virtual void update(int delta_time) override;
	void setFlashed(bool value);
private:
	CSpriteSheet m_sprite_sheet;
	Vector m_rot_offset;
	float m_time;
	bool m_flashed;
};

class CGhost;

struct CGhostStateContex
{
	int delta_time;
	CGhost* ghost;
	CWalls* walls;
};

class CGhostState
{
public:
	enum Type {BinkyChase, PinkyChase, InkyChase, ClydeChase, Frightened, Scatter, Soul, Borning, InHouse, Toy};
	CGhostState(Type type);
	Type type() const;
	virtual void update(const CGhostStateContex& contex) = 0;
	virtual void draw(const CGhostStateContex&, sf::RenderWindow* window);
    virtual void activate(const CGhostStateContex& contex) {};
    virtual void deactivate(const CGhostStateContex& contex) {};
private:
	Type m_type;
};

class CGhost : public CGameObject
{
protected:
	WaypointSystem* m_waypoint_system = NULL;
	CSpriteSheet m_sprite_sheet;
	CGameObject* m_target;
	CWalls* m_walls;
	CGhostState* m_ghost_state = NULL;
	float m_time = 0;
	Vector m_target_pos;
	float m_speed = 0;
public:
	const float NORMAL_SPEED = 0.15f;
	const float SCARRED_SPEED = 0.0725f;
	void setSpeed(float speed);
	float getSpeed() const;
	Vector getTargetPos() const;
	CGhost(const std::string& name, CGameObject* target, CWalls* walls);
	virtual void update(int delta_time) override;
	virtual void draw(sf::RenderWindow* window) override;
	void setTarget(CGameObject* target);
	CGameObject* target();
	void setColor(sf::Color color);
	sf::Color color() const;
	CGhostState::Type currentStateType();
	void setState(CGhostState* state);
	void drawBody(sf::RenderWindow* window);
	void drawEyes(sf::RenderWindow* window);
	void drawMouth(sf::RenderWindow* window);
	bool isMoving() const;
	void moveToTarget(const Vector& target_pos);
	void setMovingPath(const std::vector<Vector>& path);
	void stop();
};

#endif

