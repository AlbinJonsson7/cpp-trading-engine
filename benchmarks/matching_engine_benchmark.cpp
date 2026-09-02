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

inline constexpr std::size_t BATCH_SIZE = 100;
inline constexpr std::size_t NUMBER_OF_BATCHES = 1'000;
inline constexpr std::size_t NUMBER_OF_RUNS = 10;

inline constexpr std::size_t TOTAL_ORDERS =
    BATCH_SIZE * NUMBER_OF_BATCHES;

inline constexpr int64_t PRICE = 10'000;
inline constexpr uint32_t QUANTITY = 1;


// =====================================================
// CREATE ORDER
// =====================================================

Order makeOrder(
    uint64_t orderID,
    Side side)
{
    Order order{};

    order.orderID = orderID;
    order.side = side;
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
// PERCENTILE
// =====================================================

double percentile(
    std::vector<double> values,
    double p)
{
    std::sort(values.begin(), values.end());

    if(values.empty()){
        return 0.0;
    }

    const double position =
        p * static_cast<double>(
            values.size() - 1
        );

    const std::size_t index =
        static_cast<std::size_t>(position);

    return values[index];
}


// =====================================================
// MAIN
// =====================================================

int main()
{
    std::cout << std::fixed << std::setprecision(3);

    std::cout << "========================================\n";
    std::cout << "TEST 12: BATCH LATENCY - MATCH 1-to-1\n";
    std::cout << "========================================\n";

    std::cout
        << "Batch size: "
        << BATCH_SIZE
        << "\n";

    std::cout
        << "Batches per run: "
        << NUMBER_OF_BATCHES
        << "\n";

    std::cout
        << "Incoming orders per run: "
        << TOTAL_ORDERS
        << "\n";

    std::cout
        << "Resting orders per run: "
        << TOTAL_ORDERS
        << "\n";

    std::cout
        << "Measured runs: "
        << NUMBER_OF_RUNS
        << "\n\n";


    // =================================================
    // GENERATE RESTING SELL ORDERS
    // =================================================

    std::vector<Order> restingOrders;
    restingOrders.reserve(TOTAL_ORDERS);

    for(std::size_t i = 0;
        i < TOTAL_ORDERS;
        ++i)
    {
        restingOrders.push_back(
            makeOrder(
                static_cast<uint64_t>(i + 1),
                Side::SELL
            )
        );
    }


    // =================================================
    // GENERATE INCOMING BUY ORDERS
    // =================================================

    std::vector<Order> incomingOrders;
    incomingOrders.reserve(TOTAL_ORDERS);

    for(std::size_t i = 0;
        i < TOTAL_ORDERS;
        ++i)
    {
        incomingOrders.push_back(
            makeOrder(
                static_cast<uint64_t>(
                    1'000'000 + i
                ),
                Side::BUY
            )
        );
    }


    // =================================================
    // STORE EACH RUN'S PERCENTILES
    // =================================================

    std::vector<double> runP50s;
    std::vector<double> runP95s;
    std::vector<double> runP99s;

    runP50s.reserve(NUMBER_OF_RUNS);
    runP95s.reserve(NUMBER_OF_RUNS);
    runP99s.reserve(NUMBER_OF_RUNS);


    // =================================================
    // 10 INDEPENDENT RUNS
    // =================================================

    for(std::size_t run = 0;
        run < NUMBER_OF_RUNS;
        ++run)
    {
        MatchingEngine engine(TOTAL_ORDERS);

        /*
            One reusable V2 trade buffer for the entire
            MatchingEngine run.
        */
        std::vector<Trade> tradeBuffer;


        // =============================================
        // PRELOAD RESTING ORDERS - NOT TIMED
        // =============================================

        for(const Order& order : restingOrders){
            engine.processOrder(
                order,
                tradeBuffer
            );
        }


        /*
            Each element stores the average matching
            latency per order inside one batch of
            100 incoming orders.
        */
        std::vector<double> batchLatencies;
        batchLatencies.reserve(NUMBER_OF_BATCHES);


        // =============================================
        // MEASURE 1,000 MATCHING BATCHES
        // =============================================

        for(std::size_t batch = 0;
            batch < NUMBER_OF_BATCHES;
            ++batch)
        {
            const std::size_t startIndex =
                batch * BATCH_SIZE;


            const auto start = Clock::now();


            for(std::size_t i = 0;
                i < BATCH_SIZE;
                ++i)
            {
                engine.processOrder(
                    incomingOrders[startIndex + i],
                    tradeBuffer
                );
            }


            const auto end = Clock::now();


            const double batchNanoseconds =
                std::chrono::duration<double, std::nano>(
                    end - start
                ).count();


            /*
                Convert the time for the entire
                100-order batch into average
                nanoseconds per incoming order.
            */
            const double nsPerIncoming =
                batchNanoseconds /
                static_cast<double>(BATCH_SIZE);


            batchLatencies.push_back(
                nsPerIncoming
            );
        }


        // =============================================
        // PERCENTILES FOR THIS RUN
        // =============================================

        const double p50 =
            percentile(batchLatencies, 0.50);

        const double p95 =
            percentile(batchLatencies, 0.95);

        const double p99 =
            percentile(batchLatencies, 0.99);


        runP50s.push_back(p50);
        runP95s.push_back(p95);
        runP99s.push_back(p99);


        std::cout
            << "Run " << run + 1
            << ": "
            << "p50 = " << p50 << " ns/order"
            << " | "
            << "p95 = " << p95 << " ns/order"
            << " | "
            << "p99 = " << p99 << " ns/order"
            << "\n";
    }


    // =================================================
    // FINAL RESULTS
    // =================================================

    const double finalP50 =
        median(runP50s);

    const double finalP95 =
        median(runP95s);

    const double finalP99 =
        median(runP99s);


    std::cout << "\n========================================\n";
    std::cout << "FINAL RESULT\n";
    std::cout << "========================================\n";

    std::cout
        << "Median p50: "
        << finalP50
        << " ns/order\n";

    std::cout
        << "Median p95: "
        << finalP95
        << " ns/order\n";

    std::cout
        << "Median p99: "
        << finalP99
        << " ns/order\n";


    return 0;
}