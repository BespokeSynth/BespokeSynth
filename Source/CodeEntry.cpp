/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
/*
  ==============================================================================

    CodeEntry.cpp
    Created: 19 Apr 2020 9:26:55am
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ModularSynth.h"
#ifdef _MSC_VER
#define ssize_t ssize_t_undef_hack //fixes conflict with ssize_t typedefs between python and juce
#endif
#include "IDrawableModule.h"
#include "CodeEntry.h"
#include "Transport.h"
#include "ScriptModule.h"
#ifdef _MSC_VER
#undef ssize_t
#endif

#include "leathers/push"
#include "leathers/unused-value"
#include "leathers/range-loop-analysis"
#include "pybind11/embed.h"
#include "pybind11/stl.h"
#include "leathers/pop"

#include "juce_gui_basics/juce_gui_basics.h"

namespace py = pybind11;

//static
bool CodeEntry::sWarnJediNotInstalled = false;
bool CodeEntry::sDoPythonAutocomplete = false;
bool CodeEntry::sDoSyntaxHighlighting = false;

CodeEntry::CodeEntry(ICodeEntryListener* owner, const char* name, int x, int y, float w, float h)
: mListener(owner)
{
   SetName(name);
   SetPosition(x, y);
   mWidth = w;
   mHeight = h;
   IDrawableModule* module = dynamic_cast<IDrawableModule*>(owner);
   assert(module);
   module->AddUIControl(this);
   SetParent(dynamic_cast<IClickable*>(owner));
}

CodeEntry::~CodeEntry()
{
}

void CodeEntry::Poll()
{
   if (mCodeUpdated)
   {
      if (mDoSyntaxHighlighting && sDoSyntaxHighlighting)
      {
         try
         {
            py::globals()["syntax_highlight_code"] = GetVisibleCode();
            py::object ret = py::eval("syntax_highlight_basic()", py::globals());
            mSyntaxHighlightMapping = ret.cast<std::vector<int> >();
         }
         catch (const std::exception& e)
         {
            ofLog() << "syntax highlight execution exception: " << e.what();
         }
      }
      else
      {
         mSyntaxHighlightMapping.clear();
      }

      if (mListener)
         mListener->OnCodeUpdated();

      mCodeUpdated = false;
   }

   if (mAutocompleteUpdateTimer > 0)
   {
      mAutocompleteUpdateTimer -= 1.0 / ofGetFrameRate();
      if (mAutocompleteUpdateTimer <= 0)
      {
         if (sDoPythonAutocomplete)
         {
            mAutocompleteCaretCoords = GetCaretCoords(mCaretPosition);

            if (!GetVisibleCode().empty())
            {
               try
               {
                  std::string prefix = ScriptModule::GetBootstrapImportString() + "; import me\n";
                  py::exec("jediScript = jedi.Script('''" + prefix + GetVisibleCode() + "''', project=jediProject)", py::globals());
                  //py::exec("jediScript = jedi.Script('''" + prefix + GetVisibleCode() + "''')", py::globals());
                  //py::exec("jediScript = jedi.Interpreter('''" + prefix + GetVisibleCode() + "''', [locals(), globals()])", py::globals());
                  auto coords = GetCaretCoords(mCaretPosition);

                  {
                     py::object ret = py::eval("jediScript.get_signatures(" + ofToString(coords.y + 2) + "," + ofToString(coords.x) + ")", py::globals());
                     auto signatures = ret.cast<std::list<py::object> >();

                     size_t i = 0;
                     for (auto signature : signatures)
                     {
                        mWantToShowAutocomplete = true;
                        if (i < mAutocompleteSignatures.size())
                        {
                           mAutocompleteSignatures[i].valid = true;
                           mAutocompleteSignatures[i].entryIndex = signature.attr("index").cast<int>();
                           auto params = signature.attr("params").cast<std::vector<py::object> >();
                           mAutocompleteSignatures[i].params.resize(params.size());
                           for (size_t j = 0; j < params.size(); ++j)
                              mAutocompleteSignatures[i].params[j] = juce::String(py::str(params[j].attr("description"))).replace("param ", "").toStdString();
                           auto bracket_start = signature.attr("bracket_start").cast<std::tuple<int, int> >();
                           mAutocompleteSignatures[i].caretPos = GetCaretPosition(std::get<1>(bracket_start), std::get<0>(bracket_start) - 2);
                           ++i;
                        }
                        else
                        {
                           break;
                        }
                     }

                     for (; i < mAutocompleteSignatures.size(); ++i)
                        mAutocompleteSignatures[i].valid = false;
                  }

                  {
                     py::object ret = py::eval("jediScript.complete(" + ofToString(coords.y + 2) + "," + ofToString(coords.x) + ")", py::globals());
                     auto autocompletes = ret.cast<std::list<py::object> >();
                     //ofLog() << "autocompletes:";

                     size_t i = 0;
                     if (autocompletes.size() < 100)
                     {
                        mWantToShowAutocomplete = true;
                        mAutocompleteHighlightIndex = 0;
                        bool isPathAutocomplete = false;
                        if (mAutocompleteSignatures.size() > 0 &&
                            mAutocompleteSignatures[0].valid &&
                            mAutocompleteSignatures[0].params.size() > 0 &&
                            mAutocompleteSignatures[0].params[0] == "path")
                           isPathAutocomplete = true;

                        if (!isPathAutocomplete) //normal autocomplete
                        {
                           for (auto autocomplete : autocompletes)
                           {
                              //ofLog() << "    --" << autocomplete;
                              std::string full = py::str(autocomplete.attr("name"));
                              std::string rest = py::str(autocomplete.attr("complete"));
                              if (!((juce::String)full).startsWith("__") && i < mAutocompletes.size())
                              {
                                 mAutocompletes[i].valid = true;
                                 mAutocompletes[i].autocompleteFull = full;
                                 mAutocompletes[i].autocompleteRest = rest;
                                 ++i;
                              }
                              else
                              {
                                 break;
                              }
                           }
                        }
                        else //we're autocompleting a path, look for matching instantiated module names
                        {
                           int stringStart = mAutocompleteSignatures[0].caretPos + 2;
                           std::string writtenSoFar = mString.substr(stringStart, mCaretPosition - stringStart);

                           std::vector<IDrawableModule*> modules;
                           TheSynth->GetAllModules(modules);

                           for (auto module : modules)
                           {
                              juce::String modulePath = module->Path();
                              if (modulePath.startsWith(writtenSoFar))
                              {
                                 std::string full = modulePath.toStdString();
                                 std::string rest = full;
                                 ofStringReplace(rest, writtenSoFar, "", true);
                                 if (i < mAutocompletes.size())
                                 {
                                    mAutocompletes[i].valid = true;
                                    mAutocompletes[i].autocompleteFull = full;
                                    mAutocompletes[i].autocompleteRest = rest;
                                    ++i;
                                 }
                                 else
                                 {
                                    break;
                                 }
                              }
                           }
                        }
                     }

                     for (; i < mAutocompletes.size(); ++i)
                        mAutocompletes[i].valid = false;
                  }
               }
               catch (const std::exception& e)
               {
                  ofLog() << "autocomplete exception: " << e.what();
               }
            }
            else
            {
               for (size_t i = 0; i < mAutocompleteSignatures.size(); ++i)
                  mAutocompleteSignatures[i].valid = false;
               for (size_t i = 0; i < mAutocompletes.size(); ++i)
                  mAutocompletes[i].valid = false;
            }
         }
      }
   }
}

void CodeEntry::Render()
{
   ofPushStyle();
   ofPushMatrix();

   ofSetLineWidth(.5f);

   float w, h;
   GetDimensions(w, h);

   ofClipWindow(mX, mY, w, h, true);

   bool isCurrent = IKeyboardFocusListener::GetActiveKeyboardFocus() == this;

   ofColor color = ofColor::white;

   ofFill();

   bool hasUnpublishedCode = (mString != mPublishedString);

   if (isCurrent)
   {
      ofSetColor(currentBg);
   }
   else
   {
      if (hasUnpublishedCode)
         ofSetColor(unpublishedBg);
      else
         ofSetColor(publishedBg);
   }
   ofRect(mX, mY, w, h);

   double timeSincePublished = gTime - mLastPublishTime;
   if (TheSynth->IsAudioPaused())
      timeSincePublished = 99999;

   if (timeSincePublished < 400)
   {
      ofSetColor(0, 255, 0, 150 * (1 - timeSincePublished / 400));
      ofRect(mX, mLastPublishedLineStart * mCharHeight + mY + 3 - mScroll.y, mWidth, mCharHeight * (mLastPublishedLineEnd + 1 - mLastPublishedLineStart), L(corner, 2));
   }

   ofPushStyle();
   ofNoFill();
   if (mHasError)
   {
      ofSetColor(255, 0, 0, gModuleDrawAlpha);
      ofSetLineWidth(2);
   }
   else if (hasUnpublishedCode)
   {
      float highlight = 1 - ofClamp(timeSincePublished / 150, 0, 1);
      ofSetColor(ofLerp(170, 255, highlight), 255, ofLerp(170, 255, highlight), gModuleDrawAlpha);
      ofSetLineWidth(2 + highlight * 3);
   }
   else
   {
      ofSetColor(color, gModuleDrawAlpha);
   }
   ofRect(mX, mY, w, h);
   ofPopStyle();

   if (mHasError && mErrorLine >= 0)
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255, 0, 0, gModuleDrawAlpha * .5f);
      ofRect(mX, mErrorLine * mCharHeight + mY + 3 - mScroll.y, mWidth, mCharHeight, L(corner, 2));
      ofFill();
   }

   mCharWidth = gFontFixedWidth.GetStringWidth("x", mFontSize);
   mCharHeight = gFontFixedWidth.GetStringHeight("x", mFontSize);

   if (hasUnpublishedCode)
   {
      ofSetColor(color, gModuleDrawAlpha * .05f);
      gFontFixedWidth.DrawString(mPublishedString, mFontSize, mX + 2 - mScroll.x, mY + mCharHeight - mScroll.y);
   }

   ofSetColor(color, gModuleDrawAlpha);

   std::string drawString = GetVisibleCode();

   ofPushStyle();
   const float dim = .7f;
   DrawSyntaxHighlight(drawString, stringColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 3, -1);
   DrawSyntaxHighlight(drawString, numberColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 2, -1);
   DrawSyntaxHighlight(drawString, name1Color * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 1, -1);
   DrawSyntaxHighlight(drawString, name2Color * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 90, -1);
   DrawSyntaxHighlight(drawString, name3Color * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 91, -1);
   DrawSyntaxHighlight(drawString, definedColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 92, -1);
   DrawSyntaxHighlight(drawString, equalsColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 22, -1);
   DrawSyntaxHighlight(drawString, parenColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 7, 8);
   DrawSyntaxHighlight(drawString, braceColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 25, 26);
   DrawSyntaxHighlight(drawString, bracketColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 9, 10);
   DrawSyntaxHighlight(drawString, opColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 51, -1);
   DrawSyntaxHighlight(drawString, commaColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 12, -1);
   DrawSyntaxHighlight(drawString, commentColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 53, -1);
   DrawSyntaxHighlight(drawString, unknownColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, 52, 59); //"error" token (like incomplete quotes)
   DrawSyntaxHighlight(drawString, unknownColor * (isCurrent ? 1 : dim), mSyntaxHighlightMapping, -1, -1);
   ofPopStyle();

   /*for (int i = 0; i<60; ++i)
   {
      for (int j=0; j<15; ++j)
         ofRect(mX + 2 + mCharWidth * i, mY + 2 + mCharHeight * j, mCharWidth, mCharHeight, L(corner,2));
   }*/

   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == this)
   {
      ofVec2f coords = GetCaretCoords(mCaretPosition);
      ofVec2f caretPos = ofVec2f(coords.x * mCharWidth + mX + 1.5f - mScroll.x, coords.y * mCharHeight + mY + 2 - mScroll.y);

      if (mCaretBlink)
      {
         ofFill();

         ofRect(caretPos.x, caretPos.y, 1, mCharHeight, L(corner, 1));
      }
      mCaretBlinkTimer += ofGetLastFrameTime();
      if (mCaretBlinkTimer > .3f)
      {
         mCaretBlinkTimer -= .3f;
         mCaretBlink = !mCaretBlink;
      }

      if (mCaretPosition != mCaretPosition2)
      {
         ofPushStyle();
         ofFill();
         ofSetColor(selectedOverlay);
         int caretStart = MIN(mCaretPosition, mCaretPosition2);
         int caretEnd = MAX(mCaretPosition, mCaretPosition2);
         ofVec2f coordsStart = GetCaretCoords(caretStart);
         ofVec2f coordsEnd = GetCaretCoords(caretEnd);

         int startLineNum = (int)round(coordsStart.y);
         int endLineNum = (int)round(coordsEnd.y);
         int startCol = (int)round(coordsStart.x);
         int endCol = (int)round(coordsEnd.x);
         auto lines = GetLines(false);
         for (int i = startLineNum; i <= endLineNum; ++i)
         {
            int begin = 0;
            int end = 0;
            if (i < (int)lines.size())
               end = (int)lines[i].length();
            if (i == startLineNum)
               begin = startCol;
            if (i == endLineNum)
               end = endCol;
            ofRect(begin * mCharWidth + mX + 1.5f - mScroll.x, i * mCharHeight + mY + 3 - mScroll.y, (end - begin) * mCharWidth, mCharHeight, L(corner, 2));
         }

         ofPopStyle();
      }
   }

   /*if (mHovered)
   {
      ofSetColor(100, 100, 100, .8f*gModuleDrawAlpha);
      ofFill();
      ofRect(mX,mY-12,GetStringWidth(Name()),12);
      ofSetColor(255, 255, 255, gModuleDrawAlpha);
      DrawTextNormal(Name(), mX, mY);
   }*/

   ofPopMatrix();
   ofPopStyle();
}

