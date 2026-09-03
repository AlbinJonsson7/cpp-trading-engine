#include <cstdint>
#include <cassert>
#include "trading/order_node_pool.hpp"



OrderNodePool::OrderNodePool(std::size_t expectedOrders){
    nodes.reserve(expectedOrders);
    freeIndices.reserve(expectedOrders);
}



uint32_t OrderNodePool::acquire(const Order& order){
    uint32_t index;
    if(!freeIndices.empty()){
        index = freeIndices.back();
        freeIndices.pop_back();
        nodes[index].order = order;
        nodes[index].next = INVALID_NODE_INDEX;
        nodes[index].previous = INVALID_NODE_INDEX;
        nodes[index].active = true;
        return index;
    }

    OrderNode node;

    node.active = true;
    node.order = order;
    nodes.push_back(node);
    index = nodes.size() - 1;

    return index;
}


bool OrderNodePool::release(uint32_t nodeIndex){
    if(nodeIndex >= (nodes.size())){
        return false;
    }else if(nodes[nodeIndex].active == false){
        return false;
    }

    nodes[nodeIndex].previous = INVALID_NODE_INDEX;
    nodes[nodeIndex].next = INVALID_NODE_INDEX;
    nodes[nodeIndex].active = false;
    freeIndices.push_back(nodeIndex);

    return true;
}


OrderNode& OrderNodePool::get(uint32_t nodeIndex){
    assert(nodeIndex < nodes.size());
    assert(nodes[nodeIndex].active == true);
    OrderNode& node = nodes[nodeIndex];
    return node;
}


const OrderNode& OrderNodePool::get(uint32_t nodeIndex) const{
    assert(nodeIndex < nodes.size());
    assert(nodes[nodeIndex].active == true);
    const OrderNode& node = nodes[nodeIndex];
    return node;
}


