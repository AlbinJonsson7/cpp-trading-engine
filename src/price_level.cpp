#include "trading/price_level.hpp"
#include <iostream>
#include <stdexcept>
#include <iterator>

PriceLevel::PriceLevel(int64_t price) : price(price) {}


std::list<Order>::iterator PriceLevel::addOrder(const Order& order) {
    orders.push_back(order);
    return std::prev(orders.end());
}

void PriceLevel::removeFrontOrder(){
    if(!orders.empty()){
        orders.pop_front();
    }else{
        std::cerr << "Error: Cannot remove order from an empty price level." << std::endl;
    }
}


bool PriceLevel::removeOrder(std::list<Order>::iterator orderIterator){
    if(orderIterator != orders.end()){
        orders.erase(orderIterator);
    return true;
    }
    return false;
}


Order& PriceLevel::getFrontOrder(){
    if(!orders.empty()){
        return orders.front();
    }else{
        std::cerr << "Error: Cannot retrieve order from an empty price level." << std::endl;
        throw std::runtime_error("PriceLevel is empty");
    }

}

const Order& PriceLevel::getFrontOrder() const {
    if(!orders.empty()){
        return orders.front();
    }else{
        std::cerr << "Error: Cannot retrieve order from an empty price level." << std::endl;
        throw std::runtime_error("PriceLevel is empty");
    }
}


bool PriceLevel::isEmpty() const {
    return orders.empty();
}


int64_t PriceLevel::getPrice() const{
    return price;
}