void CodeEntry::RenderOverlay()
{
   if (IKeyboardFocusListener::GetActiveKeyboardFocus() != this)
      return;

   if (!IsAutocompleteShowing())
      return;

   ofVec2f caretPos = ofVec2f(mAutocompleteCaretCoords.x * mCharWidth + mX + 1.5f - mScroll.x, mAutocompleteCaretCoords.y * mCharHeight + mY + 2 - mScroll.y);

   ofFill();

   for (size_t i = 0; i < mAutocompletes.size(); ++i)
   {
      if (mAutocompletes[i].valid)
      {
         int charactersLeft = (int)mAutocompletes[i].autocompleteFull.length() - (int)mAutocompletes[i].autocompleteRest.length();
         float x = caretPos.x - charactersLeft * mCharWidth;
         float y = caretPos.y + mCharHeight * (i + 2) - 2;
         if (i == mAutocompleteHighlightIndex)
            ofSetColor(jediIndexBg);
         else
            ofSetColor(jediBg);
         ofRect(x, y - mCharHeight + 2, gFontFixedWidth.GetStringWidth(mAutocompletes[i].autocompleteFull, mFontSize), mCharHeight);

         ofSetColor(jediAutoComplete);
         gFontFixedWidth.DrawString(mAutocompletes[i].autocompleteFull, mFontSize, x, y);
         ofSetColor(jediAutoCompleteRest);
         std::string prefix = "";
         for (size_t j = 0; j < charactersLeft; ++j)
            prefix += " ";
         gFontFixedWidth.DrawString(prefix + mAutocompletes[i].autocompleteRest, mFontSize, x, y);
      }
   }

   for (size_t i = 0; i < mAutocompleteSignatures.size(); ++i)
   {
      if (mAutocompleteSignatures[i].valid && mAutocompleteSignatures[i].params.size() > 0)
      {
         std::string params = "(";
         std::string highlightParamString = " ";
         for (size_t j = 0; j < mAutocompleteSignatures[i].params.size(); ++j)
         {
            std::string param = mAutocompleteSignatures[i].params[j];
            std::string placeholder = "";
            for (size_t k = 0; k < param.length(); ++k)
               placeholder += " ";
            if (j == mAutocompleteSignatures[i].entryIndex)
            {
               params += placeholder;
               highlightParamString += mAutocompleteSignatures[i].params[j];
            }
            else
            {
               params += mAutocompleteSignatures[i].params[j];
               highlightParamString += placeholder;
            }

            if (j < mAutocompleteSignatures[i].params.size() - 1)
            {
               params += ", ";
               highlightParamString += "  ";
            }
         }
         params += ")";
         highlightParamString += " ";
         float x = GetLinePos(mAutocompleteCaretCoords.y, K(end), !K(published)).x + 10;
         float y = caretPos.y + mCharHeight * (i + 1) - 2;
         ofSetColor(jediBg);
         ofRect(x, y - mCharHeight + 2, gFontFixedWidth.GetStringWidth(params, mFontSize), mCharHeight + 2);
         ofSetColor(jediParams);
         gFontFixedWidth.DrawString(params, mFontSize, x, y);
         ofSetColor(jediParamsHighlight);
         gFontFixedWidth.DrawString(highlightParamString, mFontSize, x, y);
      }
   }
}

