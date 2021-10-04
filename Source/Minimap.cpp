#include "Minimap.h"
#include "ModularSynth.h"
#include "OpenFrameworksPort.h"

const float MAX_LENGTH = 150;
const float MARGIN_RIGHT = 20;
const float MARGIN_BOTTOM = 20;

Minimap::Minimap() {}

Minimap::~Minimap() {}

bool Minimap::AlwaysOnTop() { return true; }

bool Minimap::IsSingleton() const { return true; }

bool Minimap::HasTitleBar() const { return false; }

std::string Minimap::GetTitleLabel() { return ""; }

void Minimap::GetDimensions(float& width, float& height)
{
    float windowWidth = ofGetWidth();
    float windowHeight = ofGetHeight();
    float ratio = windowWidth / windowHeight;

    if (ofGetWidth() > ofGetHeight())
    {
        width = MAX_LENGTH;
        height = MAX_LENGTH / ratio;
    }
    else
    {
        height = MAX_LENGTH;
        width = MAX_LENGTH * ratio;
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
    GetDimensions(width, height);

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
    GetDimensions(width, height);

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
    ofRectangle viewport = TheSynth->GetDrawRect();
    ForcePosition();
    ComputeBoundingBox(boundingBox);
    GetDimensions(width, height);

    DrawModulesOnMinimap(boundingBox);

    ofPushStyle();
    ofSetColor(255, 255, 255, 80);
    ofRect(CoordsToMinimap(boundingBox, viewport));
    ofSetColor(255, 255, 255, 10);
    ofFill();
    ofRect(CoordsToMinimap(boundingBox, viewport));
    ofPopStyle();
}


void Minimap::OnClicked(int x, int y, bool right)
{
    ofRectangle boundingBox;
    ofRectangle viewport = TheSynth->GetDrawRect();
    ComputeBoundingBox(boundingBox);
    ofVec2f viewportCoords = CoordsToViewport(boundingBox, x, y);
    TheSynth->SetDrawOffset(ofVec2f(-viewportCoords.x + viewport.width / 2, -viewportCoords.y + viewport.height / 2));
}

void Minimap::ForcePosition()
{
    float width, height;
    GetDimensions(width, height);
    mX = ofGetWidth() - (width + MARGIN_RIGHT);
    mY = ofGetHeight() - (height + MARGIN_BOTTOM);
}
