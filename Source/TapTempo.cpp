#include "TapTempo.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"

TapTempo::TapTempo()
{
}

void TapTempo::Init()
{
    IDrawableModule::Init();
    TheTransport->AddListener(this, kInterval_32n, {0, 0}, true);
}

TapTempo::~TapTempo()
{
    TheTransport->RemoveListener(this);
}

void TapTempo::CreateUIControls()
{
    IDrawableModule::CreateUIControls();
    float width, height;
    GetModuleDimensions(width, height);
    
    mTapButton = new ClickButton(this, "tap", width/2 - 2, 18, ButtonDisplayStyle::kText);
    mTapButton->SetDimensions(width/2, 20);
    mTapButton->SetName("tap");
    AddUIControl(mTapButton);
    
    mJustFollowCheckbox = new Checkbox(this, "just follow", 5, 5, &mJustFollowMode);
    mJustFollowCheckbox->SetName("just_follow");
    AddUIControl(mJustFollowCheckbox);
}

void TapTempo::ProcessTap(double currentTime)
{
    mJustFollowMode = mJustFollowCheckbox->GetValue();
    
    if (mJustFollowMode)
    {
        if (tapCount > 0)
        {
            double interval = currentTime - lastTapTime;
            if (interval > 0 && interval <= MAX_INTERVAL)
            {
                float tempo = 60.0f / static_cast<float>(interval);
                if (tempo >= MIN_BPM && tempo <= MAX_BPM)
                {
                    TheTransport->SetTempo(tempo);
                }
            }
        }
        lastTapTime = currentTime;
        tapCount = 1;
        return;
    }
    
    if (tapCount == 0)
    {
        lastTapTime = currentTime;
        tapCount = 1;
        return;
    }
    
    double interval = currentTime - lastTapTime;
    
    if (interval > RESET_TIMEOUT)
    {
        ResetTapSequence();
        lastTapTime = currentTime;
        tapCount = 1;
        return;
    }
    
    if (!IsValidInterval(interval))
    {
        ResetTapSequence();
        return;
    }
    
    intervals.push_back(interval);
    while (intervals.size() > REQUIRED_TAPS - 1)
        intervals.pop_front();
        
    lastTapTime = currentTime;
    tapCount++;
    
    if (tapCount == REQUIRED_TAPS)
    {
        float tempo = CalculateAverageTempo();
        if (tempo >= MIN_BPM && tempo <= MAX_BPM)
        {
            double avgInterval = 60.0 / tempo;
            pendingTempoTime = currentTime + avgInterval;
            isWaitingForTempo = true;
        }
    }
    else if (tapCount > REQUIRED_TAPS)
    {
    }
}

bool TapTempo::IsValidInterval(double interval) const
{
    return interval >= MIN_INTERVAL && interval <= MAX_INTERVAL;
}

float TapTempo::CalculateAverageTempo() const
{
    if (intervals.empty())
        return TheTransport->GetTempo();
        
    double avgInterval = 0.0;
    for (double interval : intervals)
        avgInterval += interval;
    avgInterval /= intervals.size();
    
    return 60.0f / static_cast<float>(avgInterval);
}

void TapTempo::ResetTapSequence()
{
    tapCount = 0;
    intervals.clear();
    isWaitingForTempo = false;
    pendingTempoTime = 0.0;
}

void TapTempo::OnTimeEvent(double time)
{
    if (tapCount > 0 && (time - lastTapTime) > RESET_TIMEOUT)
    {
        ResetTapSequence();
    }
    
    if (isWaitingForTempo && time >= pendingTempoTime)
    {
        float tempo = CalculateAverageTempo();
        if (tempo >= MIN_BPM && tempo <= MAX_BPM)
        {
            TheTransport->SetTempo(tempo);
        }
        ResetTapSequence();
    }
}

void TapTempo::DrawModule()
{
    if (Minimized() || IsVisible() == false)
        return;

    mTapButton->Draw();
    
    float width, height;
    GetModuleDimensions(width, height);
    
    const float indicatorWidth = 10;
    const float indicatorHeight = 10;
    const float spacing = 5;
    const float startX = width/2 - ((indicatorWidth * REQUIRED_TAPS + spacing * (REQUIRED_TAPS-1)) / 2);
    const float y = 5;
    
    ofPushStyle();
    
    for (int i = 0; i < REQUIRED_TAPS; ++i)
    {
        if (isWaitingForTempo)
            ofSetColor(135, 135, 135, 150);
        else if (i < tapCount)
            ofSetColor(0, 135, 0, 150);
        else
            ofSetColor(0, 135, 0, 30);
            
        ofFill();
        ofRect(startX + i * (indicatorWidth + spacing), y, indicatorWidth, indicatorHeight);
    }
    
    ofPopStyle();
    
    std::string tempoDisplay = (tapCount == 0) ? "-" : 
                              ofToString(static_cast<int>(TheTransport->GetTempo()));
    DrawTextNormal(tempoDisplay, 5, 30);
}

void TapTempo::ButtonClicked(ClickButton* button, double time)
{
    if (button == mTapButton)
    {
        ProcessTap(time);
    }
}
