#ifndef virtualkeys_HPP_INCLUDED_
#define virtualkeys_HPP_INCLUDED_
// virtualkeys.hpp
/*
 * Copyright notice:
 * Copyright (c) 2010-2015 3Dconnexion. All rights reserved. 
 * 
 * This file and source code are an integral part of the "3Dconnexion
 * Software Developer Kit", including all accompanying documentation,
 * and is protected by intellectual property laws. All use of the
 * 3Dconnexion Software Developer Kit is subject to the License
 * Agreement found in the "LicenseAgreementSDK.txt" file.
 * All rights not expressly granted by 3Dconnexion are reserved.
 */
///////////////////////////////////////////////////////////////////////////////////
// History
//
// $Id: virtualkeys.hpp 11091 2015-01-09 11:02:45Z jwick $
//
// 22.10.12 MSB Fix: Number of keys returned for SpacePilot Pro is incorrect (27)
// 19.06.12 MSB Added SM Touch and Generic 2 Button SM
// 09.03.12 MSB Fix VirtualKeyToHid not correctly converting SpaceMousePro buttons >V3DK_3
// 03.02.12 MSB Changed the labels of the "programmable buttons" back to "1" etc
// 22.09.11 MSB Added V3DK_USER above which the users may add their own virtual keys
// 02.08.11 MSB Added pid for Viking
//              Added virtualkey / hid definition for Viking
//              Added member to the tag_VirtualKeys struct for the number of
//              buttons on the device
//              Added methods to retrieve the number of buttons on a device
//              Added methods to map the hid buttons numbers to a consecutive
//              sequence (and back again)
// 11.03.11 MSB Fix incorrect label for V3DK_ROLL_CCW
// 09.03.11 MSB Added methods to return the labels of the keys on the device
// 19.10.10 MSB Moved the standard 3dmouse virtual buttons to the s3dm namespace
// 28.09.10 MSB Added spin and tilt buttons
//              Added structure to convert virtual key number to string identifier
// 04.12.09 MSB Fix spelling mistake 'panzoon'
//

#define _TRACE_VIRTUAL_KEYS 0


#if !defined(numberof)
#define numberof(_x) (sizeof(_x)/sizeof(_x[0]))
#endif

namespace s3dm {

   enum e3dmouse_virtual_key 
   {
      V3DK_INVALID=0
      , V3DK_MENU=1, V3DK_FIT
      , V3DK_TOP, V3DK_LEFT, V3DK_RIGHT, V3DK_FRONT, V3DK_BOTTOM, V3DK_BACK
      , V3DK_ROLL_CW, V3DK_ROLL_CCW
      , V3DK_ISO1, V3DK_ISO2
      , V3DK_1, V3DK_2, V3DK_3, V3DK_4, V3DK_5, V3DK_6, V3DK_7, V3DK_8, V3DK_9, V3DK_10
      , V3DK_ESC, V3DK_ALT, V3DK_SHIFT, V3DK_CTRL
      , V3DK_ROTATE, V3DK_PANZOOM, V3DK_DOMINANT
      , V3DK_PLUS, V3DK_MINUS
      , V3DK_SPIN_CW, V3DK_SPIN_CCW
      , V3DK_TILT_CW, V3DK_TILT_CCW
      , V3DK_BUTTON_UI
      , V3DK_USER = 0x10000
   };

   static const e3dmouse_virtual_key VirtualKeys[]=
   {
      V3DK_MENU, V3DK_FIT
      , V3DK_TOP, V3DK_LEFT, V3DK_RIGHT, V3DK_FRONT, V3DK_BOTTOM, V3DK_BACK
      , V3DK_ROLL_CW, V3DK_ROLL_CCW
      , V3DK_ISO1, V3DK_ISO2
      , V3DK_1, V3DK_2, V3DK_3, V3DK_4, V3DK_5, V3DK_6, V3DK_7, V3DK_8, V3DK_9, V3DK_10
      , V3DK_ESC, V3DK_ALT, V3DK_SHIFT, V3DK_CTRL
      , V3DK_ROTATE, V3DK_PANZOOM, V3DK_DOMINANT
      , V3DK_PLUS, V3DK_MINUS
      , V3DK_SPIN_CW, V3DK_SPIN_CCW
      , V3DK_TILT_CW, V3DK_TILT_CCW
      , V3DK_BUTTON_UI

   };

   static const size_t VirtualKeyCount = numberof(VirtualKeys);
   static const size_t MaxKeyCount = 64; // Arbitary number for sanity checks