std::string CodeEntry::GetVisibleCode()
{
   std::string visible;
   std::vector<std::string> lines = GetLines(false);
   if (lines.empty())
      return "";

   int firstVisibleLine = -1;
   int lastVisibleLine = -1;

   for (int i = 0; i < (int)lines.size(); ++i)
   {
      if ((i + 1) * mCharHeight >= mScroll.y && i * mCharHeight <= mScroll.y + mHeight)
      {
         if (firstVisibleLine == -1)
            firstVisibleLine = i;
         lastVisibleLine = i;
      }
   }

   while (firstVisibleLine > 0)
   {
      if (lines[firstVisibleLine][0] != ' ')
         break;
      --firstVisibleLine;
   }

   for (int i = 0; i < (int)lines.size(); ++i)
   {
      if (i >= firstVisibleLine && i <= lastVisibleLine)
         visible += lines[i] + "\n";
      else
         visible += "\n";
   }
   return visible;
}

void CodeEntry::DrawSyntaxHighlight(std::string input, ofColor color, std::vector<int> mapping, int filter1, int filter2)
{
   std::string filtered = FilterText(input, mapping, filter1, filter2);
   ofSetColor(color, gModuleDrawAlpha);

   float shake = (1 - ofClamp((gTime - mLastPublishTime) / 150, 0, 1)) * 3.0f;
   if (TheSynth->IsAudioPaused())
      shake = 0;
   float offsetX = ofRandom(-shake, shake);
   float offsetY = ofRandom(-shake, shake);

   gFontFixedWidth.DrawString(filtered, mFontSize, mX + 2 - mScroll.x + offsetX, mY + mCharHeight - mScroll.y + offsetY);
}

