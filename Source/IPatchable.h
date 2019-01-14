//
//  IPatchable.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/13/15.
//
//

#ifndef Bespoke_IPatchable_h
#define Bespoke_IPatchable_h

class PatchCableSource;

class IPatchable
{
public:
   virtual ~IPatchable() {}
   virtual PatchCableSource* GetPatchCableSource(int index=0) = 0;
   virtual void PreRepatch(PatchCableSource* cableSource) {}
   virtual void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) {}
   virtual void OnCableGrabbed(PatchCableSource* cableSource) {}
};

#endif
