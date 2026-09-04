/********************************************************************************************************************************
*                                                                                                                               *
*                                           TEST 9: MIXED ADD + MATCH + CANCEL WORKLOAD                                          *
*                                                                                                                               *
*********************************************************************************************************************************/

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

inline constexpr std::size_t NUMBER_OF_CYCLES = 250'000;
inline constexpr std::size_t OPERATIONS_PER_CYCLE = 4;

inline constexpr std::size_t TOTAL_OPERATIONS =
    NUMBER_OF_CYCLES *
    OPERATIONS_PER_CYCLE;

inline constexpr std::size_t TOTAL_ADDS =
    NUMBER_OF_CYCLES * 2;

inline constexpr std::size_t TOTAL_MATCHES =
    NUMBER_OF_CYCLES;

inline constexpr std::size_t TOTAL_CANCELS =
    NUMBER_OF_CYCLES;

inline constexpr std::size_t NUMBER_OF_RUNS = 10;

inline constexpr std::size_t NUMBER_OF_PRICE_LEVELS = 100;

inline constexpr int64_t BASE_PRICE = 10'000;
inline constexpr uint32_t QUANTITY = 1;


// =====================================================
// OPERATION DESCRIPTION
// =====================================================

enum class OperationType {
    ADD,
    MATCH,
    CANCEL
};


struct Operation {
    OperationType type;
    Order order{};
    uint64_t cancelOrderID = 0;
};


// =====================================================
// CREATE RESTING SELL ORDER
// =====================================================

Order makeRestingOrder(
    uint64_t orderID,
    int64_t price)
{
    Order order{};

    order.orderID = orderID;
    order.side = Side::SELL;
    order.price = price;

    order.originalQuantity = QUANTITY;
    order.remainingQuantity = QUANTITY;

    order.orderType = OrderType::LIMIT;

    return order;
}


// =====================================================
// CREATE MATCHING BUY ORDER
// =====================================================

