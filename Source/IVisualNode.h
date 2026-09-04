/**
    bespoke synth - IVisualNode

    Mix-in interface for GPU visual nodes (TouchDesigner-TOP style). A node renders its output into a
    GL texture; downstream nodes / sinks pull that texture via GetOutputTexture(). Cooking is
    pull-based and memoized per frame so a node feeding several consumers only renders once.
**/

#pragma once

class IVisualNode
{
public:
   virtual ~IVisualNode() {}

   //return the GL texture id of this node's most recent output (0 if nothing yet)
   virtual unsigned int GetOutputTexture() = 0;

   //output resolution of that texture
   virtual int GetOutputWidth() const = 0;
   virtual int GetOutputHeight() const = 0;

   //ensure this node has produced its output for the given frame (memoized). Nodes with inputs
   //should pull their inputs' CookIfNeeded() before rendering themselves.
   virtual void CookIfNeeded(int frameId) = 0;
};
