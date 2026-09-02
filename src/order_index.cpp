#include <cstdint>
#include <functional>
#include "trading/order_index.hpp"


OrderIndex::OrderIndex(std::size_t expectedOrders){
    activeEntries = 0;
    tableCapacity = 1;

    auto targetCapacity = expectedOrders*2;
    while(targetCapacity > tableCapacity){
        tableCapacity *= 2;
    }
    slots.resize(tableCapacity);
}


bool OrderIndex::insert(uint64_t orderID, OrderLocation location){
    auto startIndex = std::hash<uint64_t>{}(orderID) & (tableCapacity - 1);
    std::size_t count = 0;
    while(count < tableCapacity){

        if(!slots[startIndex].occupied){
            slots[startIndex].orderID = orderID;
            slots[startIndex].location = location;
            slots[startIndex].occupied = true;
            activeEntries++;
            return true;

        }else if(slots[startIndex].orderID == orderID){
            return false;
        }

        startIndex++;
        count++;

        if(startIndex == tableCapacity){
            startIndex = 0;
        }
    }

    return false;
}


FindResult OrderIndex::find(uint64_t orderID){
    auto startIndex = std::hash<uint64_t>{}(orderID) & (tableCapacity - 1);
    FindResult searchResult = {nullptr};
    std::size_t count = 0;
    while(count < tableCapacity){
        if(!slots[startIndex].occupied){
            return searchResult;
        }

        if(slots[startIndex].orderID == orderID){
            searchResult.location = &slots[startIndex].location;
            searchResult.slotIndex = startIndex;
            return searchResult;
        }

        startIndex++;
        count++;

        if(startIndex == tableCapacity){
            startIndex = 0;
        }
    }
    return searchResult;
}


const OrderLocation* OrderIndex::find(uint64_t orderID) const{
    auto startIndex = std::hash<uint64_t>{}(orderID) & (tableCapacity - 1);
    std::size_t count = 0;

    while(count < tableCapacity){
        if(!slots[startIndex].occupied){
            return nullptr;
        }

        if(slots[startIndex].orderID == orderID){
            return &slots[startIndex].location;
        }

        startIndex++;
        count++;

        if(startIndex == tableCapacity){
            startIndex = 0;
        }
    }
    return nullptr;
}


bool OrderIndex::erase(uint64_t orderID){
    FindResult searchResult = find(orderID);
    if(searchResult.location == nullptr){
        return false;
    }
    return eraseAt(searchResult.slotIndex, orderID);
    
}


bool OrderIndex::eraseAt(std::size_t slotIndex, uint64_t orderID){
    if(slotIndex >= tableCapacity){
        return false;
    }
    
    if(!slots[slotIndex].occupied){
        return false;
    }

    if(slots[slotIndex].orderID == orderID){
        const std::size_t mask = tableCapacity - 1;
        std::size_t hole = slotIndex;
        std::size_t current = (hole + 1) & mask;
        std::size_t scanned = 0;

        while(scanned < tableCapacity - 1 && slots[current].occupied){

            std::size_t home = std::hash<uint64_t>{}(slots[current].orderID) & mask;

            std::size_t distanceToCurrent = (current - home) & mask;

            std::size_t distanceToHole = (hole - home) & mask;

            if(distanceToHole < distanceToCurrent){
                slots[hole] = slots[current];
                hole = current;
            }

            current = (current + 1) & mask;
            scanned++;
        }

        slots[hole].occupied = false;
        activeEntries--;

        return true;
    }
    return false;
}


bool OrderIndex::contains(uint64_t orderID) const{
    return find(orderID) != nullptr;
}
