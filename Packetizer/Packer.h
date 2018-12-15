#pragma once

#include <vector>
#include "CRC.h"

namespace Packetizer
{
    class Packer
    {
    public:

        Packer(Checker m = Checker::CRC8) : mode(m) {}
        ~Packer() {}

        void setCheckMode(Checker m) { mode = m; }

		uint8_t* data() { return pack_buffer.data(); }

        size_t size() { return pack_buffer.size(); }


        const uint8_t* pack(uint8_t* sbuf, uint8_t size, const uint8_t& index = 0)
        {
            pack_buffer.clear();

            append((uint8_t)START_BYTE, false);
            append((uint8_t)index);
            append((uint8_t)size);
            append((uint8_t*)sbuf, size);

            if (mode == Checker::Sum)
            {
                uint8_t sum = (uint8_t)START_BYTE + index + (uint8_t)size;
                for (size_t i = 0; i < size; ++i) sum += (uint8_t)sbuf[i];
                append(sum);
            }
            else if (mode == Checker::CRC8)
            {
                append(CRC::getCRC8((uint8_t*)sbuf, size));
            }

            return data();
        }

    protected:

        void append(const uint8_t* const data, const size_t& size, bool isEscape = true)
        {
            if (isEscape)
            {
                std::vector<uint16_t> escapes;
                for (size_t i = 0; i < size; ++i)
                    if ((data[i] == START_BYTE) || (data[i] == ESCAPE_BYTE))
                        escapes.push_back(i);

                if (escapes.empty())
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        pack_buffer.push_back(data[i]);
                    }
                }
                else
                {
                    size_t start = 0;
                    while (!escapes.empty())
                    {
                        const size_t& idx = escapes.front();
                        append(data + start, idx - start);
                        append(data[idx], true);
                        start = idx + 1;
                        escapes.erase(escapes.begin());
                    }
                    if (start < size) append(data + start, size - start);
                }
            }
            else
            {
                for (size_t i = 0; i < size; ++i)
                {
                    pack_buffer.push_back(data[i]);
                }
            }
        }


        void append(const uint8_t& data, bool isEscape = true)
        {
            if (isEscape && ((data == START_BYTE) || (data == ESCAPE_BYTE)))
            {
                pack_buffer.push_back(ESCAPE_BYTE);
                pack_buffer.push_back((uint8_t)(data ^ ESCAPE_MASK));
            }
            else
            {
                pack_buffer.push_back(data);
            }
        }

    private:

        Checker mode;

        std::vector<uint8_t> pack_buffer;
    };

}
