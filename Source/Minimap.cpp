#include "Minimap.h"
#include "ModularSynth.h"
#include "OpenFrameworksPort.h"

const float MAX_LENGTH = 150;
const float MARGIN_RIGHT = 15;
const float MARGIN_BOTTOM = 15;
const float BOOKMARK_SIZE = 15;
const float NUM_BOOKMARKS = 9;

Minimap::Minimap()
   : mClick(false)
   , mGrid(nullptr)
{}

Minimap::~Minimap() {}

void Minimap::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mGrid = new UIGrid(0, 0, MAX_LENGTH, BOOKMARK_SIZE, NUM_BOOKMARKS, 1, this);
}

void Minimap::GetDimensions(float& width, float& height)
{
   float windowWidth = ofGetWidth();
   float windowHeight = ofGetHeight();
   float ratio = windowWidth / windowHeight;

   if (ofGetWidth() > ofGetHeight())
   {
      width = MAX_LENGTH;
      height = (MAX_LENGTH / ratio) + BOOKMARK_SIZE;
   }
   else
   {
      height = MAX_LENGTH;
      width = (MAX_LENGTH * ratio) + BOOKMARK_SIZE;
   }
}

void Minimap::GetDimensionsMinimap(float& width, float& height){
   GetDimensions(width, height);
   if (width < height)
   {
      width -= BOOKMARK_SIZE;
   }
   else
   {
      height -= BOOKMARK_SIZE;
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

   if (width < height)
   {
      mGrid->SetDimensions(BOOKMARK_SIZE, height);
      mGrid->SetPosition(width - BOOKMARK_SIZE, 0);
      mGrid->SetGrid(1, NUM_BOOKMARKS);
   }
   else
   {
      mGrid->SetDimensions(width, BOOKMARK_SIZE);
      mGrid->SetPosition(0, height - BOOKMARK_SIZE);
      mGrid->SetGrid(NUM_BOOKMARKS, 1);
   }
   ofPushStyle();
   ofSetColor(255, 255, 255, 80);
    ofRect(CoordsToMinimap(boundingBox, viewport));
   ofSetColor(255, 255, 255, 10);
   ofFill();
    ofRect(CoordsToMinimap(boundingBox, viewport));
   ofPopStyle();
   mGrid->Draw();
}

void Minimap::UpdateGridValues()
{
   UpdateGridValues(0, 0);
}

void Minimap::UpdateGridValues(float x, float y)
{
   mGrid->Clear();
   for (int i = 0; i < mGrid->GetRows() * mGrid->GetCols(); ++i)
   {
      float val = 0.2;
      if (TheSynth->GetLocationZoomer()->HasLocation(i + 49))
         val = .5f;
      mGrid->SetVal(i % mGrid->GetCols(), i / mGrid->GetCols(), val);
   }
   float gridX, gridY;
   mGrid->GetPosition(gridX, gridY, true);
   if (mGrid->TestHover(x - gridX, y - gridY))
   {
      GridCell cell = mGrid->GetGridCellAt(x - gridX, y - gridY);
      mGrid->SetVal(cell.mCol, cell.mRow, .8f);
   }
}

void Minimap::OnClicked(int x, int y, bool right)
{
   if (mGrid->TestClick(x, y, right, true))
   {
      float gridX, gridY;
      mGrid->GetPosition(gridX, gridY, true);
      GridCell cell = mGrid->GetGridCellAt(x - gridX, y - gridY);
      int number = cell.mCol + cell.mRow + 49; // We assume a single row or column always so adding is fine.
      if (GetKeyModifiers() == kModifier_Shift)
         TheSynth->GetLocationZoomer()->WriteCurrentLocation(number);
      else
         TheSynth->GetLocationZoomer()->MoveToLocation(number);
      UpdateGridValues();
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
   UpdateGridValues(x, y);
   return false;
}

void Minimap::ForcePosition()
{
   float width, height, scale;
   scale = 1 / TheSynth->GetUIScale();
   GetDimensions(width, height);
   mX = (ofGetWidth() * scale) - (width + MARGIN_RIGHT);
   mY = (ofGetHeight() * scale) - (height + MARGIN_BOTTOM);
   gDrawScale = gDrawScale;
}
