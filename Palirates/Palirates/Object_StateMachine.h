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

	ETC
};

struct Key_State
{
	// true == Key_Down
	// false == Key_Up

	bool forward; // w �Ǵ� ����Ű
	bool back; // s �Ǵ� ����Ű
	bool left; // a �Ǵ� ����Ű
	bool right; // d �Ǵ� ����Ű

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
	Jump,
	Attack_Normal,
	ETC
};


class StateMachine
{
protected:
	State lastState = State::Idle; // ���� ����
	State currentState = State::Idle; // ���� ����

	Key_State key_state;
	XMFLOAT3 pos{ 0.0f, 0.0f, 0.0f };
public:
	bool is_protected = false;

	StateMachine() {
		currentState = State::Idle;
	}

	void start();
	void update(float Elapsed_time);

	void handleEvent(Key_Value key_event);
	void changeState(State newState, Key_Value key_event);

	XMFLOAT3 Get_Pos() { return pos; };
	State  Get_State() { return currentState; };

private:

	void enterState(State state, Key_Value key_event);

	void exitState(State state, Key_Value key_event);

	// �̰ɷ� �ִϸ��̼� ó��
	virtual void doAction(State state, float Elapsed_time) {};

};

