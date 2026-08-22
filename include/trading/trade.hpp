#ifndef TRADE_HPP
#define TRADE_HPP

#include <cstdint>


struct Trade{
    uint64_t buyOrderID;
    uint64_t sellOrderID;
    int64_t price;
    uint32_t quantity;
};


/*
Trade
    buyOrderID  → uint64_t
    sellOrderID → uint64_t
    price       → int64_t
    quantity    → uint32_t

*/


#endif // TRADE_HPP