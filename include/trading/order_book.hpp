#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <cstdint>
#include <map>
#include <functional>
#include <optional>
#include "price_level.hpp"
#include "order.hpp"



class OrderBook {
    private:
        std::map<int64_t, PriceLevel, std::greater<int64_t>> bids;
        std::map<int64_t, PriceLevel, std::less<int64_t>> asks;

    public:
        void addOrder(const Order& order);
        std::optional<int64_t> getBestBid() const;
        std::optional<int64_t> getBestAsk() const;
        std::optional<Order> getBestBidOrder() const;
        std::optional<Order> getBestAskOrder() const;
        bool hasBids() const;
        bool hasAsks() const;
        bool removePriceLevel(int64_t price, Side side);
        bool fillBestOrder(Side side, uint32_t quantity);
        bool cancelOrder(uint64_t orderID);
};




#endif // ORDER_BOOK_HPP