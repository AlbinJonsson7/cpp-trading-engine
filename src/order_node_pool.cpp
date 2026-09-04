#include <cstdint>
#include <cassert>
#include "trading/order_node_pool.hpp"


OrderNodePool::OrderNodePool(std::size_t expectedOrders){
    orders.reserve(expectedOrders);
    links.reserve(expectedOrders);
    freeIndices.reserve(expectedOrders);
}



uint32_t OrderNodePool::acquire(const Order& order){
    uint32_t index;
    if(!freeIndices.empty()){
        index = freeIndices.back();
        freeIndices.pop_back();
        orders[index] = order;
        links[index].next = INVALID_NODE_INDEX;
        links[index].previous = INVALID_NODE_INDEX;
        links[index].active = true;
        return index;
    }

    OrderLinks link;

    orders.push_back(order);
    index = static_cast<uint32_t>(orders.size() - 1);
    link.active = true;
    links.push_back(link);


    return index;
}


bool OrderNodePool::release(uint32_t nodeIndex){
    assert(links.size() == orders.size());
    if(nodeIndex >= (orders.size())){
        return false;
    }else if(!links[nodeIndex].active){
        return false;
    }

    links[nodeIndex].previous = INVALID_NODE_INDEX;
    links[nodeIndex].next = INVALID_NODE_INDEX;
    links[nodeIndex].active = false;
    freeIndices.push_back(nodeIndex);

    return true;
}


Order& OrderNodePool::getOrder(uint32_t nodeIndex){
    assert(links.size() == orders.size());
    assert(nodeIndex < orders.size());
    assert(links[nodeIndex].active == true);
    
    return orders[nodeIndex];
}


OrderLinks& OrderNodePool::getLinks(uint32_t nodeIndex){
    assert(links.size() == orders.size());
    assert(nodeIndex < orders.size());
    assert(links[nodeIndex].active == true);
    
    return links[nodeIndex];
}


const Order& OrderNodePool::getOrder(uint32_t nodeIndex) const{
    assert(links.size() == orders.size());
    assert(nodeIndex < orders.size());
    assert(links[nodeIndex].active == true);
    
    return orders[nodeIndex];
}


const OrderLinks& OrderNodePool::getLinks(uint32_t nodeIndex) const{
    assert(links.size() == orders.size());
    assert(nodeIndex < orders.size());
    assert(links[nodeIndex].active == true);
    
    return links[nodeIndex];
}
