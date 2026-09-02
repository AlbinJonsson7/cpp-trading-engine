#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include "trade.hpp"
#include "order_book.hpp"
#include "order.hpp"
#include "process_status.hpp"


class MatchingEngine{
    private:
        OrderBook orderBook;
        static constexpr uint32_t MAX_ORDER_QUANTITY = 1000000;

    public:
        MatchingEngine(std::size_t expectedOrders);
        ProcessStatus processOrder(const Order& incomingOrder, std::vector<Trade>& tradeBuffer);
        bool cancelOrder(uint64_t orderID);

};

#endif // MATCHING_ENGINE_HPP