   struct tag_VirtualKeyLabel
   {
     e3dmouse_virtual_key vkey;
     const wchar_t* szLabel;
   };

   static const struct tag_VirtualKeyLabel VirtualKeyLabel[] =
   {
     {V3DK_MENU, L"MENU"} , {V3DK_FIT, L"FIT"}
     , {V3DK_TOP, L"T"}, {V3DK_LEFT, L"L"}, {V3DK_RIGHT, L"R"}, {V3DK_FRONT, L"F"}, {V3DK_BOTTOM, L"B"}, {V3DK_BACK, L"BK"}
     , {V3DK_ROLL_CW, L"Roll +"}, {V3DK_ROLL_CCW, L"Roll -"}
     , {V3DK_ISO1, L"ISO1"}, {V3DK_ISO2, L"ISO2"}
     , {V3DK_1, L"1"}, {V3DK_2, L"2"}, {V3DK_3, L"3"}, {V3DK_4, L"4"}, {V3DK_5, L"5"}
     , {V3DK_6, L"6"}, {V3DK_7, L"7"}, {V3DK_8, L"8"}, {V3DK_9, L"9"}, {V3DK_10, L"10"}
     , {V3DK_ESC, L"ESC"}, {V3DK_ALT, L"ALT"}, {V3DK_SHIFT, L"SHIFT"}, {V3DK_CTRL, L"CTRL"}
     , {V3DK_ROTATE, L"Rotate"}, {V3DK_PANZOOM, L"Pan Zoom"}, {V3DK_DOMINANT, L"Dom"}
     , {V3DK_PLUS, L"+"}, {V3DK_MINUS, L"-"}
     , {V3DK_SPIN_CW, L"Spin +"}, {V3DK_SPIN_CCW, L"Spin -"}
     , {V3DK_TILT_CW, L"Tilt +"}, {V3DK_TILT_CCW, L"Tilt -"}
     , {V3DK_BUTTON_UI, L"UI"}
   };

   struct tag_VirtualKeyId
   {
     e3dmouse_virtual_key vkey;
     const wchar_t* szId;
   };

   static const struct tag_VirtualKeyId VirtualKeyId[] =
   {
     {V3DK_INVALID, L"V3DK_INVALID"}
     , {V3DK_MENU, L"V3DK_MENU"} , {V3DK_FIT, L"V3DK_FIT"}
     , {V3DK_TOP, L"V3DK_TOP"}, {V3DK_LEFT, L"V3DK_LEFT"}, {V3DK_RIGHT, L"V3DK_RIGHT"}, {V3DK_FRONT, L"V3DK_FRONT"}, {V3DK_BOTTOM, L"V3DK_BOTTOM"}, {V3DK_BACK, L"V3DK_BACK"}
     , {V3DK_ROLL_CW, L"V3DK_ROLL_CW"}, {V3DK_ROLL_CCW, L"V3DK_ROLL_CCW"}
     , {V3DK_ISO1, L"V3DK_ISO1"}, {V3DK_ISO2, L"V3DK_ISO2"}
     , {V3DK_1, L"V3DK_1"}, {V3DK_2, L"V3DK_2"}, {V3DK_3, L"V3DK_3"}, {V3DK_4, L"V3DK_4"}, {V3DK_5, L"V3DK_5"}
     , {V3DK_6, L"V3DK_6"}, {V3DK_7, L"V3DK_7"}, {V3DK_8, L"V3DK_8"}, {V3DK_9, L"V3DK_9"}, {V3DK_10, L"V3DK_10"}
     , {V3DK_ESC, L"V3DK_ESC"}, {V3DK_ALT, L"V3DK_ALT"}, {V3DK_SHIFT, L"V3DK_SHIFT"}, {V3DK_CTRL, L"V3DK_CTRL"}
     , {V3DK_ROTATE, L"V3DK_ROTATE"}, {V3DK_PANZOOM, L"V3DK_PANZOOM"}, {V3DK_DOMINANT, L"V3DK_DOMINANT"}
     , {V3DK_PLUS, L"V3DK_PLUS"}, {V3DK_MINUS, L"V3DK_MINUS"}
     , {V3DK_SPIN_CW, L"V3DK_SPIN_CW"}, {V3DK_SPIN_CCW, L"V3DK_SPIN_CCW"}
     , {V3DK_TILT_CW, L"V3DK_TILT_CW"}, {V3DK_TILT_CCW, L"V3DK_TILT_CCW"}
     , {V3DK_BUTTON_UI, L"V3DK_BUTTON_UI"}
   };

