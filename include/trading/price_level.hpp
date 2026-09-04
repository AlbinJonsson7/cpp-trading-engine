#ifndef PRICE_LEVEL_HPP
#define PRICE_LEVEL_HPP

#include <cstdint>
#include "order.hpp"
#include "order_node_pool.hpp"


class PriceLevel {
    private:
        int64_t price;
        uint32_t nodeHeadIndex = INVALID_NODE_INDEX;
        uint32_t nodeTailIndex = INVALID_NODE_INDEX;
        OrderNodePool* nodePtr = nullptr;
        
    public:
        PriceLevel(int64_t price, OrderNodePool* nodePtr);
        uint32_t addOrder(const Order& order);
        void removeFrontOrder();
        bool removeOrder(uint32_t nodeIndex);
        Order& getFrontOrder();
        const Order& getFrontOrder() const;
        bool isEmpty() const;
        int64_t getPrice() const;

};

#endif // PRICE_LEVEL_HPP