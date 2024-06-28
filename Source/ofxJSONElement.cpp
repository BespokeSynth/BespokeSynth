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

#include <json/reader.h>
#include <json/writer.h>

#include "juce_core/juce_core.h"

using namespace Json;


//--------------------------------------------------------------
ofxJSONElement::ofxJSONElement(const Json::Value& v)
: Value(v)
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
   CharReaderBuilder rb;
   auto reader = std::unique_ptr<Json::CharReader>(rb.newCharReader());
   Json::String errors;
   if (!reader->parse(jsonString.c_str(),
                      jsonString.c_str() + jsonString.size(),
                      this, &errors))
   {
      ofLog() << "Unable to parse string: " << errors;
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

      CharReaderBuilder builder;
      auto reader = std::unique_ptr<CharReader>(builder.newCharReader());
      auto mS = str.toStdString();

      if (!reader->parse(mS.c_str(), mS.c_str() + mS.size(), this, nullptr))
      {
         ofLog() << "Unable to parse " + filename;
         return false;
      }
   }
   else
   {
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
   if (!file.exists())
   {
      ofLog() << "Unable to create " + filename;
      return false;
   }

   Json::StreamWriterBuilder builder;

   if (pretty)
   {
      builder["indentation"] = "   ";
   }
   const std::string json_file = Json::writeString(builder, *this);
   file.replaceWithText(json_file);

   ofLog() << "JSON saved to " + filename;
   return true;
}


//--------------------------------------------------------------
std::string ofxJSONElement::getRawString(bool pretty)
{
   std::string raw;
   Json::StreamWriterBuilder builder;

   if (pretty)
   {
      builder["indentation"] = "   ";
   }
   raw = Json::writeString(builder, *this);
   return raw;
}
