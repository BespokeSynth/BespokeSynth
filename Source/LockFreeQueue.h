/*
  ==============================================================================

    LockFreeQueue.h
    Created: 1 Apr 2014 6:27:32pm
    Author:  Tom Maisey

  ==============================================================================
*/

#pragma once

/**
 * A simple single producer & consumer lock free queue, based on Herb Sutter's code:
 * http://www.drdobbs.com/parallel/writing-lock-free-code-a-corrected-queue/
 * 
 * This is a linked list, which can expand arbitrarily without losing old data,
 * but which has poor cache locality (I plan to write a ring buffer version soon).
 */
template <typename T>
class LockFreeQueue
{
public:
   LockFreeQueue()
   {
      first = new Node(T()); // Dummy seperator.

      last.set(first);
      divider.set(first);
   }

   ~LockFreeQueue()
   {
      while (first != nullptr)
      {
         Node* toDel = first;
         first = toDel->next;
         delete toDel;
      }
   }

   /**
     * Add an item to the queue. Obviously, should only be called from producer's thread.
     */
   void produce(const T& t)
   {
      last.get()->next = new Node(t);

      last.set(last.get()->next);

      while (first != divider.get())
      { // trim unused nodes
         Node* tmp = first;
         first = first->next;
         delete tmp;
      }
   }

   /**
     * Consume an item in the queue. Returns false if no items left to consume.
     */
   bool consume(T& result)
   {
      Node* div = divider.get();

      if (div != last.get())
      { // if queue is nonempty
         result = div->next->value; // copy requested value
         divider.set(div->next); // publish that we took it
         return true;
      }

      return false;
   }

private:
   struct Node
   {
      Node(const T& val)
      : value(val)
      {}

      T value;
      Node* next{ nullptr };
   };

   Node* first{ nullptr };
   Atomic<Node*> divider, last;
};
