#ifndef ORDER_INDEX_HPP
#define ORDER_INDEX_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>
#include <memory>
#include "order_location.hpp"


class OrderIndex{
    private:
        static constexpr uint32_t PAGE_SIZE = 4096;  
        struct Page{
            std::array<OrderLocation,PAGE_SIZE> locations;
            std::size_t activeEntries = 0;
        };
        std::vector<std::unique_ptr<Page>> pageDirectory;
        std::vector<std::unique_ptr<Page>> freePages;
        uint64_t latestPageIndex = 0;
        bool hasLatestPage = false;

    public:
        OrderIndex(std::size_t expectedOrders);
        bool insert(uint64_t orderID, OrderLocation location);
        OrderLocation* find(uint64_t orderID);
        const OrderLocation* find(uint64_t orderID) const;
        bool erase(uint64_t orderID);
        bool contains(uint64_t orderID) const;
};


#endif // ORDER_INDEX_HPP