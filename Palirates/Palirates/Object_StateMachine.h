#pragma once

enum class Key_Value
{
	None,

	Forward_Key_Down,
	Forward_Key_Up,

	Back_Key_Down,
	Back_Key_Up,

	Left_Key_Down,
	Left_Key_Up,

	Right_Key_Down,
	Right_Key_Up,

	Jump_Key_Down,
	Jump_Key_Up,

	Dive_Key_Down,
	Dive_Key_Up,

	ETC
};

struct Key_State
{
	// true == Key_Down
	// false == Key_Up

	bool forward; // w 또는 방향키
	bool back; // s 또는 방향키
	bool left; // a 또는 방향키
	bool right; // d 또는 방향키
	bool dive{ false }; // shift

	Key_State()
	{
		forward = false;		back = false;
		left = false;		right = false;
	}
	void update(Key_Value key_state);

	bool check_move();
};

//===================================================================

enum class State
{
	Idle,
	Run,
	Knock_Down,
	Get_Up,
	Dive,
	Jump,
	Attack_Normal,
	ETC
};

extern std::map<State, std::wstring> stateToStringMap;

class CPlayer;
class StateMachine
{
protected:
	State lastState = State::Idle; // 이전 상태
	State currentState = State::Idle; // 현재 상태
	State nextState = State::Idle; // 다음 상태

	Key_State key_state;
	XMFLOAT3 pos{ 0.0f, 0.0f, 0.0f };


	bool canMove{ true };
	bool moveEnabled{ false };

public:
	bool is_protected = false;

	StateMachine() : m_pOwner(nullptr), currentState(State::Idle) {}  // 기본 생성자 추가

	StateMachine(CPlayer* owner)
		: m_pOwner(owner), currentState(State::Idle) {
	}

	void start();
	void update(float Elapsed_time);

	void handleEvent(UCHAR* pKeysBuffer);
	void changeState(State newState, Key_Value key_event);

	XMFLOAT3 Get_Pos() { return pos; };
	State  Get_State() { return currentState; };
	State  Get_LastState() { return lastState; };

	constexpr int GetStateKey(State state)
	{
		return static_cast<int>(state);
	}

	void SetCaMove(bool b) { canMove = b; }
	void SetMoveEnabled(bool b) { moveEnabled = b; }

private:

	void enterState(State state, Key_Value key_event);

	void exitState(State state, Key_Value key_event);

	// 이걸로 애니메이션 처리
	virtual void doAction(State state, float Elapsed_time) {};


	CPlayer* m_pOwner;

};

