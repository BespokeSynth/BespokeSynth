#include "Minimap.h"
#include "ModularSynth.h"
#include "OpenFrameworksPort.h"

namespace
{
   const float kMaxLength = 150;
   const float kMarginRight = 10;
   const float kMarginTop = 45;
   const float kBookmarkSize = 15;
   const float kNumBookmarks = 9;
}

Minimap::Minimap()
   : mClick(false)
   , mGrid(nullptr)
   , mHoveredBookmarkPos(-1, -1)
{
}

Minimap::~Minimap()
{
}

void Minimap::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGrid = new UIGrid(0, 0, kMaxLength, kBookmarkSize, kNumBookmarks, 1, this);
}

void Minimap::GetDimensions(float& width, float& height)
{
   float windowWidth = ofGetWidth();
   float windowHeight = ofGetHeight();
   float ratio = windowWidth / windowHeight;

   if (ofGetWidth() > ofGetHeight())
   {
      width = kMaxLength;
      height = (kMaxLength / ratio) + kBookmarkSize;
   }
   else
   {
      height = kMaxLength;
      width = (kMaxLength * ratio) + kBookmarkSize;
   }
}

void Minimap::GetDimensionsMinimap(float& width, float& height)
{
   GetDimensions(width, height);
   if (width < height)
   {
      width -= kBookmarkSize;
   }
   else
   {
      height -= kBookmarkSize;
   }
}

void Minimap::ComputeBoundingBox(ofRectangle& rect)
{
   std::vector<IDrawableModule*> modules;
   TheSynth->GetRootContainer()->GetAllModules(modules);

   if (modules.empty())
   {
      rect = TheSynth->GetDrawRect();
      return;
   }

   rect = modules[0]->GetRect();

   for (int i = 1; i < modules.size(); ++i)
   {
      if (!modules[i]->IsShowing())
      {
         continue;
      }
      ofRectangle moduleRect = modules[i]->GetRect();
      RectUnion(rect, moduleRect);
   }
   
   float minimapWidth, minimapHeight;
   GetDimensionsMinimap(minimapWidth, minimapHeight);
   float boundsAspectRatio = rect.width / rect.height;
   float minimapAspectRatio = minimapWidth / minimapHeight;
   //retain aspect ratio
   if (boundsAspectRatio > minimapAspectRatio)
      rect.height = rect.width / minimapAspectRatio;
   else
      rect.width = rect.height * minimapAspectRatio;
}

ofRectangle Minimap::CoordsToMinimap(ofRectangle& boundingBox, ofRectangle& source)
{
   float width;
   float height;
   GetDimensionsMinimap(width, height);

   float x1 = (source.getMinX() - boundingBox.x) / boundingBox.width * width;
   float y1 = (source.getMinY() - boundingBox.y) / boundingBox.height * height;
   float x2 = (source.getMaxX() - boundingBox.x) / boundingBox.width * width;
   float y2 = (source.getMaxY() - boundingBox.y) / boundingBox.height * height;

   return {x1, y1, x2 - x1, y2 - y1};
}

ofVec2f Minimap::CoordsToViewport(ofRectangle& boundingBox, float x, float y)
{
   float width;
   float height;
   GetDimensionsMinimap(width, height);

   float x1 = x / width * boundingBox.width + boundingBox.x;
   float y1 = y / height * boundingBox.height + boundingBox.y;

   return {x1, y1};
}

void Minimap::DrawModulesOnMinimap(ofRectangle& boundingBox)
{
   std::vector<IDrawableModule*> modules;
   TheSynth->GetRootContainer()->GetAllModules(modules);

   ofPushStyle();
   for (int i = 0; i < modules.size(); ++i)
   {
      if (!modules[i]->IsShowing())
      {
         continue;
      }
      ofRectangle moduleRect = modules[i]->GetRect();
      ofColor moduleColor(IDrawableModule::GetColor(modules[i]->GetModuleType()));
      ofSetColor(moduleColor);
      ofFill();
      ofRect(CoordsToMinimap(boundingBox, moduleRect));
   }
   ofPopStyle();
}

void Minimap::RectUnion(ofRectangle& target, ofRectangle& unionRect)
{
   float x2 = target.getMaxX();
   float y2 = target.getMaxY();
   if (target.x > unionRect.x)
   {
      target.x = unionRect.x;
      target.width = fabs(x2) - target.x;
   }

   if (target.y > unionRect.y)
   {
      target.y = unionRect.y;
      target.height = fabs(y2) - target.y;
   }

   if (target.getMaxX() < unionRect.getMaxX())
   {
      x2 = unionRect.getMaxX();
   }

   if (target.getMaxY() < unionRect.getMaxY())
   {
      y2 = unionRect.getMaxY();
   }

   target.width = fabs(x2) - target.x;
   target.height = fabs(y2) - target.y;
}

