#ifndef ORDER_INDEX_HPP
#define ORDER_INDEX_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include "order_location.hpp"


struct FindResult{
    OrderLocation* location{};
    std::size_t slotIndex;
};

class OrderIndex{
    private:
        struct Slot{
            uint64_t orderID;
            OrderLocation location;
            bool occupied = false;
        };
        std::vector<Slot> slots;
        std::size_t activeEntries;
        std::size_t tableCapacity;

    public:
        OrderIndex(std::size_t expectedOrders);
        bool insert(uint64_t orderID, OrderLocation location);
        FindResult find(uint64_t orderID);
        const OrderLocation* find(uint64_t orderID) const;
        bool erase(uint64_t orderID);
        bool eraseAt(std::size_t slotIndex, uint64_t orderID);
        bool contains(uint64_t orderID) const;
};


#endif // ORDER_INDEX_HPP