   /*-----------------------------------------------------------------------------
   *
   * const  wchar_t* VirtualKeyToId(e3dmouse_virtual_key virtualkey)
   *
   * Args:
   *    virtualkey  the 3dmouse virtual key 
   *
   * Return Value:
   *    Returns a string representation of the standard 3dmouse virtual key, or
   *    an empty string 
   *
   * Description:
   *    Converts a 3dmouse virtual key number to its string identifier
   *
   *---------------------------------------------------------------------------*/
   __inline const wchar_t* VirtualKeyToId(e3dmouse_virtual_key virtualkey)
   {
      if (0 < virtualkey && virtualkey <= numberof(VirtualKeyId) 
        && virtualkey == VirtualKeyId[virtualkey-1].vkey)
        return VirtualKeyId[virtualkey-1].szId;

      for (size_t i=0; i<numberof(VirtualKeyId); ++i)
      {
        if (VirtualKeyId[i].vkey == virtualkey)
          return VirtualKeyId[i].szId;
      }
      return L"";
   }


   /*-----------------------------------------------------------------------------
   *
   * const e3dmouse_virtual_key IdToVirtualKey ( TCHAR *id )
   *
   * Args:
   *    id  - the 3dmouse virtual key ID (a string)
   *
   * Return Value:
   *    The virtual_key number for the id, 
   *    or V3DK_INVALID if it is not a valid tag_VirtualKeyId
   *
   * Description:
   *    Converts a 3dmouse virtual key id (a string) to the V3DK number
   *
   *---------------------------------------------------------------------------*/
   __inline e3dmouse_virtual_key IdToVirtualKey(const wchar_t* id)
   {
      for (size_t i=0; i<sizeof(VirtualKeyId)/sizeof(VirtualKeyId[0]); ++i)
      {
        if (_tcsicmp (VirtualKeyId[i].szId, id) == 0)
          return VirtualKeyId[i].vkey;
      }
      return V3DK_INVALID;
   }


   /*-----------------------------------------------------------------------------
   *
   * const wchar_t* GetKeyLabel(e3dmouse_virtual_key virtualkey)
   *
   * Args:
   *    virtualkey  the 3dmouse virtual key 
   *
   * Return Value:
   *    Returns a string of thye label used on the standard 3dmouse virtual key, or
   *    an empty string 
   *
   * Description:
   *    Converts a 3dmouse virtual key number to its label
   *
   *---------------------------------------------------------------------------*/
   __inline const wchar_t* GetKeyLabel(e3dmouse_virtual_key virtualkey)
   {
      for (size_t i=0; i<numberof(VirtualKeyLabel); ++i)
      {
        if (VirtualKeyLabel[i].vkey == virtualkey)
          return VirtualKeyLabel[i].szLabel;
      }
      return L"";
   }

} // namespace s3dm

namespace tdx {
   enum e3dconnexion_pid {
      eSpacePilot = 0xc625
      , eSpaceNavigator = 0xc626
      , eSpaceExplorer = 0xc627
      , eSpaceNavigatorForNotebooks = 0xc628
      , eSpacePilotPRO = 0xc629
      , eSpaceMousePRO = 0xc62b
      , eSpaceMouseTouch = 0xc62c
      , eSpaceMouse = 0xc62d
   };


   struct tag_VirtualKeys
   {
      e3dconnexion_pid pid;
      size_t nLength;
      s3dm::e3dmouse_virtual_key *vkeys;
      size_t nKeys;
   };

   static const s3dm::e3dmouse_virtual_key SpaceExplorerKeys [] = 
   {
      s3dm::V3DK_INVALID     // there is no button 0
      , s3dm::V3DK_1, s3dm::V3DK_2
      , s3dm::V3DK_TOP, s3dm::V3DK_LEFT, s3dm::V3DK_RIGHT, s3dm::V3DK_FRONT
      , s3dm::V3DK_ESC, s3dm::V3DK_ALT, s3dm::V3DK_SHIFT, s3dm::V3DK_CTRL
      , s3dm::V3DK_FIT, s3dm::V3DK_MENU
      , s3dm::V3DK_PLUS, s3dm::V3DK_MINUS
      , s3dm::V3DK_ROTATE
   };

