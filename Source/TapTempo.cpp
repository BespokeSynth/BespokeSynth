#include "TapTempo.h"
#include "PatchCableSource.h"
#include "UIControlMacros.h"


TapTempo::TapTempo()
    : lastTapTime(0)
    , isFirstTap(true)
    , currentTempo(120)
    , maxIntervals(4)
    , minInterval(0.2)
    , maxInterval(2.0)
{
    // Инициализация
}

TapTempo::~TapTempo()
{
    // Очистка
}

void TapTempo::CreateUIControls()
{
    IDrawableModule::CreateUIControls(); // Вызов базового метода
    mTapButton = new ClickButton(this, "tap", 0, 0, ButtonDisplayStyle::kText);
}

void TapTempo::DrawModule()
{
    // Реализация отрисовки
    mTapButton->Draw();

    // задать обработчик событий для кнопки
    mTapButton->SetCableTargetable(true);
}

void TapTempo::ProcessTap()
{
    // Используем текущее время в секундах
    double currentTime = TheTransport->GetMeasureTime(gTime) / (TheTransport->GetTempo() / 60.0f);

    if (isFirstTap) {
        isFirstTap = false;
        lastTapTime = currentTime;
        return;
    }

    double timeDifference = currentTime - lastTapTime;
    
    // Проверяем, находится ли интервал в допустимом диапазоне
    if (timeDifference >= minInterval && timeDifference <= maxInterval) {
        // Добавляем ноый интервал
        intervals.push_back(timeDifference);
        
        // Удаляем старый интервал, если превышен максимальный размер
        if (intervals.size() > maxIntervals) {
            intervals.erase(intervals.begin());
        }
        
        // Вычисляем среднее значение интервалов
        float averageInterval = 0;
        for (float interval : intervals) {
            averageInterval += interval;
        }
        averageInterval /= intervals.size();
        
        // Вычисляем новый темп (BPM = 60/interval)
        currentTempo = 60.0f / averageInterval;
        
        TheTransport->SetTempo(currentTempo);
    }
    else if (timeDifference > maxInterval) {
        // Сброс при слишком большом интервале
        intervals.clear();
        isFirstTap = true;
    }
    
    lastTapTime = currentTime;
}


void TapTempo::ButtonClicked(ClickButton* button, double time)
{
   if (button == mTapButton)
   {
      ProcessTap();
   }
}
