#include "PSMoveMgr.h"
#include <chrono>
#include <thread>

//--------------------------------------------------------------
void PSMoveMgr::Setup()
{
   for (int i = 0; i < MAX_NUM_PS_MOVES; ++i)
      mMove[i] = NULL;

   AddMoves();
}

void PSMoveMgr::AddMoves()
{
   for (int i = 0; i < MAX_NUM_PS_MOVES; ++i)
   {
      if (mMove[i] == NULL)
         mMove[i] = SetUpMove(i);
   }
}

_PSMove* PSMoveMgr::SetUpMove(int id)
{
   _PSMove* move = NULL;
   int lastBTController = -1;
   for (int i = 0; i <= id; ++i)
   {
      enum PSMove_Connection_Type ctype = Conn_Unknown; //valid: Conn_Unknown, Conn_USB, Conn_Bluetooth
      for (int j = lastBTController + 1; ctype != Conn_Bluetooth; ++j) //find the next bluetooth one
      {
         move = psmove_connect_by_id(j);
         if (move == NULL)
         {
            return NULL;
         }
         ctype = psmove_connection_type(move);
         lastBTController = j;
      }
   }

   for (int i = 0; i < 10; i++)
   {
      psmove_set_leds(move, 0, 255 * (i % 3 == 0), 0);
      //psmove_set_rumble(move, 255*(i%2));
      psmove_update_leds(move);
      std::this_thread::sleep_for(std::chrono::milliseconds{ 10 * (i % 10) });
   }

   for (int i = 250; i >= 0; i -= 5)
   {
      psmove_set_leds(move, i, i, 0);
      psmove_set_rumble(move, 0);
      psmove_update_leds(move);
   }

   psmove_set_leds(move, 0, 0, 0);
   psmove_set_rumble(move, 0);
   psmove_update_leds(move);

   mButtons[id] = 0;

   printf("Connected Move #%d\n", id);

   return move;
}

void PSMoveMgr::GetGyros(int id, ofVec3f& gyros)
{
   if (mMove[id])
   {
      int x, y, z;
      psmove_get_gyroscope(mMove[id], &x, &y, &z);
      gyros.x = x;
      gyros.y = y;
      gyros.z = z;
   }
}

void PSMoveMgr::GetAccel(int id, ofVec3f& accel)
{
   if (mMove[id])
   {
      int x, y, z;
      psmove_get_accelerometer(mMove[id], &x, &y, &z);
      accel.x = x;
      accel.y = y;
      accel.z = z;
   }
}

bool PSMoveMgr::IsButtonDown(int id, PSMove_Button button)
{
   if (mMove[id])
   {
      int currentButtons = psmove_get_buttons(mMove[id]);
      return currentButtons & button;
   }
   return false;
}

float PSMoveMgr::GetBattery(int id)
{
   if (mMove[id])
   {
      return psmove_get_battery(mMove[id]) / 5.0f;
   }
   return 0;
}

//--------------------------------------------------------------
void PSMoveMgr::Update()
{
   for (int i = 0; i < MAX_NUM_PS_MOVES; ++i)
   {
      if (mMove[i])
      {
         _PSMove* move = mMove[i];
         int res = 1;
         while (res)
            res = psmove_poll(move); //poll until there's no new data, so we have the freshest

         int currentButtons = psmove_get_buttons(move);
         int change = currentButtons ^ mButtons[i];
         int pressed = currentButtons & change;
         int released = mButtons[i] & change;
         mButtons[i] = currentButtons;
         if (Btn_TRIANGLE & pressed)
            SendButtonMessage(i, "triangle", 1);
         if (Btn_TRIANGLE & released)
            SendButtonMessage(i, "triangle", 0);
         if (Btn_CIRCLE & pressed)
            SendButtonMessage(i, "circle", 1);
         if (Btn_CIRCLE & released)
            SendButtonMessage(i, "circle", 0);
         if (Btn_CROSS & pressed)
            SendButtonMessage(i, "cross", 1);
         if (Btn_CROSS & released)
            SendButtonMessage(i, "cross", 0);
         if (Btn_SQUARE & pressed)
            SendButtonMessage(i, "square", 1);
         if (Btn_SQUARE & released)
            SendButtonMessage(i, "square", 0);
         if (Btn_SELECT & pressed)
            SendButtonMessage(i, "select", 1);
         if (Btn_SELECT & released)
            SendButtonMessage(i, "select", 0);
         if (Btn_START & pressed)
            SendButtonMessage(i, "start", 1);
         if (Btn_START & released)
            SendButtonMessage(i, "start", 0);
         if (Btn_PS & pressed)
            SendButtonMessage(i, "ps", 1);
         if (Btn_PS & released)
            SendButtonMessage(i, "ps", 0);
         if (Btn_MOVE & pressed)
            SendButtonMessage(i, "move", 1);
         if (Btn_MOVE & released)
            SendButtonMessage(i, "move", 0);
         if (Btn_T & pressed)
            SendButtonMessage(i, "trigger", 1);
         if (Btn_T & released)
            SendButtonMessage(i, "trigger", 0);

         psmove_update_leds(move);
      }
   }
}

void PSMoveMgr::SendButtonMessage(int id, std::string button, int val)
{
   std::string address = "/" + ofToString(id) + "/button/" + button;
   //TheMessenger->SendIntMessage(address,val);

   if (mListener)
      mListener->OnPSMoveButton(id, button, val);
}

void PSMoveMgr::Exit()
{
   for (int i = 0; i < MAX_NUM_PS_MOVES; ++i)
   {
      if (mMove[i])
      {
         psmove_disconnect(mMove[i]);
      }
   }
}

void PSMoveMgr::SetVibration(int id, float amount)
{
   if (mMove[id] == NULL)
      return;

   psmove_set_rumble(mMove[id], int(amount * 255));
}

void PSMoveMgr::SetColor(int id, float r, float g, float b)
{
   if (mMove[id] == NULL)
      return;

   psmove_set_leds(mMove[id], int(r * 255), int(g * 255), int(b * 255));
}