   static const s3dm::e3dmouse_virtual_key SpacePilotKeys [] = 
   {
      s3dm::V3DK_INVALID 
      , s3dm::V3DK_1, s3dm::V3DK_2, s3dm::V3DK_3, s3dm::V3DK_4, s3dm::V3DK_5, s3dm::V3DK_6
      , s3dm::V3DK_TOP, s3dm::V3DK_LEFT, s3dm::V3DK_RIGHT, s3dm::V3DK_FRONT
      , s3dm::V3DK_ESC, s3dm::V3DK_ALT, s3dm::V3DK_SHIFT, s3dm::V3DK_CTRL
      , s3dm::V3DK_FIT, s3dm::V3DK_MENU
      , s3dm::V3DK_PLUS, s3dm::V3DK_MINUS
      , s3dm::V3DK_DOMINANT, s3dm::V3DK_ROTATE
      , static_cast<s3dm::e3dmouse_virtual_key>(s3dm::V3DK_USER+0x01)
   };

   static const s3dm::e3dmouse_virtual_key SpaceMouseKeys [] = 
   {
      s3dm::V3DK_INVALID 
      , s3dm::V3DK_MENU, s3dm::V3DK_FIT
   };

   static const s3dm::e3dmouse_virtual_key SpacePilotProKeys [] = 
   {
      s3dm::V3DK_INVALID 
      , s3dm::V3DK_MENU, s3dm::V3DK_FIT
      , s3dm::V3DK_TOP, s3dm::V3DK_LEFT, s3dm::V3DK_RIGHT, s3dm::V3DK_FRONT, s3dm::V3DK_BOTTOM, s3dm::V3DK_BACK
      , s3dm::V3DK_ROLL_CW, s3dm::V3DK_ROLL_CCW
      , s3dm::V3DK_ISO1, s3dm::V3DK_ISO2
      , s3dm::V3DK_1, s3dm::V3DK_2, s3dm::V3DK_3, s3dm::V3DK_4, s3dm::V3DK_5
      , s3dm::V3DK_6, s3dm::V3DK_7, s3dm::V3DK_8, s3dm::V3DK_9, s3dm::V3DK_10
      , s3dm::V3DK_ESC, s3dm::V3DK_ALT, s3dm::V3DK_SHIFT, s3dm::V3DK_CTRL
      , s3dm::V3DK_ROTATE, s3dm::V3DK_PANZOOM, s3dm::V3DK_DOMINANT
      , s3dm::V3DK_PLUS, s3dm::V3DK_MINUS
   };

   static const s3dm::e3dmouse_virtual_key SpaceMouseProKeys [] = 
   {
      s3dm::V3DK_INVALID 
      , s3dm::V3DK_MENU, s3dm::V3DK_FIT
      , s3dm::V3DK_TOP, s3dm::V3DK_INVALID, s3dm::V3DK_RIGHT, s3dm::V3DK_FRONT, s3dm::V3DK_INVALID, s3dm::V3DK_INVALID
      , s3dm::V3DK_ROLL_CW, s3dm::V3DK_INVALID
      , s3dm::V3DK_INVALID, s3dm::V3DK_INVALID
      , s3dm::V3DK_1, s3dm::V3DK_2, s3dm::V3DK_3, s3dm::V3DK_4, s3dm::V3DK_INVALID
      , s3dm::V3DK_INVALID, s3dm::V3DK_INVALID, s3dm::V3DK_INVALID, s3dm::V3DK_INVALID, s3dm::V3DK_INVALID
      , s3dm::V3DK_ESC, s3dm::V3DK_ALT, s3dm::V3DK_SHIFT, s3dm::V3DK_CTRL
      , s3dm::V3DK_ROTATE
   };

   static const s3dm::e3dmouse_virtual_key SpaceMouseTouchKeys [] = 
   {
      s3dm::V3DK_INVALID 
      , s3dm::V3DK_MENU, s3dm::V3DK_FIT
      , s3dm::V3DK_TOP, s3dm::V3DK_LEFT, s3dm::V3DK_RIGHT, s3dm::V3DK_FRONT, s3dm::V3DK_BOTTOM, s3dm::V3DK_BACK
      , s3dm::V3DK_ROLL_CW, s3dm::V3DK_ROLL_CCW
      , s3dm::V3DK_ISO1, s3dm::V3DK_ISO2
      , s3dm::V3DK_1, s3dm::V3DK_2, s3dm::V3DK_3, s3dm::V3DK_4, s3dm::V3DK_5
      , s3dm::V3DK_6, s3dm::V3DK_7, s3dm::V3DK_8, s3dm::V3DK_9, s3dm::V3DK_10
   };


