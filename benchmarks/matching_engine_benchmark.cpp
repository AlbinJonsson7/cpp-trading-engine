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


// =====================================================
// TEST SETTINGS
// =====================================================

inline constexpr std::size_t NUMBER_OF_ORDERS = 1'000'000;
inline constexpr std::size_t NUMBER_OF_RUNS = 10;

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
// MAIN
// =====================================================

int main()
{
    std::cout << std::fixed << std::setprecision(6);

    std::cout << "========================================\n";
    std::cout << "V3.4 TEST: CANCEL - 1 LEVEL - SEQUENTIAL\n";
    std::cout << "========================================\n";

    std::cout
        << "Orders per run: "
        << NUMBER_OF_ORDERS
        << "\n";

    std::cout
        << "Measured runs: "
        << NUMBER_OF_RUNS
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
    // STORE RUN TIMES
    // =================================================

    std::vector<double> runTimes;
    runTimes.reserve(NUMBER_OF_RUNS);


    // =================================================
    // 10 INDEPENDENT RUNS
    // =================================================

    for(std::size_t run = 0;
        run < NUMBER_OF_RUNS;
        ++run)
    {
        MatchingEngine engine(NUMBER_OF_ORDERS);

        std::vector<Trade> tradeBuffer;


        // =============================================
        // PRELOAD - NOT TIMED
        // =============================================

        for(const Order& order : orders){
            engine.processOrder(
                order,
                tradeBuffer
            );
        }


        // =============================================
        // SEQUENTIAL CANCELLATION - TIMED
        // =============================================

        const auto start = Clock::now();


        for(const Order& order : orders){
            engine.cancelOrder(
                order.orderID
            );
        }


        const auto end = Clock::now();


        // =============================================
        // RESULTS FOR THIS RUN
        // =============================================

        const double seconds =
            std::chrono::duration<double>(
                end - start
            ).count();


        runTimes.push_back(seconds);


        const double throughput =
            static_cast<double>(
                NUMBER_OF_ORDERS
            ) / seconds;


        const double nsPerCancel =
            seconds * 1e9
            / static_cast<double>(
                NUMBER_OF_ORDERS
            );


        std::cout
            << "Run " << run + 1
            << ": "
            << seconds
            << " s"
            << " | "
            << throughput
            << " cancels/s"
            << " | "
            << nsPerCancel
            << " ns/cancel"
            << "\n";
    }


    // =================================================
    // FINAL MEDIAN
    // =================================================

    const double medianSeconds =
        median(runTimes);


    const double medianThroughput =
        static_cast<double>(
            NUMBER_OF_ORDERS
        ) / medianSeconds;


    const double medianNsPerCancel =
        medianSeconds * 1e9
        / static_cast<double>(
            NUMBER_OF_ORDERS
        );


    std::cout << "\n========================================\n";
    std::cout << "FINAL RESULT\n";
    std::cout << "========================================\n";

    std::cout
        << "Median: "
        << medianSeconds
        << " s\n";

    std::cout
        << "Throughput: "
        << medianThroughput
        << " cancels/s\n";

    std::cout
        << "Cost: "
        << medianNsPerCancel
        << " ns/cancel\n";


    return 0;
}