std::string CodeEntry::FilterText(std::string input, std::vector<int> mapping, int filter1, int filter2)
{
   for (size_t i = 0; i < input.size(); ++i)
   {
      if (input[i] != '\n')
      {
         if (i < mapping.size())
         {
            if (mapping[i] != filter1 && mapping[i] != filter2)
               input[i] = ' ';
         }
         else
         {
            bool showLeftovers = (filter1 == -1 && filter2 == -1);
            if (!showLeftovers)
               input[i] = ' ';
         }
      }
   }
   return input;
}

//static
void CodeEntry::OnPythonInit()
{
   const std::string syntaxHighlightCode = R"(def syntax_highlight_basic():
   #this uses the built in lexer/tokenizer in python to identify part of code
   #will return a meaningful lookuptable for index colours per character
   import tokenize
   import io
   import token
   import sys

   isPython3 = False
   if sys.version_info[0] >= 3:
      isPython3 = True

   if isPython3:
      text = str(syntax_highlight_code)
   else:
      text = unicode(syntax_highlight_code)

   tok_name = token.tok_name
   #print(tok_name) #   <--- dict of token-kinds.
   
   output = []
   defined = []
   lastCharEnd = 0
   lastRowEnd = 0

   with io.StringIO(text) as f:
       try:
          tokens = tokenize.generate_tokens(f.readline)

          for token in tokens:
              #print(token)
              if token.type in (0, 5, 56, 256):
                  continue
              if not token.string or (token.start == token.end):
                  continue

              token_type = token.type
              
              if token.type == 1:
                  if token.string in {'print', 'def', 'class', 'break', 'continue', 'return', 'while', 'or', 'and', 'dir', 'if', 'elif', 'else', 'is', 'in', 'as', 'out', 'with', 'from', 'import', 'with', 'for'}:
                      token_type = 90
                  elif token.string in {'False', 'True', 'yield', 'repr', 'range', 'enumerate', 'len', 'type', 'list', 'tuple', 'int', 'str', 'float'}:
                      token_type = 91
                  elif token.string in globals() or token.string in defined:
                      token_type = 92
                  #else:
                  #    defined.append(token.string) don't do this one, it makes for confusing highlighting
              elif isPython3 and token.type == 54:
                  # OPS
                  # 7: 'LPAR', 8: 'RPAR'
                  # 9: 'LSQB', 10: 'RSQB'
                  # 25: 'LBRACE', 26: 'RBRACE'
                  if token.exact_type in {7, 8, 9, 10, 25, 26}:
                      token_type = token.exact_type
                  else:
                      token_type = 51
              elif token.type == 51:
                  # OPS
                  # 7: 'LPAR', 8: 'RPAR
                  # 9: 'LSQB', 10: 'RSQB'
                  # 25: 'LBRACE', 26: 'RBRACE'
                  if token.string in {'(', ')'}:
                     token_type = 7
                  elif token.string in {'[', ']'}:
                     token_type = 9
                  elif token.string in {'{', '}'}:
                    token_type = 25
                  elif token.string in {'='}:
                    token_type = 22
                  elif token.string in {','}:
                    token_type = 12
              elif isPython3 and tok_name[token.type] == 'COMMENT':
                 token_type = 53

              if not token_type in [3, 2, 1, 90, 91, 92, 22, 7, 8, 25, 26, 9, 10, 51, 12, 53, 52, 59]: #this list matches the list of matches we use for the DrawSyntaxHighlight() calls
                 token_type = -1

              row_start, char_start = token.start[0]-1, token.start[1]
              row_end, char_end = token.end[0]-1, token.end[1]
              if lastRowEnd != row_end:
                 lastCharEnd = 0

              output = output + [99]*(char_start - lastCharEnd) + [token_type]*(char_end - char_start)
              lastCharEnd = char_end
              lastRowEnd = row_end

              #print(token_type)
       except Exception as e:
          exceptionText = str(e)
          if not 'EOF' in exceptionText:
            print("exception when syntax highlighting: "+exceptionText)
          pass
           

   #print(output)
   return output)";

   try
   {
      py::exec(syntaxHighlightCode, py::globals());
      sDoSyntaxHighlighting = true;
   }
   catch (const std::exception& e)
   {
      ofLog() << "syntax highlight initialization exception: " << e.what();
   }


   //autocomplete
   try
   {
      py::exec("import jedi", py::globals());

      try
      {
         //py::exec("jedi.preload_module(['bespoke','module','scriptmodule','random','math'])", py::globals());
         py::exec("jediProject = jedi.get_default_project()", py::globals());
         py::exec("jediProject.added_sys_path = [\"" + ofToResourcePath("python_stubs") + "\"]", py::globals());
         //py::eval_file(ofToResourcePath("bespoke_stubs.pyi"), py::globals());
         //py::exec("import sys;sys.path.append(\""+ ofToResourcePath("python_stubs")+"\")", py::globals());
         sDoPythonAutocomplete = true;
      }
      catch (const std::exception& e)
      {
         ofLog() << "autocomplete setup exception: " << e.what();
      }
   }
   catch (const std::exception& e)
   {
      ofLog() << "autocomplete initialization exception: " << e.what();
      ofLog() << "maybe jedi is not installed? if you want autocompletion, use \"python -m pip install jedi\" in your system console to install";
      sWarnJediNotInstalled = true;
   }
}