   static const struct tag_VirtualKeys _3dmouseHID2VirtualKeys[]= 
   {
      eSpacePilot
      , numberof(SpacePilotKeys)
      , const_cast<s3dm::e3dmouse_virtual_key *>(SpacePilotKeys)
      , numberof(SpacePilotKeys)-1

      , eSpaceExplorer
      , numberof(SpaceExplorerKeys)
      , const_cast<s3dm::e3dmouse_virtual_key *>(SpaceExplorerKeys)
      , numberof(SpaceExplorerKeys)-1

      , eSpaceNavigator
      , numberof(SpaceMouseKeys)
      , const_cast<s3dm::e3dmouse_virtual_key *>(SpaceMouseKeys)
      , numberof(SpaceMouseKeys)-1
      
      , eSpaceNavigatorForNotebooks
      , numberof(SpaceMouseKeys)
      , const_cast<s3dm::e3dmouse_virtual_key *>(SpaceMouseKeys)
      , numberof(SpaceMouseKeys)-1
      
      , eSpacePilotPRO
      , numberof(SpacePilotProKeys)
      , const_cast<s3dm::e3dmouse_virtual_key *>(SpacePilotProKeys)
      , numberof(SpacePilotProKeys)-1

      , eSpaceMousePRO
      , numberof(SpaceMouseProKeys)
      , const_cast<s3dm::e3dmouse_virtual_key *>(SpaceMouseProKeys)
      , 15

      , eSpaceMouse
      , numberof(SpaceMouseKeys)
      , const_cast<s3dm::e3dmouse_virtual_key *>(SpaceMouseKeys)
      , numberof(SpaceMouseKeys)-1

      , eSpaceMouseTouch
      , numberof(SpaceMouseTouchKeys)
      , const_cast<s3dm::e3dmouse_virtual_key *>(SpaceMouseTouchKeys)
      , numberof(SpaceMouseTouchKeys)-1

   };



   /*-----------------------------------------------------------------------------
   *
   * unsigned short HidToVirtualKey(unsigned short pid, unsigned short hidKeyCode)
   *
   * Args:
   *    pid - USB Product ID (PID) of 3D mouse device 
   *    hidKeyCode - Hid keycode as retrieved from a Raw Input packet
   *
   * Return Value:
   *    Returns the standard 3d mouse virtual key (button identifier) or zero if an error occurs.
   *
   * Description:
   *    Converts a hid device keycode (button identifier) of a pre-2009 3Dconnexion USB device
   *    to the standard 3d mouse virtual key definition.
   *
   *---------------------------------------------------------------------------*/
   __inline unsigned short HidToVirtualKey(unsigned long pid, unsigned short hidKeyCode)
   {
      unsigned short virtualkey=hidKeyCode;
      for (size_t i=0; i<numberof(_3dmouseHID2VirtualKeys); ++i)
      {
         if (pid == _3dmouseHID2VirtualKeys[i].pid)
         {
            if (hidKeyCode < _3dmouseHID2VirtualKeys[i].nLength)
               virtualkey = _3dmouseHID2VirtualKeys[i].vkeys[hidKeyCode];

//          Change 10/24/2012: if the key doesn't need translating then pass it through
//            else
//              virtualkey = s3dm::V3DK_INVALID;
            break;
         }
      }
      // Remaining devices are unchanged
#if _TRACE_VIRTUAL_KEYS
     TRACE(L"Converted %d to %s(=%d) for pid 0x%x\n", hidKeyCode, VirtualKeyToId(virtualkey), virtualkey, pid);
#endif
      return virtualkey;
   }