void Minimap::DrawModule()
{
   float width;
   float height;
   ofRectangle boundingBox;
   ofRectangle boundingBoxMM;
   ofRectangle viewport = TheSynth->GetDrawRect();
   ForcePosition();
   ComputeBoundingBox(boundingBox);
   GetDimensions(width, height);

   DrawModulesOnMinimap(boundingBox);

   for (int i = 0; i < mGrid->GetCols() * mGrid->GetRows(); ++i)
   {
      float val = 0.0f;
      if (TheSynth->GetLocationZoomer()->HasLocation(i + '1'))
         val = .5f;
      mGrid->SetVal(i % mGrid->GetCols(), i / mGrid->GetCols(), val);
   }
   
   if (width < height)
   {
      mGrid->SetDimensions(kBookmarkSize, height);
      mGrid->SetPosition(width - kBookmarkSize, 0);
      mGrid->SetGrid(1, kNumBookmarks);
   }
   else
   {
      mGrid->SetDimensions(width, kBookmarkSize);
      mGrid->SetPosition(0, height - kBookmarkSize);
      mGrid->SetGrid(kNumBookmarks, 1);
   }

   ofPushMatrix();
   float widthMM;
   float heightMM;
   GetDimensionsMinimap(widthMM, heightMM);
   ofClipWindow(0, 0, widthMM, heightMM, true);
   ofPushStyle();
   ofSetColor(255, 255, 255, 80);
   ofRect(CoordsToMinimap(boundingBox, viewport));
   ofSetColor(255, 255, 255, 10);
   ofFill();
   ofRect(CoordsToMinimap(boundingBox, viewport));
   ofPopStyle();
   ofPopMatrix();

   mGrid->Draw();

   if (mHoveredBookmarkPos.mCol != -1)
   {
      ofPushStyle();
      ofSetColor(255, 255, 255);
      ofFill();

      ofVec2f pos = mGrid->GetCellPosition(mHoveredBookmarkPos.mCol, mHoveredBookmarkPos.mRow) + mGrid->GetPosition(true);
      float xsize = float(mGrid->GetWidth()) / mGrid->GetCols();
      float ysize = float(mGrid->GetHeight()) / mGrid->GetRows();

      if (GetKeyModifiers() == kModifier_Shift)
      {
         ofRect(pos.x + xsize / 2 - 1, pos.y + 2, 2, ysize - 4, 0);
         ofRect(pos.x + 2, pos.y + ysize / 2 - 1, xsize - 4, 2, 0);
      }
      else
      {
         ofCircle(pos.x + xsize / 2, pos.y + ysize / 2, xsize / 2 - 2);
      }

      ofPopStyle();
   }
}

void Minimap::OnClicked(int x, int y, bool right)
{
   if (mGrid->TestClick(x, y, right, true))
   {
      float gridX, gridY;
      mGrid->GetPosition(gridX, gridY, true);
      GridCell cell = mGrid->GetGridCellAt(x - gridX, y - gridY);
      int number = cell.mCol + cell.mRow + '1';
      if (GetKeyModifiers() == kModifier_Shift)
         TheSynth->GetLocationZoomer()->WriteCurrentLocation(number);
      else
         TheSynth->GetLocationZoomer()->MoveToLocation(number);
   }
   else
   {
      ofRectangle boundingBox;
      ofRectangle viewport = TheSynth->GetDrawRect();
      ComputeBoundingBox(boundingBox);
      ofVec2f viewportCoords = CoordsToViewport(boundingBox, x, y);
      TheSynth->SetDrawOffset(ofVec2f(-viewportCoords.x + viewport.width / 2, -viewportCoords.y + viewport.height / 2));
      mClick = true;
   }
}

void Minimap::MouseReleased()
{
   mClick = false;
}

bool Minimap::MouseMoved(float x, float y)
{
   if (mClick)
   {
      ofRectangle boundingBox;
      ofRectangle viewport = TheSynth->GetDrawRect();
      ComputeBoundingBox(boundingBox);
      ofVec2f viewportCoords = CoordsToViewport(boundingBox, x, y);
      TheSynth->SetDrawOffset(ofVec2f(-viewportCoords.x + viewport.width / 2, -viewportCoords.y + viewport.height / 2));
   }
   mGrid->NotifyMouseMoved(x, y);

   float gridX, gridY;
   mGrid->GetPosition(gridX, gridY, true);
   if (mGrid->TestHover(x - gridX, y - gridY))
   {
      mHoveredBookmarkPos = mGrid->GetGridCellAt(x - gridX, y - gridY);
   }
   else
   {
      mHoveredBookmarkPos.mCol = -1;
      mHoveredBookmarkPos.mRow = -1;
   }

   return false;
}

void Minimap::ForcePosition()
{
   float width, height, scale;
   scale = 1 / TheSynth->GetUIScale();
   GetDimensions(width, height);
   mX = (ofGetWidth() * scale) - (width + kMarginRight);
   mY = kMarginTop;
}
