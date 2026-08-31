#ifndef PRICE_LEVEL_HPP
#define PRICE_LEVEL_HPP

#include <cstdint>
#include <list>
#include <memory_resource>
#include "order.hpp"

class PriceLevel {
    private:
        int64_t price;
        std::pmr::list<Order> orders;
    public:
        PriceLevel(int64_t price,std::pmr::memory_resource* pool);
        std::pmr::list<Order>::iterator addOrder(const Order& order);
        void removeFrontOrder();
        bool removeOrder(std::pmr::list<Order>::iterator orderIterator);
        Order& getFrontOrder();
        const Order& getFrontOrder() const;
        bool isEmpty() const;
        int64_t getPrice() const;
};

#endif // PRICE_LEVEL_HPP