void CodeEntry::OnCodeUpdated()
{
   mCodeUpdated = true;
}

bool CodeEntry::IsAutocompleteShowing()
{
   if (!mWantToShowAutocomplete || mCaretPosition != mCaretPosition2 || mString == mPublishedString)
      return false;

   if (mAutocompletes[0].valid == false && mAutocompleteSignatures[0].valid == false)
      return false;

   if (mAutocompleteUpdateTimer <= 0)
   {
      ofVec2f coords = GetCaretCoords(mCaretPosition);
      return coords.x == mAutocompleteCaretCoords.x && coords.y == mAutocompleteCaretCoords.y;
   }

   return true;
}

void CodeEntry::SetError(bool error, int errorLine)
{
   mHasError = error;
   mErrorLine = errorLine;
}

void CodeEntry::OnClicked(float x, float y, bool right)
{
   if (right)
      return;

   float col = GetColForX(x);
   float row = GetRowForY(y);
   MoveCaret(GetCaretPosition(col, row));

   MakeActive();
}

void CodeEntry::MakeActive()
{
   SetActiveKeyboardFocus(this);
   mCaretBlink = true;
   mCaretBlinkTimer = 0;
}

namespace
{
   const int kTabSize = 3;
}

void CodeEntry::OnKeyPressed(int key, bool isRepeat)
{
   if (key == juce::KeyPress::backspaceKey)
   {
      if (mCaretPosition != mCaretPosition2)
      {
         RemoveSelectedText();
      }
      else if (mCaretPosition > 0)
      {
         MoveCaret(mCaretPosition - 1, false);
         if (mCaretPosition < mString.length() - 1)
            UpdateString(mString.substr(0, mCaretPosition) + mString.substr(mCaretPosition + 1));
         else
            UpdateString(mString.substr(0, mCaretPosition));
      }
   }
   else if (key == juce::KeyPress::deleteKey)
   {
      if (mCaretPosition != mCaretPosition2)
      {
         RemoveSelectedText();
      }
      else
      {
         if (mCaretPosition == 0)
            UpdateString(mString.substr(1));
         if (mCaretPosition < mString.length() - 1)
            UpdateString(mString.substr(0, mCaretPosition) + mString.substr(mCaretPosition + 1));
      }
   }
   else if (key == OF_KEY_TAB)
   {
      if (mCaretPosition != mCaretPosition2 || (GetKeyModifiers() & kModifier_Shift))
      {
         ShiftLines(GetKeyModifiers() & kModifier_Shift);
      }
      else
      {
         ofVec2f coords = GetCaretCoords(mCaretPosition);
         int spacesNeeded = kTabSize - (int)coords.x % kTabSize;
         std::string tab;
         for (int i = 0; i < spacesNeeded; ++i)
            tab += " ";
         AddString(tab);
      }
   }
   else if (key == ']' && GetKeyModifiers() == kModifier_Command)
   {
      ShiftLines(false);
   }
   else if (key == '[' && GetKeyModifiers() == kModifier_Command)
   {
      ShiftLines(true);
   }
   else if (key == OF_KEY_ESC)
   {
      if (IsAutocompleteShowing())
      {
         mWantToShowAutocomplete = false;
      }
      else
      {
         IKeyboardFocusListener::ClearActiveKeyboardFocus(K(notifyListeners));
         UpdateString(mPublishedString); //revert
         mCaretPosition2 = mCaretPosition;
      }
   }
   else if (key == OF_KEY_LEFT)
   {
      if (GetKeyModifiers() & kModifier_Command)
      {
         MoveCaretToStart();
      }
      else if (GetKeyModifiers() & kModifier_Alt)
      {
         MoveCaretToNextToken(true);
      }
      else
      {
         if (!(GetKeyModifiers() & kModifier_Shift) && mCaretPosition != mCaretPosition2)
            MoveCaret(MIN(mCaretPosition, mCaretPosition2));
         else if (mCaretPosition > 0)
            MoveCaret(mCaretPosition - 1);
      }
   }
   else if (key == OF_KEY_RIGHT)
   {
      if (GetKeyModifiers() & kModifier_Command)
      {
         MoveCaretToEnd();
      }
      else if (GetKeyModifiers() & kModifier_Alt)
      {
         MoveCaretToNextToken(false);
      }
      else
      {
         if (!(GetKeyModifiers() & kModifier_Shift) && mCaretPosition != mCaretPosition2)
            MoveCaret(MAX(mCaretPosition, mCaretPosition2));
         else if (mCaretPosition < mString.length())
            MoveCaret(mCaretPosition + 1);
      }
   }
   else if (key == OF_KEY_UP)
   {
      if (IsAutocompleteShowing() && mAutocompleteHighlightIndex > 0)
      {
         --mAutocompleteHighlightIndex;
      }
      else
      {
         ofVec2f coords = GetCaretCoords(mCaretPosition);
         if (coords.y > 0)
            --coords.y;
         MoveCaret(GetCaretPosition(coords.x, coords.y));
      }
   }
   else if (key == OF_KEY_DOWN)
   {
      if (IsAutocompleteShowing() && (mAutocompleteHighlightIndex + 1 < mAutocompletes.size() && mAutocompletes[mAutocompleteHighlightIndex].valid))
      {
         ++mAutocompleteHighlightIndex;
      }
      else
      {
         ofVec2f coords = GetCaretCoords(mCaretPosition);
         ++coords.y;
         MoveCaret(GetCaretPosition(coords.x, coords.y));
      }
   }
   else if (key == OF_KEY_RETURN)
   {
      if (IsAutocompleteShowing())
      {
         AcceptAutocompletion();
      }
      else
      {
         if (mCaretPosition != mCaretPosition2)
            RemoveSelectedText();

         ofVec2f coords = GetCaretCoords(mCaretPosition);
         int lineNum = (int)round(coords.y);
         auto lines = GetLines(false);
         int numSpaces = 0;
         if (mCaretPosition > 0 && mString[mCaretPosition - 1] == ':') //auto-indent
            numSpaces += kTabSize;
         if (lineNum < (int)lines.size())
         {
            for (int i = 0; i < lines[lineNum].length(); ++i)
            {
               if (lines[lineNum][i] == ' ')
                  ++numSpaces;
               else
                  break;
            }
         }
         std::string tab = "\n";
         for (int i = 0; i < numSpaces; ++i)
            tab += ' ';
         AddString(tab);
      }
   }
   else if (toupper(key) == 'V' && GetKeyModifiers() == kModifier_Command)
   {
      if (mCaretPosition != mCaretPosition2)
         RemoveSelectedText();

      juce::String clipboard = TheSynth->GetTextFromClipboard();
      AddString(clipboard.toStdString());
   }
   else if (toupper(key) == 'V' && (GetKeyModifiers() == (kModifier_Command | kModifier_Shift)))
   {
      IClickable* pasteName = nullptr;
      if (gHoveredUIControl != nullptr)
         pasteName = gHoveredUIControl;
      else if (gHoveredModule != nullptr)
         pasteName = gHoveredModule;
      if (pasteName != nullptr)
      {
         if (mCaretPosition != mCaretPosition2)
            RemoveSelectedText();

         std::string insert = pasteName->Path();
         AddString(insert);
      }
   }
   else if ((toupper(key) == 'C' || toupper(key) == 'X') && GetKeyModifiers() == kModifier_Command)
   {
      if (mCaretPosition != mCaretPosition2)
      {
         int caretStart = MIN(mCaretPosition, mCaretPosition2);
         int caretEnd = MAX(mCaretPosition, mCaretPosition2);
         TheSynth->CopyTextToClipboard(mString.substr(caretStart, caretEnd - caretStart));

         if (toupper(key) == 'X')
            RemoveSelectedText();
      }
   }
   else if (toupper(key) == 'Z' && GetKeyModifiers() == kModifier_Command)
   {
      Undo();
   }
   else if (toupper(key) == 'Z' && (GetKeyModifiers() == (kModifier_Command | kModifier_Shift)))
   {
      Redo();
   }
   else if (toupper(key) == 'A' && GetKeyModifiers() == kModifier_Command)
   {
      mCaretPosition = 0;
      mCaretPosition2 = (int)mString.size();
   }
   else if (key == juce::KeyPress::endKey)
   {
      MoveCaretToEnd();
   }
   else if (key == juce::KeyPress::homeKey)
   {
      MoveCaretToStart();
   }
   else if (toupper(key) == 'R' && GetKeyModifiers() == kModifier_Command)
   {
      Publish();
      mListener->ExecuteCode();
   }
   else if (toupper(key) == 'R' && GetKeyModifiers() == (kModifier_Command | kModifier_Shift))
   {
      Publish();
      int lineStart = MIN(GetCaretCoords(mCaretPosition).y, GetCaretCoords(mCaretPosition2).y);
      int lineEnd = MAX(GetCaretCoords(mCaretPosition).y, GetCaretCoords(mCaretPosition2).y);
      std::pair<int, int> ranLines = mListener->ExecuteBlock(lineStart, lineEnd);
      mLastPublishedLineStart = ranLines.first;
      mLastPublishedLineEnd = ranLines.second;
   }
   else if (key < CHAR_MAX)
   {
      AddCharacter((char)key);
   }
}

