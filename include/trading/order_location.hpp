#ifndef ORDER_LOCATION_HPP
#define ORDER_LOCATION_HPP

#include <cstdint>
#include "price_level.hpp"
#include "order_node_pool.hpp"


struct OrderLocation{
    uint32_t nodeIndex = INVALID_NODE_INDEX;
    PriceLevel* priceLevel = nullptr;
};


#endif // ORDER_LOCATION_HPP