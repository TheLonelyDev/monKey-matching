#include <eosio/crypto.hpp>
#include <array>

class random
{
public:
    random(eosio::checksum256 seed)
    {
        set_seed(seed);
    }

    uint32_t next(uint64_t max)
    {
        //return (get_value() % (max + 1 - min)) + min;
        return get_value() % max;
    }

private:
    uint64_t offset = 0;
    std::array<uint8_t, 32> raw;

    void reseed()
    {
        set_seed(eosio::sha256((char *)raw.data(), 32));
    }

    void set_seed(eosio::checksum256 seed)
    {
        // Set raw byte array & reset offset
        raw = seed.extract_as_byte_array();
        offset = 0;
    }

    uint64_t get_value()
    {
        // Reseed after x uses
        if (offset > 19)
        {
            reseed();
        }

        uint64_t value = 0;
        for (int i = 0; i < 8; i++)
        {
            value = (value << 8) + raw[offset++];
        }

        return value;
    }
};