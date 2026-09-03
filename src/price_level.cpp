#include "trading/price_level.hpp"
#include <cassert>


PriceLevel::PriceLevel(int64_t price, OrderNodePool* nodePtr) : price(price), nodePtr(nodePtr) {
    assert(nodePtr != nullptr);
}


uint32_t PriceLevel::addOrder(const Order& order) {
    auto nodeIndex = nodePtr->acquire(order);

    if (nodeHeadIndex == INVALID_NODE_INDEX){

        nodeHeadIndex = nodeIndex;
        nodeTailIndex = nodeIndex;

        return nodeIndex;
    }

    auto oldTail = nodeTailIndex;

    nodePtr->get(oldTail).next = nodeIndex;
    nodePtr->get(nodeIndex).previous = oldTail;

    nodeTailIndex = nodeIndex;
    
    return nodeIndex;
}

void PriceLevel::removeFrontOrder(){
    if(nodeHeadIndex == INVALID_NODE_INDEX){
        return;
    }

    auto headNode = &nodePtr->get(nodeHeadIndex);
    auto oldHeadIndex = nodeHeadIndex;
    
    if(headNode->next == INVALID_NODE_INDEX){
        nodeHeadIndex = INVALID_NODE_INDEX;
        nodeTailIndex = INVALID_NODE_INDEX;
    }else{
        nodeHeadIndex = headNode->next;
        nodePtr->get(nodeHeadIndex).previous = INVALID_NODE_INDEX;
    }

    nodePtr->release(oldHeadIndex);

    return;
}


bool PriceLevel::removeOrder(uint32_t nodeIndex){
    auto node = &nodePtr->get(nodeIndex);
    auto oldNextIndex = node->next;
    auto oldPrevIndex = node->previous;

    if(oldPrevIndex != INVALID_NODE_INDEX){
        nodePtr->get(oldPrevIndex).next = oldNextIndex;
    }else{
        nodeHeadIndex = oldNextIndex;
    }

    if(oldNextIndex != INVALID_NODE_INDEX){
        nodePtr->get(oldNextIndex).previous = oldPrevIndex;
    }else{
        nodeTailIndex = oldPrevIndex;
    }

    return nodePtr->release(nodeIndex);
}


Order& PriceLevel::getFrontOrder(){
    assert(nodeHeadIndex != INVALID_NODE_INDEX);
    Order& frontOrder = nodePtr->get(nodeHeadIndex).order;

    return frontOrder;
}

const Order& PriceLevel::getFrontOrder() const {
    assert(nodeHeadIndex != INVALID_NODE_INDEX);
    const Order& frontOrder = nodePtr->get(nodeHeadIndex).order;

    return frontOrder;
}


bool PriceLevel::isEmpty() const {
    return nodeHeadIndex == INVALID_NODE_INDEX;
}


int64_t PriceLevel::getPrice() const{
    return price;
}

