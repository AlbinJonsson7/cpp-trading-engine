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
        void addOrder(const Order& order);
        void removeFrontOrder();
        void removeOrder(uint64_t orderID);
        Order& getFrontOrder();
        const Order& getFrontOrder() const;
        bool isEmpty() const;
        int64_t getPrice() const;
    

};


/*
PriceLevel
    Private:
        Price → int64_t
        Orders → std::list<Order>
    
    public:
        addOrder(...)
        removeOrder(...)
        getFrontOrder(...)
        isEmpty(...)
        getPrice(...)
*/


#endif // PRICE_LEVEL_HPP