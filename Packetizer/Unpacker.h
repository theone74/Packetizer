#pragma once

#include "CRC.h"
#include <queue>

namespace Packetizer
{
    class Unpacker
    {
    public:

        Unpacker(Checker m = Checker::CRC8)
        : r_buffer(), state(State::Start), b_escape(false) , sum(0), count(0), mode(m)
        {}

        ~Unpacker() {}

        size_t available() { return _readBuffer.size(); }
        uint8_t index() { return _readBuffer.front().index; }
        uint8_t size() { return _readBuffer.front().size; }
        uint8_t* data() { return _readBuffer.front().sbuf.data(); }

        void setCheckMode(Checker m) { mode = m; }

        void pop() { _readBuffer.pop(); }

        void feed(const uint8_t* const data, const size_t size)
        {
            for (size_t i = 0; i < size; ++i) feed(data[i]);
        }

        void feed(uint8_t d)
        {
            uint16_t buff = append(d);
            if (buff == ESCAPE_MARKER) return;

            uint8_t data = (uint8_t)(buff & 0x00FF);

            if ((mode == Checker::Sum) && (state != State::Checksum)) sum += data;

            switch(state)
            {
                case State::Start:
                {
                    reset();
                    if (data == (uint8_t)START_BYTE) state = State::Index;
                    if (mode == Checker::Sum) sum += data;
                    break;
                }
                case State::Index:
                {
                    r_buffer.index = data;
                    state = State::Size;
                    break;
                }
                case State::Size:
                {
                    r_buffer.size = data;
                    state = State::Data;
                    break;
                }
                case State::Data:
                {
                    r_buffer.sbuf.push_back(data);
                    if (++count >= r_buffer.size) state = State::Checksum;
                    break;
                }
                case State::Checksum:
                {
                    if ((mode == Checker::Sum) && (sum == data))
                    {
                        _readBuffer.push(r_buffer);
                    }
                    else if (mode == Checker::CRC8)
                    {
                        uint8_t crc8 = CRC::getCRC8((uint8_t*)r_buffer.sbuf.data(), r_buffer.sbuf.size());
                        if (crc8 == data) _readBuffer.push(r_buffer);
                    }
                    reset();
                    break;
                }
                default:
                {
                    reset();
                    break;
                }
            }
        }

    protected:

        uint16_t append(const uint8_t& data)
        {
            uint16_t byteIn = (uint16_t)data;

            if (byteIn == (uint16_t)ESCAPE_BYTE)
            {
                byteIn = ESCAPE_MARKER;
                b_escape = true;
            }
            else
            {
                if (b_escape)
                {
                    byteIn = (uint16_t)(byteIn ^ ESCAPE_MASK);
                    b_escape = false;
                }
            }
            return byteIn;
        }

        void reset()
        {
            r_buffer.clear();
            sum = count = 0;
            state = State::Start;
        }

    protected:

        struct Buffer
        {
            uint8_t index;
            uint8_t size;
            std::vector<uint8_t> sbuf;

            void clear() { index = size = 0; sbuf.clear(); }
        };

        Buffer r_buffer;

        State state;
        bool b_escape;

        uint8_t sum;
        uint8_t count;

        Checker mode;
        std::queue<Buffer> _readBuffer;
    };

}
