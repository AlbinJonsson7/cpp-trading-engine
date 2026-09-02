#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <cstdint>
#include <map>
#include <functional>
#include <cstddef>
#include <optional>
#include <memory_resource>
#include "price_level.hpp"
#include "order.hpp"
#include "order_location.hpp"
#include "order_index.hpp"




class OrderBook {
    private:
        std::pmr::unsynchronized_pool_resource pool;
        std::map<int64_t, PriceLevel, std::greater<int64_t>> bids;
        std::map<int64_t, PriceLevel, std::less<int64_t>> asks;
        OrderIndex orderLocations;
        

    public:
        OrderBook(std::size_t expectedOrders);
        bool addOrder(const Order& order);
        std::optional<int64_t> getBestBid() const;
        std::optional<int64_t> getBestAsk() const;
        const Order* getBestBidOrder() const;
        const Order* getBestAskOrder() const;
        bool hasBids() const;
        bool hasAsks() const;
        bool removePriceLevel(int64_t price, Side side);
        bool fillBestOrder(Side side, uint32_t quantity);
        bool cancelOrder(uint64_t orderID);
        bool containsOrder(uint64_t orderID) const;
};




#endif // ORDER_BOOK_HPP