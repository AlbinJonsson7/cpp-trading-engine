#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <cstdint>
#include <map>
#include <unordered_map>
#include <functional>
#include <optional>
#include "price_level.hpp"
#include "order.hpp"


struct OrderLocation{
    Side side;
    int64_t price;
    std::list<Order>::iterator orderIterator;
};



class OrderBook {
    private:
        std::map<int64_t, PriceLevel, std::greater<int64_t>> bids;
        std::map<int64_t, PriceLevel, std::less<int64_t>> asks;
        std::unordered_map<uint64_t, OrderLocation> orderLocations;

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
        bool containsOrder(uint64_t orderID) const;
};




#endif // ORDER_BOOK_HPP