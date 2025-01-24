#include "stdafx.h"
#include "Object_StateMachine.h"


void Key_State::update(Key_Value key_state)
{
    switch (key_state)
    {
    case Key_Value::Forward_Key_Down:
        forward = true;
        break;
    case Key_Value::Forward_Key_Up:
        forward = false;
        break;
    case Key_Value::Back_Key_Down:
        back = true;
        break;
    case Key_Value::Back_Key_Up:
        back = false;
        break;
    case Key_Value::Left_Key_Down:
        left = true;
        break;
    case Key_Value::Left_Key_Up:
        left = false;
        break;
    case Key_Value::Right_Key_Down:
        right = true;
        break;
    case Key_Value::Right_Key_Up:
        right = false;
        break;

    case Key_Value::Jump_Key_Down:
        break;
    case Key_Value::Jump_Key_Up:
        break;

    case Key_Value::None:
    case Key_Value::ETC:
    default:
        break;
    }
}

bool Key_State::check_move()
{
    // ���� �ݴ�Ǵ� Ű�� ���� ���� ������ ��� x
    return (left != right) || (forward != back);
}

//========================================================
// ���� �ӽ�

void StateMachine::start()
{
    enterState(currentState, Key_Value::None);
}

void StateMachine::update(float Elapsed_time)
{
    // ���� ���¿� ���� ���� or �ִϸ��̼��� ����
    doAction(currentState, Elapsed_time);
}

void StateMachine::handleEvent(Key_Value key_event)
{
    // ���� Ű�� ���� ����
    key_state.update(key_event);

    switch (currentState)
    {
    case State::Idle:
    {
        changeState(State::Run, key_event);
    }
    break;

    case State::Run:
    {
        changeState(State::Idle, key_event);
    }
    break;

    case State::Jump:
    {
    }
    break;

    case State::Attack_Normal:
    {
    }
    break;

    default:
        break;
    }
}

void StateMachine::changeState(State newState, Key_Value key_event)
{
    lastState = currentState;
    exitState(currentState, key_event);
    currentState = newState;
    enterState(currentState, key_event);
}

void StateMachine::enterState(State state, Key_Value key_event)
{
    switch (state)
    {
    case State::Idle:
        break;
    case State::Run:
        break;
    case State::Jump:
        break;
    case State::Attack_Normal:
        break;

    default:
        break;
    }
}

void StateMachine::exitState(State state, Key_Value key_event)
{
    switch (state)
    {
    case State::Idle:
        break;
    case State::Run:
        break;
    case State::Jump:
        break;
    case State::Attack_Normal:
        break;
        
    default:
        break;
    }
}
