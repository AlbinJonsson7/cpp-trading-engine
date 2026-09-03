#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include "trading/matching_engine.hpp"
#include "trading/order.hpp"
#include "trading/trade.hpp"


using Clock = std::chrono::steady_clock;

constexpr std::size_t NUMBER_OF_RUNS = 1000;
constexpr std::size_t BATCH_SIZE = 100;
constexpr int64_t PRICE = 10'000;


// =====================================================
// ORDER HELPER
// =====================================================

Order makeLimitOrder(
    uint64_t orderID,
    Side side,
    int64_t price,
    uint32_t quantity)
{
    Order order{};

    order.orderID = orderID;
    order.side = side;
    order.price = price;
    order.originalQuantity = quantity;
    order.remainingQuantity = quantity;
    order.orderType = OrderType::LIMIT;

    return order;
}


// =====================================================
// MEDIAN
// =====================================================

double median(std::vector<double> values)
{
    std::sort(values.begin(), values.end());

    const std::size_t size = values.size();

    if(size % 2 == 1){
        return values[size / 2];
    }

    return (
        values[size / 2 - 1] +
        values[size / 2]
    ) / 2.0;
}


// =====================================================
// CASE A — WARM PAGE
// =====================================================

double runWarmPage()
{
    MatchingEngine engine(5000);
    std::vector<Trade> trades;

    // Allocates page 0 before timing.
    Order preload =
        makeLimitOrder(
            1,
            Side::BUY,
            PRICE,
            1
        );

    engine.processOrder(preload, trades);


    // IDs 1000–1099 all belong to page 0.
    std::vector<Order> orders;
    orders.reserve(BATCH_SIZE);

    for(std::size_t i = 0; i < BATCH_SIZE; ++i){
        orders.push_back(
            makeLimitOrder(
                1000 + i,
                Side::BUY,
                PRICE,
                1
            )
        );
    }


    const auto start = Clock::now();

    for(const Order& order : orders){
        engine.processOrder(order, trades);
    }

    const auto end = Clock::now();


    const double seconds =
        std::chrono::duration<double>(
            end - start
        ).count();

    return seconds * 1e9 / BATCH_SIZE;
}


// =====================================================
// CASE B — CROSS PAGE BOUNDARY
// =====================================================

double runPageBoundary()
{
    MatchingEngine engine(5000);
    std::vector<Trade> trades;

    /*
        ID 4000 belongs to page 0.

        This allocates page 0 before timing, just like
        the warm-page test.
    */
    Order preload =
        makeLimitOrder(
            4000,
            Side::BUY,
            PRICE,
            1
        );

    engine.processOrder(preload, trades);


    /*
        IDs 4050–4149.

        Page 0:
            IDs 1–4096

        Page 1:
            IDs 4097–8192

        Therefore this timed batch crosses the boundary
        and forces page 1 to be allocated.
    */
    std::vector<Order> orders;
    orders.reserve(BATCH_SIZE);

    for(std::size_t i = 0; i < BATCH_SIZE; ++i){
        orders.push_back(
            makeLimitOrder(
                4050 + i,
                Side::BUY,
                PRICE,
                1
            )
        );
    }


    const auto start = Clock::now();

    for(const Order& order : orders){
        engine.processOrder(order, trades);
    }

    const auto end = Clock::now();


    const double seconds =
        std::chrono::duration<double>(
            end - start
        ).count();

    return seconds * 1e9 / BATCH_SIZE;
}


// =====================================================
// MAIN
// =====================================================

int main()
{
    std::cout << std::fixed << std::setprecision(3);

    std::vector<double> warmResults;
    std::vector<double> boundaryResults;

    warmResults.reserve(NUMBER_OF_RUNS);
    boundaryResults.reserve(NUMBER_OF_RUNS);


    std::cout << "========================================\n";
    std::cout << "TARGETED ADD PAGE-BOUNDARY BENCHMARK\n";
    std::cout << "========================================\n";
    std::cout << "Batch size: " << BATCH_SIZE << '\n';
    std::cout << "Measured runs: " << NUMBER_OF_RUNS << "\n\n";


    for(std::size_t run = 0; run < NUMBER_OF_RUNS; ++run){

        warmResults.push_back(
            runWarmPage()
        );

        boundaryResults.push_back(
            runPageBoundary()
        );
    }


    const double warmMedian =
        median(warmResults);

    const double boundaryMedian =
        median(boundaryResults);


    std::cout << "========================================\n";
    std::cout << "FINAL RESULT\n";
    std::cout << "========================================\n";

    std::cout
        << "Warm page median: "
        << warmMedian
        << " ns/add\n";

    std::cout
        << "Boundary median: "
        << boundaryMedian
        << " ns/add\n";

    std::cout
        << "Boundary overhead: "
        << boundaryMedian - warmMedian
        << " ns/add\n";

    std::cout
        << "Boundary / Warm: "
        << boundaryMedian / warmMedian
        << "x\n";


    return 0;
}