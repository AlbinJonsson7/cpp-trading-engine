#include <iostream>
#include "trading/order_book.hpp"


OrderBook::OrderBook(std::size_t expectedOrders):nodePool(expectedOrders),orderLocations(expectedOrders){}


bool OrderBook::addOrder(const Order& order){
    OrderLocation data{};
    if(order.side == Side::BUY){
        auto it = bids.try_emplace(order.price,order.price,&nodePool);
        
        data.nodeIndex = it.first->second.addOrder(order);
        data.priceLevel = &it.first->second;

        if(!orderLocations.insert(order.orderID, data)){
            data.priceLevel->removeOrder(data.nodeIndex);
            if(data.priceLevel->isEmpty()){
                bids.erase(it.first);
            }
            return false;
        }
        return true;

    }else if(order.side == Side::SELL){
        auto it = asks.try_emplace(order.price,order.price,&nodePool);
        
        data.nodeIndex = it.first->second.addOrder(order);
        data.priceLevel = &it.first->second;
        
        if(!orderLocations.insert(order.orderID, data)){
            data.priceLevel->removeOrder(data.nodeIndex);

            if(data.priceLevel->isEmpty()){
                asks.erase(it.first);
            }

            return false;
        }
        return true;
        
    }
    return false;
}


std::optional<int64_t> OrderBook::getBestBid() const{
    if(bids.empty()){
        return std::nullopt;
    }
    return std::make_optional(bids.begin()->second.getPrice());
}


std::optional<int64_t> OrderBook::getBestAsk() const{
    if(asks.empty()){
        return std::nullopt;
    }
    return std::make_optional(asks.begin()->second.getPrice());
}


const Order* OrderBook::getBestBidOrder() const{
    if(bids.empty()){
        return nullptr;
    }
    return &bids.begin()->second.getFrontOrder();
}


const Order* OrderBook::getBestAskOrder() const{
    if(asks.empty()){
        return nullptr;
    }
    return &asks.begin()->second.getFrontOrder();
}


bool OrderBook::hasBids() const{
    return !bids.empty();
}


bool OrderBook::hasAsks() const{
    return !asks.empty();
}


bool OrderBook::removePriceLevel(int64_t price, Side side){
    if(side == Side::BUY){
        auto it = bids.find(price);
        if(it != bids.end()){
            bids.erase(it);
            return true;
        }
    }else if(side == Side::SELL){
        auto it = asks.find(price);
        if(it != asks.end()){
            asks.erase(it);
            return true;
        }
    }
    return false;
}


bool OrderBook::fillBestOrder(Side side, uint32_t quantity){
    if(side == Side::SELL){
        if(asks.empty()){
            return false;
        }
        auto it = asks.begin();
        Order& order = it->second.getFrontOrder();
        if(quantity > order.remainingQuantity)return false;
        order.remainingQuantity = order.remainingQuantity - quantity;
        if(order.remainingQuantity == 0){
            orderLocations.erase(order.orderID); 
            it->second.removeFrontOrder(); 
        }
        if(it->second.isEmpty()){
            asks.erase(it);
        }
        return true;
    }else if (side == Side::BUY){
        if(bids.empty()){
            return false;
        }
        auto it = bids.begin();
        Order& order = it->second.getFrontOrder();
        if(quantity > order.remainingQuantity) return false;
        order.remainingQuantity = order.remainingQuantity - quantity;
        if(order.remainingQuantity == 0){
            orderLocations.erase(order.orderID); 
            it->second.removeFrontOrder(); 
        }
        if(it->second.isEmpty()){
            bids.erase(it);
        }
        return true;
    }
    return false;
}


bool OrderBook::cancelOrder(uint64_t orderID){

    OrderLocation* orderData = orderLocations.find(orderID);
    if(orderData == nullptr){
        return false;
    }

    auto nodeIndex = orderData->nodeIndex;
    auto priceLevel = orderData->priceLevel;

    auto nodeOrder = &nodePool.getOrder(nodeIndex);

    auto price = nodeOrder->price;
    auto side = nodeOrder->side;

    if(priceLevel->removeOrder(nodeIndex)){
        orderLocations.erase(orderID);
        if(priceLevel->isEmpty()){
            removePriceLevel(price,side);
        }
        return true;
    }
    return false;
}


bool OrderBook::containsOrder(uint64_t orderID) const{
    return orderLocations.contains(orderID);
}


