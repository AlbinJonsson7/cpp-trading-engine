#ifndef ORDER_LOCATION_HPP
#define ORDER_LOCATION_HPP

#include <cstdint>
#include "price_level.hpp"
#include "order.hpp"


struct OrderLocation{
    std::pmr::list<Order>::iterator orderIterator;
    PriceLevel* priceLevel = nullptr;
};


#endif // ORDER_LOCATION_HPP