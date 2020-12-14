#ifndef __bpButtons_h__
#define __bpButtons_h__


class bpButtons
{
    public:

        void setup();
        void run();

    private:

        u8       m_state;
        uint32_t m_poll_time;
        uint32_t m_time;
        int      m_repeat_count;
        bool     m_handled;
};


extern bpButtons buttons;


#endif  // __bpButtons_h__