#ifndef ORDER_LOCATION_HPP
#define ORDER_LOCATION_HPP

#include <cstdint>
#include "price_level.hpp"
#include "order.hpp"


struct OrderLocation{
    Side side;
    int64_t price;
    std::pmr::list<Order>::iterator orderIterator;
    PriceLevel* priceLevel;
};


#endif // ORDER_LOCATION_HPP