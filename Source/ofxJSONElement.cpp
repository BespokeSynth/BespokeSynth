/*
 *  ofxJSONFile.cpp
 *  asift
 *
 *  Created by Jeffrey Crouse on 12/17/10.
 *  Copyright 2010 Eyebeam. All rights reserved.
 *
 */

#include "ofxJSONElement.h"
#include "SynthGlobals.h"

#include "juce_core/juce_core.h"

using namespace Json;


//--------------------------------------------------------------
ofxJSONElement::ofxJSONElement(const Json::Value& v) : Value(v)
{
   
}


//--------------------------------------------------------------
ofxJSONElement::ofxJSONElement(std::string jsonString)
{
   parse(jsonString);
}


//--------------------------------------------------------------
bool ofxJSONElement::parse(std::string jsonString)
{
   Reader reader;
   if(!reader.parse( jsonString, *this )) {
      ofLog() << "Unable to parse string";
      return false;
   }
   return true;
}


//--------------------------------------------------------------
bool ofxJSONElement::open(std::string filename)
{
   juce::File file(filename);
   
   if (file.exists())
   {
      juce::String str = file.loadFileAsString();
      
      Reader reader;
      if(!reader.parse( str.toStdString(), *this ))
      {
         ofLog() << "Unable to parse "+filename;
         return false;
      }
   } else {
      ofLog() << "Could not load file " + filename;
      return false;
   }
   
   return true;
}


//--------------------------------------------------------------
bool ofxJSONElement::save(std::string filename, bool pretty)
{
   filename = ofToDataPath(filename);
   juce::File file(filename);
   file.create();
   if (!file.exists()) {
      ofLog() << "Unable to create "+filename;
      return false;
   }
   
   if(pretty) {
      StyledWriter writer;
      file.replaceWithText(writer.write( *this ));
   } else {
      FastWriter writer;
      file.replaceWithText(writer.write( *this ));
   }
   ofLog() << "JSON saved to "+filename;
   return true;
}



//--------------------------------------------------------------
std::string ofxJSONElement::getRawString(bool pretty)
{
   std::string raw;
   if(pretty) {
      StyledWriter writer;
      raw = writer.write(*this);
   } else {
      FastWriter writer;
      raw = writer.write(*this);
   }
   return raw;
}

//--------------------------------------------------------------
std::string ofxJSONElement::decodeURL(std::string &SRC)
{
   std::string ret;
   char ch;
   int i, ii;
   for (i=0; i<SRC.length(); i++) {
      if (int(SRC[i])==37) {
         sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
         ch=static_cast<char>(ii);
         ret+=ch;
         i=i+2;
      } else {
         ret+=SRC[i];
      }
   }
   return (ret);
}