   /*-----------------------------------------------------------------------------
   *
   * unsigned short VirtualKeyToHid(unsigned short pid, unsigned short virtualkey)
   *
   * Args:
   *    pid - USB Product ID (PID) of 3D mouse device 
   *    virtualkey - standard 3d mouse virtual key
   *
   * Return Value:
   *    Returns the Hid keycode as retrieved from a Raw Input packet
   *
   * Description:
   *    Converts a standard 3d mouse virtual key definition 
   *    to the hid device keycode (button identifier).
   *
   *---------------------------------------------------------------------------*/
   __inline unsigned short VirtualKeyToHid(unsigned long pid, unsigned short virtualkey)
   {
      unsigned short hidKeyCode = virtualkey;
      for (size_t i=0; i<numberof(_3dmouseHID2VirtualKeys); ++i)
      {
         if (pid == _3dmouseHID2VirtualKeys[i].pid)
         {
            for (unsigned short hidCode=0; hidCode<_3dmouseHID2VirtualKeys[i].nLength; ++hidCode)
            {
              if (virtualkey==_3dmouseHID2VirtualKeys[i].vkeys[hidCode])
                return hidCode;
            }
//          Change 10/24/2012: if the key doesn't need translating then pass it through
            return hidKeyCode;
         }
      }
      // Remaining devices are unchanged
#if _TRACE_VIRTUAL_KEYS
     TRACE(L"Converted %d to %s(=%d) for pid 0x%x\n", virtualkey, VirtualKeyToId(virtualkey), hidKeyCode, pid);
#endif
      return hidKeyCode;
   }

   /*-----------------------------------------------------------------------------
   *
   * unsigned int NumberOfButtons(unsigned short pid)
   *
   * Args:
   *    pid - USB Product ID (PID) of 3D mouse device 
   *
   * Return Value:
   *    Returns the number of buttons of the device.
   *
   * Description:
   *   Returns the number of buttons of the device.
   *
   *---------------------------------------------------------------------------*/
   __inline size_t NumberOfButtons(unsigned long pid)
   {
      for (size_t i=0; i<numberof(_3dmouseHID2VirtualKeys); ++i)
      {
         if (pid == _3dmouseHID2VirtualKeys[i].pid)
            return _3dmouseHID2VirtualKeys[i].nKeys;
      }
      return 0;
   }

   /*-----------------------------------------------------------------------------
   *
   * int HidToIndex(unsigned short pid, unsigned short hidKeyCode)
   *
   * Args:
   *    pid - USB Product ID (PID) of 3D mouse device 
   *    hidKeyCode - Hid keycode as retrieved from a Raw Input packet
   *
   * Return Value:
   *    Returns the index of the hid button or -1 if an error occurs.
   *
   * Description:
   *    Converts a hid device keycode (button identifier) to a zero based 
   *    sequential index.
   *
   *---------------------------------------------------------------------------*/
   __inline int HidToIndex(unsigned long pid, unsigned short hidKeyCode)
   {
      for (size_t i=0; i<numberof(_3dmouseHID2VirtualKeys); ++i)
      {
         if (pid == _3dmouseHID2VirtualKeys[i].pid)
         {
            int index=-1;
            if (hidKeyCode < _3dmouseHID2VirtualKeys[i].nLength)
            {
              unsigned short virtualkey = _3dmouseHID2VirtualKeys[i].vkeys[hidKeyCode];
              if (virtualkey != s3dm::V3DK_INVALID)
              {
                for (int key=1; key<=hidKeyCode; ++key)
                {
                  if (_3dmouseHID2VirtualKeys[i].vkeys[key] != s3dm::V3DK_INVALID)
                    ++index;
                }
              }
            }
            return index;
         }
      }
      return hidKeyCode-1;
   }

   /*-----------------------------------------------------------------------------
   *
   * unsigned short IndexToHid(unsigned short pid, int index)
   *
   * Args:
   *    pid - USB Product ID (PID) of 3D mouse device 
   *    index - index of button
   *
   * Return Value:
   *    Returns the Hid keycode of the nth button or 0 if an error occurs.
   *
   * Description:
   *    Returns the hid device keycode of the nth button
   *
   *---------------------------------------------------------------------------*/
   __inline unsigned short IndexToHid(unsigned long pid, int index)
   {
      if (index < 0)
        return 0;
      for (size_t i=0; i<numberof(_3dmouseHID2VirtualKeys); ++i)
      {
         if (pid == _3dmouseHID2VirtualKeys[i].pid)
         {
            if (index < static_cast<int>(_3dmouseHID2VirtualKeys[i].nLength))
            {
              for (size_t key=1; key<_3dmouseHID2VirtualKeys[i].nLength; ++key)
              {
                if (_3dmouseHID2VirtualKeys[i].vkeys[key] != s3dm::V3DK_INVALID)
                {
                  --index;
                  if (index == -1)
                    return static_cast<unsigned short>(key);
                }
              }
            }
            return 0;
         }
      }
      return static_cast<unsigned short>(index+1);
   }

}; //namespace tdx
#endif // virtualkeys_HPP_INCLUDED_