void CodeEntry::AcceptAutocompletion()
{
   AddString(mAutocompletes[mAutocompleteHighlightIndex].autocompleteRest);
   mAutocompleteUpdateTimer = 0;
   mWantToShowAutocomplete = false;
}

void CodeEntry::Publish()
{
   mPublishedString = mString;
   mLastPublishTime = gTime;
   mLastPublishedLineStart = 0;
   mLastPublishedLineEnd = (int)GetLines(true).size();
   OnCodeUpdated();
}

void CodeEntry::Undo()
{
   if (mUndosLeft > 0)
   {
      int size = (int)mUndoBuffer.size();
      mUndoBufferPos = (mUndoBufferPos - 1 + size) % size;
      --mUndosLeft;
      mRedosLeft = MIN(mRedosLeft + 1, size);

      mString = mUndoBuffer[mUndoBufferPos].mString;
      mCaretPosition = mUndoBuffer[mUndoBufferPos].mCaretPos;
      mCaretPosition2 = mUndoBuffer[mUndoBufferPos].mCaretPos;

      OnCodeUpdated();
   }
}

void CodeEntry::Redo()
{
   if (mRedosLeft > 0)
   {
      int size = (int)mUndoBuffer.size();
      mUndoBufferPos = (mUndoBufferPos + 1) % size;
      --mRedosLeft;
      mUndosLeft = MIN(mUndosLeft + 1, size);

      mString = mUndoBuffer[mUndoBufferPos].mString;
      mCaretPosition = mUndoBuffer[mUndoBufferPos].mCaretPos;
      mCaretPosition2 = mUndoBuffer[mUndoBufferPos].mCaretPos;

      OnCodeUpdated();
   }
}