Order makeMatchingOrder(
    uint64_t orderID,
    int64_t price)
{
    Order order{};

    order.orderID = orderID;
    order.side = Side::BUY;
    order.price = price;

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
    std::sort(
        values.begin(),
        values.end()
    );

    const std::size_t size =
        values.size();

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
    std::cout
        << std::fixed
        << std::setprecision(6);


    std::cout
        << "========================================\n";

    std::cout
        << "TEST 9: MIXED ADD + MATCH + CANCEL WORKLOAD\n";

    std::cout
        << "========================================\n";


    std::cout
        << "Cycles per run: "
        << NUMBER_OF_CYCLES
        << "\n";


    std::cout
        << "Operations per cycle: "
        << OPERATIONS_PER_CYCLE
        << "\n";


    std::cout
        << "Total operations per run: "
        << TOTAL_OPERATIONS
        << "\n";


    std::cout
        << "Adds per run: "
        << TOTAL_ADDS
        << "\n";


    std::cout
        << "Matches per run: "
        << TOTAL_MATCHES
        << "\n";


    std::cout
        << "Cancels per run: "
        << TOTAL_CANCELS
        << "\n";


    std::cout
        << "Price levels: "
        << NUMBER_OF_PRICE_LEVELS
        << "\n";


    std::cout
        << "Measured runs: "
        << NUMBER_OF_RUNS
        << "\n\n";


    // =================================================
    // PRE-GENERATE OPERATION STREAM
    //
    // Every cycle:
    //
    // 1. ADD SELL A
    // 2. ADD SELL B
    // 3. BUY matches SELL A
    // 4. CANCEL SELL B
    //
    // The book returns to empty after every cycle.
    //
    // Submitted Order IDs are strictly increasing:
    //
    // Cycle 0:
    //     SELL 1
    //     SELL 2
    //     BUY  3
    //
    // Cycle 1:
    //     SELL 4
    //     SELL 5
    //     BUY  6
    //
    // etc.
    //
    // CANCEL does not submit a new Order ID.
    // =================================================

    std::vector<Operation> operations;

    operations.reserve(
        TOTAL_OPERATIONS
    );


    uint64_t nextOrderID = 1;


    for(std::size_t cycle = 0;
        cycle < NUMBER_OF_CYCLES;
        ++cycle)
    {
        const int64_t price =
            BASE_PRICE +
            static_cast<int64_t>(
                cycle %
                NUMBER_OF_PRICE_LEVELS
            );


        // ---------------------------------------------
        // ADD resting SELL A
        // ---------------------------------------------

        const uint64_t firstSellID =
            nextOrderID++;


        Operation addFirst{};

        addFirst.type =
            OperationType::ADD;

        addFirst.order =
            makeRestingOrder(
                firstSellID,
                price
            );


        operations.push_back(
            addFirst
        );


        // ---------------------------------------------
        // ADD resting SELL B
        // ---------------------------------------------

        const uint64_t secondSellID =
            nextOrderID++;


        Operation addSecond{};

        addSecond.type =
            OperationType::ADD;

        addSecond.order =
            makeRestingOrder(
                secondSellID,
                price
            );


        operations.push_back(
            addSecond
        );


        // ---------------------------------------------
        // MATCH first resting SELL
        //
        // Since both SELLs are at the same price,
        // FIFO means SELL A is matched first.
        // ---------------------------------------------

        Operation match{};

        match.type =
            OperationType::MATCH;

        match.order =
            makeMatchingOrder(
                nextOrderID++,
                price
            );


        operations.push_back(
            match
        );


        // ---------------------------------------------
        // CANCEL remaining SELL B
        // ---------------------------------------------

        Operation cancel{};

        cancel.type =
            OperationType::CANCEL;

        cancel.cancelOrderID =
            secondSellID;


        operations.push_back(
            cancel
        );
    }


    // =================================================
    // STORE RUN TIMES
    // =================================================

    std::vector<double> runTimes;

    runTimes.reserve(
        NUMBER_OF_RUNS
    );


    // =================================================
    // 10 INDEPENDENT RUNS
    // =================================================

    for(std::size_t run = 0;
        run < NUMBER_OF_RUNS;
        ++run)
    {
        /*
            Fresh engine every run.

            Construction is outside timing.
        */

        MatchingEngine engine(
            TOTAL_OPERATIONS
        );


        /*
            Reusable trade buffer.

            Same approach as Tests 1-8.
        */

        std::vector<Trade> tradeBuffer;


        // =============================================
        // TIMED MIXED WORKLOAD
        // =============================================

        const auto start =
            Clock::now();


        for(const Operation& operation : operations){

            if(operation.type == OperationType::ADD){

                engine.processOrder(
                    operation.order,
                    tradeBuffer
                );

            }else if(operation.type == OperationType::MATCH){

                engine.processOrder(
                    operation.order,
                    tradeBuffer
                );

            }else{

                engine.cancelOrder(
                    operation.cancelOrderID
                );
            }
        }


        const auto end =
            Clock::now();


        // =============================================
        // RESULTS
        // =============================================

        const double seconds =
            std::chrono::duration<double>(
                end - start
            ).count();


        runTimes.push_back(
            seconds
        );


        const double throughput =
            static_cast<double>(
                TOTAL_OPERATIONS
            ) / seconds;


        const double nsPerOperation =
            seconds * 1e9 /
            static_cast<double>(
                TOTAL_OPERATIONS
            );


        std::cout
            << "Run "
            << run + 1
            << ": "
            << seconds
            << " s"
            << " | "
            << throughput
            << " ops/s"
            << " | "
            << nsPerOperation
            << " ns/op"
            << "\n";
    }


    // =================================================
    // FINAL MEDIAN
    // =================================================

    const double medianSeconds =
        median(
            runTimes
        );


    const double medianThroughput =
        static_cast<double>(
            TOTAL_OPERATIONS
        ) / medianSeconds;


    const double medianNsPerOperation =
        medianSeconds * 1e9 /
        static_cast<double>(
            TOTAL_OPERATIONS
        );


    std::cout
        << "\n========================================\n";

    std::cout
        << "FINAL RESULT\n";

    std::cout
        << "========================================\n";


    std::cout
        << "Median: "
        << medianSeconds
        << " s\n";


    std::cout
        << "Throughput: "
        << medianThroughput
        << " ops/s\n";


    std::cout
        << "Cost: "
        << medianNsPerOperation
        << " ns/op\n";


    return 0;
}