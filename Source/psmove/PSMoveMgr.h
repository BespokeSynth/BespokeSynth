#pragma once

#include "../OpenFrameworksPort.h"
#include "psmove.h"

struct _PSMove;

#define MAX_NUM_PS_MOVES 7

class PSMoveListener
{
public:
   virtual ~PSMoveListener() {}
   virtual void OnPSMoveButton(int id, string button, int val) = 0;
};

class PSMoveMgr
{
public:
   PSMoveMgr() : mListener(NULL) {}
   
   void Setup();
   void Update();
   void Exit();

   void AddMoves();
   void SetVibration(int id, float amount);
   void SetColor(int id, float r, float g, float b);

   void GetGyros(int id, ofVec3f& gyros);
   void GetAccel(int id, ofVec3f& accel);
   bool IsButtonDown(int id, PSMove_Button button);
   float GetBattery(int id);
   
   void SetListener(PSMoveListener* listener) { mListener = listener; }
   
private:
   _PSMove* SetUpMove(int id);
   void SendButtonMessage(int id, string button, int val);

   _PSMove* mMove[MAX_NUM_PS_MOVES];
   int mButtons[MAX_NUM_PS_MOVES];
   
   PSMoveListener* mListener;
};
