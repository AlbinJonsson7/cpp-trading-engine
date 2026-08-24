#include "trading/price_level.hpp"
#include <iostream>
#include <stdexcept>

PriceLevel::PriceLevel(int64_t price) : price(price) {}


void PriceLevel::addOrder(const Order& order) {
    orders.push_back(order);
}

void PriceLevel::removeFrontOrder(){
    if(!orders.empty()){
        orders.pop_front();
    }else{
        std::cerr << "Error: Cannot remove order from an empty price level." << std::endl;
    }
}

void PriceLevel::removeOrder(uint64_t orderID){
    for(auto it = orders.begin(); it != orders.end(); ++it){
        if(it->orderID == orderID){
            orders.erase(it);
            return;
        }
    }
    std::cerr << "Error: Order with ID " << orderID << " not found in this price level." << std::endl;
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
