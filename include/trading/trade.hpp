#ifndef TRADE_HPP
#define TRADE_HPP

#include <cstdint>


struct Trade{
    uint64_t buyOrderID;
    uint64_t sellOrderID;
    int64_t price;
    uint32_t quantity;
};

#endif // TRADE_HPP