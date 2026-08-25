#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP

#include <cstdint>
#include <vector>
#include "order_book.hpp"
#include "order.hpp"
#include "trade.hpp"


class MatchingEngine{
    private:
        OrderBook orderBook;

    public:
        std::vector<Trade> processOrder(Order incomingOrder);

};

#endif // MATCHING_ENGINE_HPP