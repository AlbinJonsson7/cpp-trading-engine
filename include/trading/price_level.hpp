#ifndef PRICE_LEVEL_HPP
#define PRICE_LEVEL_HPP

#include <cstdint>
#include <list>
#include "order.hpp"

class PriceLevel {
    private:
        int64_t price;
        std::list<Order> orders;
    public:
        PriceLevel(int64_t price);
        std::list<Order>::iterator addOrder(const Order& order);
        void removeFrontOrder();
        bool removeOrder(std::list<Order>::iterator orderIterator);
        Order& getFrontOrder();
        const Order& getFrontOrder() const;
        bool isEmpty() const;
        int64_t getPrice() const;
};

#endif // PRICE_LEVEL_HPP