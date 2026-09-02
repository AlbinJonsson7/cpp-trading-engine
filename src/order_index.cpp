#include <cstdint>
#include "trading/order_index.hpp"


OrderIndex::OrderIndex(std::size_t expectedOrders){
    auto reservedEntries = expectedOrders/PAGE_SIZE;
    if((expectedOrders % PAGE_SIZE) != 0){
        reservedEntries +=1;
    }
    pageDirectory.reserve(reservedEntries);
}


bool OrderIndex::insert(uint64_t orderID, OrderLocation location){
    if(orderID == 0){
        return false;
    }

    auto zeroBasedID = orderID - 1;
    auto pageIndex = zeroBasedID / PAGE_SIZE;
    auto offset = zeroBasedID % PAGE_SIZE;

    if(pageDirectory.size() <= pageIndex){
        pageDirectory.resize(pageIndex + 1);
    }

    if (pageDirectory[pageIndex] == nullptr){
        pageDirectory[pageIndex] = std::make_unique<Page>();
    }
    
    if (pageDirectory[pageIndex]->locations[offset].priceLevel != nullptr){
        return false;
    }

    pageDirectory[pageIndex]->locations[offset] = location;
    pageDirectory[pageIndex]->activeEntries += 1;

    return true;
}


OrderLocation* OrderIndex::find(uint64_t orderID){
    if(orderID == 0){
        return nullptr;
    }

    auto zeroBasedID = orderID - 1;
    auto pageIndex = zeroBasedID/PAGE_SIZE;
    auto offset = zeroBasedID % PAGE_SIZE;

    if(pageIndex >= pageDirectory.size()){
        return nullptr;
    }else if(pageDirectory[pageIndex] == nullptr){
        return nullptr;
    }else if(pageDirectory[pageIndex]->locations[offset].priceLevel == nullptr){
        return nullptr;
    }

    return &pageDirectory[pageIndex]->locations[offset];
}


const OrderLocation* OrderIndex::find(uint64_t orderID) const{
    if(orderID == 0){
        return nullptr;
    }

    auto zeroBasedID = orderID - 1;
    auto pageIndex = zeroBasedID/PAGE_SIZE;
    auto offset = zeroBasedID % PAGE_SIZE;

    if(pageIndex >= pageDirectory.size()){
        return nullptr;
    }else if(pageDirectory[pageIndex] == nullptr){
        return nullptr;
    }else if(pageDirectory[pageIndex]->locations[offset].priceLevel == nullptr){
        return nullptr;
    }

    return &pageDirectory[pageIndex]->locations[offset];
}


bool OrderIndex::erase(uint64_t orderID){
     if(orderID == 0){
        return false;
    }

    auto zeroBasedID = orderID - 1;
    auto pageIndex = zeroBasedID/PAGE_SIZE;
    auto offset = zeroBasedID%PAGE_SIZE;

    if(pageIndex >= pageDirectory.size()){
        return false;
    }else if(pageDirectory[pageIndex] == nullptr){
        return false;
    }else if(pageDirectory[pageIndex]->locations[offset].priceLevel == nullptr){
        return false;
    }

    pageDirectory[pageIndex]->locations[offset] = {{},nullptr};
    pageDirectory[pageIndex]->activeEntries -= 1;
    if(pageDirectory[pageIndex]->activeEntries == 0){
        pageDirectory[pageIndex].reset();
    }
    
    return true;
}


bool OrderIndex::contains(uint64_t orderID) const{
    return find(orderID) != nullptr;
}
