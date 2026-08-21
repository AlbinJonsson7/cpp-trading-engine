#ifndef ORDER_HPP
#define ORDER_HPP

#include <cstdint>


enum class Side : uint8_t {
    BUY,
    SELL
};

struct Order{
    uint64_t orderID;
    Side side;
    int64_t price;
    uint32_t originalQuantity;
    uint32_t remainingQuantity;
};

// For ID variable, use uint64_t since it has a large range of 0 to 18,446,744,073,709,551,615.
// For side variable (BUY or SELL side), Side → enum class { Buy, Sell }
// Original Quantity → uint32_t, wont be negative and wont be large enough to exceed maximum space.
// Remaining Quantity→ uint32_t
// Price will be a scaled integer to avoid floating errors. Using int64_t (signed 64 bit integer)


/*
Order
    Order ID           → uint64_t
    Side               → enum class
    Price              → int64_t
    Original Quantity  → uint32_t
    Remaining Quantity → uint32_t


*/
#endif