void CodeEntry::UpdateString(std::string newString)
{
   mUndoBuffer[mUndoBufferPos].mCaretPos = mCaretPosition;

   mString = newString;

   int size = (int)mUndoBuffer.size();
   mRedosLeft = 0;
   mUndosLeft = MIN(mUndosLeft + 1, size);
   mUndoBufferPos = (mUndoBufferPos + 1) % size;
   mUndoBuffer[mUndoBufferPos].mString = mString;

   OnCodeUpdated();
}

void CodeEntry::AddCharacter(char c)
{
   if (AllowCharacter(c))
   {
      std::string s;
      s += c;
      AddString(s);

      mAutocompleteUpdateTimer = .2f;
   }
}

void CodeEntry::AddString(std::string s)
{
   if (mCaretPosition != mCaretPosition2)
      RemoveSelectedText();

   std::string toAdd;
   for (int i = 0; i < s.size(); ++i)
   {
      if (AllowCharacter(s[i]))
         toAdd += s[i];
   }

   UpdateString(mString.substr(0, mCaretPosition) + toAdd + mString.substr(mCaretPosition));
   MoveCaret(mCaretPosition + (int)toAdd.size(), false);
}

bool CodeEntry::AllowCharacter(char c)
{
   if (c == '\n')
      return true;
   return juce::CharacterFunctions::isPrintable(c);
}

void CodeEntry::RemoveSelectedText()
{
   int caretStart = MIN(mCaretPosition, mCaretPosition2);
   int caretEnd = MAX(mCaretPosition, mCaretPosition2);
   UpdateString(mString.substr(0, caretStart) + mString.substr(caretEnd));
   MoveCaret(caretStart, false);
}

void CodeEntry::ShiftLines(bool backwards)
{
   int caretStart = MIN(mCaretPosition, mCaretPosition2);
   int caretEnd = MAX(mCaretPosition, mCaretPosition2);
   ofVec2f coordsStart = GetCaretCoords(caretStart);
   ofVec2f coordsEnd = GetCaretCoords(caretEnd);

   auto lines = GetLines(false);
   std::string newString = "";
   for (size_t i = 0; i < lines.size(); ++i)
   {
      if (i >= coordsStart.y && i <= coordsEnd.y)
      {
         int numSpaces = 0;
         for (size_t j = 0; j < lines[i].size(); ++j)
         {
            if (lines[i][j] != ' ')
               break;
            ++numSpaces;
         }

         if (backwards)
         {
            int charsToRemove = numSpaces % kTabSize;
            if (charsToRemove == 0)
               charsToRemove = kTabSize;
            if (charsToRemove > numSpaces)
               charsToRemove = numSpaces;
            lines[i] = lines[i].substr(charsToRemove);
            if (i == coordsStart.y)
               caretStart = (int)newString.size() + numSpaces - charsToRemove;
            if (i == coordsEnd.y)
               caretEnd = (int)newString.size() + (int)lines[i].size();
         }
         else
         {
            int spacesNeeded = kTabSize - (int)numSpaces % kTabSize;
            for (int j = 0; j < spacesNeeded; ++j)
               lines[i] = " " + lines[i];
            if (i == coordsStart.y)
               caretStart = (int)newString.size() + numSpaces + spacesNeeded;
            if (i == coordsEnd.y)
               caretEnd = (int)newString.size() + (int)lines[i].size();
         }
      }
      newString += lines[i] + "\n";
   }
   UpdateString(newString);

   mCaretPosition = caretStart;
   mCaretPosition2 = caretEnd;
}

void CodeEntry::MoveCaret(int pos, bool allowSelection /*=true*/)
{
   mCaretPosition = pos;
   if (!allowSelection || !(GetKeyModifiers() & kModifier_Shift))
      mCaretPosition2 = mCaretPosition;
   mCaretBlink = true;
   mCaretBlinkTimer = 0;
}

void CodeEntry::MoveCaretToStart()
{
   ofVec2f coords = GetCaretCoords(mCaretPosition);
   auto lines = GetLines(false);
   int x = 0;
   if (coords.y < lines.size())
   {
      for (size_t i = 0; i < lines[coords.y].size(); ++i)
      {
         if (lines[coords.y][i] == ' ')
            ++x;
         else
            break;
      }
   }
   if (x == coords.x)
      x = 0;
   MoveCaret(GetCaretPosition(x, coords.y));
}

