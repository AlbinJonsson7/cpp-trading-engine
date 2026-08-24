#include <iostream>
#include "trading/order_book.hpp"


void OrderBook::addOrder(const Order& order){
    if(order.side == Side::BUY){
        auto it = bids.find(order.price);
        if(it != bids.end()){
            it->second.addOrder(order);
        }else{
            PriceLevel newPriceLevel(order.price);
            newPriceLevel.addOrder(order);
            bids.emplace(order.price, newPriceLevel);
        }
    }else if(order.side == Side::SELL){
        auto it = asks.find(order.price);
        if(it != asks.end()){
            it->second.addOrder(order);
        }else{
            PriceLevel newPriceLevel(order.price);
            newPriceLevel.addOrder(order);
            asks.emplace(order.price, newPriceLevel);
        }
    }else{
        std::cerr << "Error: Invalid order side." << std::endl;
    }

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


std::optional<Order> OrderBook::getBestBidOrder() const{
    if(bids.empty()){
        return std::nullopt;
    }
    return std::make_optional(bids.begin()->second.getFrontOrder());
}


std::optional<Order> OrderBook::getBestAskOrder() const{
    if(asks.empty()){
        return std::nullopt;
    }
    return std::make_optional(asks.begin()->second.getFrontOrder());
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
            it->second.removeFrontOrder();  
        }
        if(it->second.isEmpty()){
            removePriceLevel(it->first,Side::SELL);
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
            it->second.removeFrontOrder();
        }
        if(it->second.isEmpty()){
            removePriceLevel(it->first, Side::BUY);
        }
        return true;
    }
    return false;
}



