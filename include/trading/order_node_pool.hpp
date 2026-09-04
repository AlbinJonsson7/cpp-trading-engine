#ifndef ORDER_NODE_POOL_HPP
#define ORDER_NODE_POOL_HPP

#include <cstdint>
#include <limits>
#include <vector>
#include <cstddef>
#include "order.hpp"


constexpr uint32_t INVALID_NODE_INDEX = std::numeric_limits<uint32_t>::max();

struct OrderLinks{
    uint32_t previous = INVALID_NODE_INDEX;
    uint32_t next = INVALID_NODE_INDEX;
    bool active = false;
};


class OrderNodePool{
    private:
        std::vector<Order> orders;
        std::vector<OrderLinks> links;
        std::vector<uint32_t> freeIndices;
    public:
        OrderNodePool(std::size_t expectedOrders);
        uint32_t acquire(const Order& order);
        bool release(uint32_t nodeIndex);
        Order& getOrder(uint32_t nodeIndex);
        OrderLinks& getLinks(uint32_t nodeIndex);
        const Order& getOrder(uint32_t nodeIndex) const;
        const OrderLinks& getLinks(uint32_t nodeIndex) const;
};

#endif // ORDER_NODE_POOL_HPP