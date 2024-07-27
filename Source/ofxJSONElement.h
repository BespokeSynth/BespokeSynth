/*
 *  ofxJSONFile.h
 *  asift
 *
 *  Created by Jeffrey Crouse on 12/17/10.
 *  Copyright 2010 Eyebeam. All rights reserved.
 *
 */

#pragma once

#include <json/json.h>
#include "OpenFrameworksPort.h"

//using namespace Json;
extern "C" size_t decode_html_entities_utf8(char* dest, const char* src);

class ofxJSONElement : public Json::Value
{
public:
   ofxJSONElement(){};
   ofxJSONElement(std::string jsonString);
   ofxJSONElement(const Json::Value& v);

   bool parse(std::string jsonString);
   bool open(std::string filename);
   bool save(std::string filename, bool pretty = false);
   std::string getRawString(bool pretty = true);
};
