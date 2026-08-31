#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP

#include <cstdint>
#include <cstddef>
#include "order_book.hpp"
#include "order.hpp"
#include "process_result.hpp"


class MatchingEngine{
    private:
        OrderBook orderBook;
        static constexpr uint32_t MAX_ORDER_QUANTITY = 1000000;

    public:
        MatchingEngine(std::size_t expectedOrders);
        ProcessResult processOrder(const Order& incomingOrder);
        bool cancelOrder(uint64_t orderID);

};

#endif // MATCHING_ENGINE_HPP