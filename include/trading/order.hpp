#ifndef ORDER_HPP
#define ORDER_HPP

#include <cstdint>
#include <string_view>


enum class Side : uint8_t {
    BUY,
    SELL
};

enum class OrderType : uint8_t{
    MARKET,
    LIMIT,
};

struct Order{
    uint64_t orderID;
    int64_t price;
    uint32_t originalQuantity;
    uint32_t remainingQuantity;
    Side side;
    OrderType orderType = OrderType::LIMIT; //Limit order is default
};


constexpr std::string_view toString(Side side) {
    switch (side) {
        case Side::BUY:  return "BUY";
        case Side::SELL: return "SELL";
        default:          return "UNKNOWN";
    }
}

#endif // ORDER_HPP