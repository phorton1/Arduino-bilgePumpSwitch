#include "myDebug.h"
#include "bpButtons.h"
#include "bpUI.h"

// buttons call bpUI::onButton() when pressed.
// buttons 1 and 2 implement long press auto-increment


#define dbg_buttons  1


#define PIN_BUTTON0          8
#define NUM_BUTTONS          3
    // the button pins are 10, 9, and 8
#define BUTTON_PIN(i)        (PIN_BUTTON0 + NUM_BUTTONS - i - 1)


#define POLL_INTERVAL       20  // ms
    // handles debouncing too ..


bpButtons buttons;


void bpButtons::setup()
{
    m_state = 0;
    m_poll_time = 0;
    m_time = 0;
    m_repeat_count = 0;

    for (int i=0; i<NUM_BUTTONS; i++)
        pinMode(PIN_BUTTON0+i,INPUT_PULLUP);
}

void bpButtons::run()
{
    uint32_t now = millis();
    if (now > m_poll_time + POLL_INTERVAL)
    {
        m_poll_time = now;
        for (int i=0; i<NUM_BUTTONS; i++)
        {
            u8 mask = 1<<i;
            u8 was_pressed = m_state & mask ? 1 : 0;
            u8 is_pressed = digitalRead(BUTTON_PIN(i)) ? 0 : 1;

            if (is_pressed != was_pressed)
            {
                m_time = now;
                m_repeat_count = 0;
                if (is_pressed)
                {
                    display(dbg_buttons,"BUTTON_PRESS(%d)",i);
                    m_state |= mask;
                    bpui.onButton(i);
                }
                else
                {
                    display(dbg_buttons,"BUTTON_RELEASE(%d)",i);
                    m_state &= ~mask;
                }
            }
            else if (i>0 && is_pressed)
            {
                #define START_REPEAT_DELAY   300    // ms
                int dif = now - m_time;
                if (dif > START_REPEAT_DELAY)
                {
                    // this will be called every 20 ms
                    // we want to scale the mod for repeat count
                    // from 10 down to 1 over 2 seconds

                    dif -= START_REPEAT_DELAY;      // 0..n
                    dif = 2000 - dif;               // 2000..n
                    if (dif < 0) dif = 0;           // 2000..0
                    dif = dif / 200;                // 10..0
                    if (dif == 0) dif = 1;          // 10..1

                    if (m_repeat_count % dif == 0)
                    {
                        display(dbg_buttons,"BUTTON_REPEAT(%d)",i);
                        bpui.onButton(i);
                    }

                    m_repeat_count++;
                }
            }
        }
    }
}