void CodeEntry::MoveCaretToEnd()
{
   ofVec2f coords = GetCaretCoords(mCaretPosition);
   MoveCaret(GetCaretPosition(9999, coords.y));
}

void CodeEntry::MoveCaretToNextToken(bool backwards)
{
   //eh... just move it more for now
   ofVec2f coords = GetCaretCoords(mCaretPosition);
   int amount = 5;
   if (backwards)
      amount *= -1;
   MoveCaret(GetCaretPosition(MAX(0, coords.x + amount), coords.y));
}

bool CodeEntry::MouseMoved(float x, float y)
{
   mHovered = TestHover(x, y);
   CheckHover(x, y);
   return false;
}

bool CodeEntry::MouseScrolled(float x, float y, float scrollX, float scrollY, bool isSmoothScroll, bool isInvertedScroll)
{
   if (fabs(scrollX) > fabsf(scrollY))
      scrollY = 0;
   else
      scrollX = 0;

   mScroll.x = MAX(mScroll.x - scrollX * 10, 0);
   mScroll.y = MAX(mScroll.y - scrollY * 10, 0);

   OnCodeUpdated();

   mWantToShowAutocomplete = false;

   return false;
}

void CodeEntry::SetDimensions(float width, float height)
{
   mWidth = width;
   mHeight = height;
   OnCodeUpdated();
}

int CodeEntry::GetCaretPosition(int col, int row)
{
   auto lines = GetLines(false);
   int caretPos = 0;
   for (size_t i = 0; i < row && i < lines.size(); ++i)
      caretPos += lines[i].length() + 1;

   if (row >= 0 && row < (int)lines.size())
      caretPos += MIN(col, lines[row].length());

   return MIN(caretPos, (int)mString.length());
}

int CodeEntry::GetColForX(float x)
{
   x -= 2;

   x += mScroll.x;

   if (x < 0)
      x = 0;

   return round(x / mCharWidth);
}

int CodeEntry::GetRowForY(float y)
{
   y -= 2;

   y += mScroll.y;

   if (y < 0)
      y = 0;

   return int(y / mCharHeight);
}

ofVec2f CodeEntry::GetLinePos(int lineNum, bool end, bool published /*= true*/)
{
   float x = mX - mScroll.x;
   float y = lineNum * mCharHeight + mY - mScroll.y;

   if (end)
   {
      std::string str = published ? mPublishedString : mString;
      auto lines = ofSplitString(str, "\n");
      if (lineNum < (int)lines.size())
         x += lines[lineNum].length() * mCharWidth;
   }

   return ofVec2f(x, y);
}

ofVec2f CodeEntry::GetCaretCoords(int caret)
{
   ofVec2f coords;
   int caretRemaining = caret;
   auto lines = GetLines(false);
   for (size_t i = 0; i < lines.size(); ++i)
   {
      if (caretRemaining >= lines[i].length() + 1)
      {
         caretRemaining -= lines[i].length() + 1;
         ++coords.y;
      }
      else
      {
         coords.x = caretRemaining;
         break;
      }
   }
   return coords;
}

namespace
{
   const int kSaveStateRev = 0;
}

void CodeEntry::SaveState(FileStreamOut& out)
{
   out << kSaveStateRev;

   out << mString;
}

void CodeEntry::LoadState(FileStreamIn& in, bool shouldSetValue)
{
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);

   std::string var;
   in >> var;
   if (shouldSetValue)
   {
      UpdateString(var);

      //Publish();
      //if (mListener != nullptr)
      //   mListener->ExecuteCode(mString);
   }
}

void CodeEntry::SetStyleFromJSON(const ofxJSONElement& vdict)
{
   auto fs = vdict.get("font-size", 14);
   auto fsi = fs.asInt();
   if (fsi > 4 && fsi < 200)
      mFontSize = fsi - 2;

   auto fromRGB = [vdict](const std::string& key, ofColor& onto)
   {
      auto def = Json::Value(Json::arrayValue);
      def[0u] = 255;
      def[1u] = 0;
      def[2u] = 0;
      auto arr = vdict.get(key, def);
      if (def.size() < 3)
      {
         onto.r = 255;
         onto.g = 0;
         onto.b = 0;
      }
      else
      {
         try
         {
            onto.r = arr[0u].asInt();
            onto.g = arr[1u].asInt();
            onto.b = arr[2u].asInt();

            if (def.size() > 3)
               onto.a = arr[3u].asInt();
         }
         catch (Json::LogicError& e)
         {
            TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
         }
      }
   };
   fromRGB("currentBg", currentBg);
   fromRGB("publishedBg", publishedBg);
   fromRGB("unpublishedBg", unpublishedBg);
   fromRGB("string", stringColor);
   fromRGB("number", numberColor);
   fromRGB("name1", name1Color);
   fromRGB("name2", name2Color);
   fromRGB("name3", name3Color);
   fromRGB("defined", definedColor);
   fromRGB("equals", equalsColor);
   fromRGB("paren", parenColor);
   fromRGB("brace", braceColor);
   fromRGB("bracket", bracketColor);
   fromRGB("op", opColor);
   fromRGB("comma", commaColor);
   fromRGB("comment", commentColor);
   fromRGB("selectedOverlay", selectedOverlay);

   fromRGB("jediBg", jediBg);
   fromRGB("jediIndexBg", jediIndexBg);
   fromRGB("jediAutoComplete", jediAutoComplete);
   fromRGB("jediAutoCompleteRest", jediAutoCompleteRest);
   fromRGB("jediParams", jediParams);
   fromRGB("jediParamsHighlight", jediParamsHighlight);
}
