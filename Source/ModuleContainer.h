/*
  ==============================================================================

    ModuleContainer.h
    Created: 16 Oct 2016 3:47:41pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#ifndef MODULECONTAINER_H_INCLUDED
#define MODULECONTAINER_H_INCLUDED

#include "OpenFrameworksPort.h"
#include "IDrawableModule.h"
#include "ofxJSONElement.h"

class ModuleContainer
{
public:
   ModuleContainer();
   
   const vector<IDrawableModule*>& GetModules() const { return mModules; }
   
   void SetOwner(IDrawableModule* owner) { mOwner = owner; }
   void Draw();
   void DrawPatchCables();
   void PostRender();
   void Poll();
   void Clear();
   void Exit();
   
   void KeyPressed(int key, bool isRepeat);
   void KeyReleased(int key);
   void MouseMoved(float x, float y);
   void MouseReleased();
   IDrawableModule* GetModuleAt(float x, float y);
   void GetModulesWithinRect(ofRectangle rect, vector<IDrawableModule*>& output);
   void MoveToFront(IDrawableModule* module);
   void AddModule(IDrawableModule* module);
   void TakeModule(IDrawableModule* module);
   void DeleteModule(IDrawableModule* module);
   IDrawableModule* FindModule(string name, bool fail = true);
   IUIControl* FindUIControl(string path);
   bool IsHigherThan(IDrawableModule* checkFor, IDrawableModule* checkAgainst) const;
   void GetAllModules(vector<IDrawableModule*>& out);
   
   template<class T> vector<string> GetModuleNames()
   {
      vector<string> ret;
      for (int i=0; i<mModules.size(); ++i)
      {
         if (dynamic_cast<T>(mModules[i]))
            ret.push_back(mModules[i]->Name());
      }
      return ret;
   }
   
   void LoadModules(const ofxJSONElement& modules);
   ofxJSONElement WriteModules();
   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);
   
   static constexpr int GetModuleSeparatorLength() { return 13; }
   static const char* GetModuleSeparator() { return "ryanchallinor"; }
   static bool DoesModuleHaveMoreSaveData(FileStreamIn& in);
   
private:
   ofVec2f GetOwnerPosition() const;
   
   vector<IDrawableModule*> mModules;
   IDrawableModule* mOwner;
};

#endif  // MODULECONTAINER_H_INCLUDED
