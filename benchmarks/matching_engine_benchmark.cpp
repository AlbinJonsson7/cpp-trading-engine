#include <cstdint>
#include <iostream>
#include <vector>

#include "trading/matching_engine.hpp"
#include "trading/order.hpp"
#include "trading/trade.hpp"


inline constexpr std::size_t NUMBER_OF_ORDERS = 1'000'000;
inline constexpr std::size_t NUMBER_OF_PROFILE_RUNS = 10;

inline constexpr int64_t PRICE = 10'000;
inline constexpr uint32_t QUANTITY = 100;


// =====================================================
// CREATE ORDER
// =====================================================

Order makeOrder(uint64_t orderID)
{
    Order order{};

    order.orderID = orderID;
    order.side = Side::BUY;
    order.price = PRICE;
    order.originalQuantity = QUANTITY;
    order.remainingQuantity = QUANTITY;
    order.orderType = OrderType::LIMIT;

    return order;
}


// =====================================================
// PRELOAD
// =====================================================

__declspec(noinline)
void preloadOrderBook(
    MatchingEngine& engine,
    const std::vector<Order>& orders,
    std::vector<Trade>& tradeBuffer)
{
    for(const Order& order : orders){
        engine.processOrder(
            order,
            tradeBuffer
        );
    }
}


// =====================================================
// CANCELLATION WORKLOAD
// =====================================================

__declspec(noinline)
void runCancellationWorkload(
    MatchingEngine& engine,
    const std::vector<Order>& orders)
{
    for(const Order& order : orders){
        engine.cancelOrder(order.orderID);
    }
}


// =====================================================
// MAIN
// =====================================================

int main()
{
    std::cout << "========================================\n";
    std::cout << "V2 SEQUENTIAL CANCEL CPU PROFILE\n";
    std::cout << "========================================\n";

    std::cout
        << "Orders per run: "
        << NUMBER_OF_ORDERS
        << "\n";

    std::cout
        << "Price levels: 1\n";

    std::cout
        << "Profile runs: "
        << NUMBER_OF_PROFILE_RUNS
        << "\n\n";


    // =================================================
    // GENERATE ORDERS ONCE
    // =================================================

    std::vector<Order> orders;
    orders.reserve(NUMBER_OF_ORDERS);

    for(std::size_t i = 0;
        i < NUMBER_OF_ORDERS;
        ++i)
    {
        orders.push_back(
            makeOrder(
                static_cast<uint64_t>(i + 1)
            )
        );
    }


    // =================================================
    // PROFILE RUNS
    // =================================================

    for(std::size_t run = 0;
        run < NUMBER_OF_PROFILE_RUNS;
        ++run)
    {
        MatchingEngine engine(NUMBER_OF_ORDERS);

        std::vector<Trade> tradeBuffer;


        // Preload is separated in the call tree.
        preloadOrderBook(
            engine,
            orders,
            tradeBuffer
        );


        // Cancellation path we actually want to study.
        runCancellationWorkload(
            engine,
            orders
        );


        std::cout
            << "Completed profile run "
            << run + 1
            << " / "
            << NUMBER_OF_PROFILE_RUNS
            << "\n";
    }


    std::cout << "\n========================================\n";
    std::cout << "PROFILE WORKLOAD COMPLETE\n";
    std::cout << "========================================\n";

    return 0;
}