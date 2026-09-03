#include <cstdint>
#include <utility>
#include "trading/order_index.hpp"


OrderIndex::OrderIndex(std::size_t expectedOrders){
    auto reservedEntries = expectedOrders/PAGE_SIZE;
    if((expectedOrders % PAGE_SIZE) != 0){
        reservedEntries +=1;
    }
    pageDirectory.reserve(reservedEntries);
    freePages.reserve(reservedEntries);
    auto count = 0;
    while(count < reservedEntries){
        freePages.push_back(std::make_unique<Page>());
        count++;
    }
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

    if(!hasLatestPage){
        hasLatestPage = true;
        latestPageIndex = pageIndex;
    }else if(pageIndex > latestPageIndex){
        if(pageDirectory[latestPageIndex] != nullptr){
            if(pageDirectory[latestPageIndex]->activeEntries == 0){
                freePages.push_back(std::move(pageDirectory[latestPageIndex]));
            }
        }
        latestPageIndex = pageIndex;
    }

    if(!freePages.empty() && pageDirectory[pageIndex] == nullptr){
        pageDirectory[pageIndex] = std::move(freePages.back());
        freePages.pop_back();
    }else if(freePages.empty() && pageDirectory[pageIndex] == nullptr){
        pageDirectory[pageIndex] = std::make_unique<Page>();
    }
    
    if(pageDirectory[pageIndex]->locations[offset].priceLevel != nullptr){
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

    pageDirectory[pageIndex]->locations[offset] = {INVALID_NODE_INDEX,nullptr};
    pageDirectory[pageIndex]->activeEntries -= 1;
    if(pageDirectory[pageIndex]->activeEntries == 0 && pageIndex < latestPageIndex){
        freePages.push_back(std::move(pageDirectory[pageIndex]));
    }
    
    return true;
}


bool OrderIndex::contains(uint64_t orderID) const{
    return find(orderID) != nullptr;
}
