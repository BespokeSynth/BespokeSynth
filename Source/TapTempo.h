#pragma once

// Сначала базовые системные включения
#include <vector>

// Затем включения проекта в алфавитном порядке
#include "ClickButton.h"
#include "Transport.h"
#include "IDrawableModule.h"
#include "INoteReceiver.h"

class TapTempo : public IDrawableModule, public INoteReceiver, public ITimeListener, public IButtonListener
{
public:
   TapTempo();
   virtual ~TapTempo();
   static IDrawableModule* Create() { return new TapTempo(); }
    // Методы IDrawableModule
    void CreateUIControls() override;
    void DrawModule() override;
    std::string GetTitleLabel() const override {
        return std::to_string(currentTempo) + " bpm";
    };

    // Методы INoteReceiver
    void OnNote(double time, int pitch, int velocity);
    void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override { ProcessTap(); }
    void SendCC(int control, int value, int voiceIdx = -1) override { ProcessTap(); }

    void ButtonClicked(ClickButton* button, double time) override;
    void OnTimeEvent(double time) override {}  // От ITimeListener
private:
    void ProcessTap();
    
    double lastTapTime;
    bool isFirstTap;
    float currentTempo;
    
    const int maxIntervals;
    const float minInterval;
    const float maxInterval;
    std::vector<float> intervals;

    ClickButton* mTapButton;
};
