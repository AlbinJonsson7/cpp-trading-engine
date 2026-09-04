#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include "trading/matching_engine.hpp"
#include "trading/order.hpp"
#include "trading/trade.hpp"


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


void printOrder(const Order& order)
{
    std::cout
        << "Order "
        << order.orderID
        << " | "
        << (order.side == Side::BUY ? "BUY " : "SELL")
        << " | Price: $"
        << std::fixed
        << std::setprecision(2)
        << static_cast<double>(order.price) / 100.0
        << " | Quantity: "
        << order.originalQuantity
        << "\n";
}


void printTrades(const std::vector<Trade>& trades)
{
    if(trades.empty()){
        std::cout << "No trades generated. Order rests in book.\n";
        return;
    }

    for(const Trade& trade : trades){

        std::cout
            << "TRADE"
            << " | Buy Order: "
            << trade.buyOrderID
            << " | Sell Order: "
            << trade.sellOrderID
            << " | Price: $"
            << std::fixed
            << std::setprecision(2)
            << static_cast<double>(trade.price) / 100.0
            << " | Quantity: "
            << trade.quantity
            << "\n";
    }
}


int main()
{
    MatchingEngine engine(100);

    std::vector<Trade> trades;


    std::cout << "========================================\n";
    std::cout << "C++ TRADING ENGINE DEMO\n";
    std::cout << "========================================\n\n";


    // =================================================
    // 1. ADD RESTING SELL ORDERS
    // =================================================

    std::cout << "--- Adding Resting Sell Orders ---\n\n";


    Order sell1 =
        makeLimitOrder(
            1,
            Side::SELL,
            10'200,
            10
        );

    printOrder(sell1);

    engine.processOrder(
        sell1,
        trades
    );

    printTrades(trades);

    std::cout << "\n";


    Order sell2 =
        makeLimitOrder(
            2,
            Side::SELL,
            10'200,
            15
        );

    printOrder(sell2);

    engine.processOrder(
        sell2,
        trades
    );

    printTrades(trades);

    std::cout << "\n";


    Order sell3 =
        makeLimitOrder(
            3,
            Side::SELL,
            10'300,
            20
        );

    printOrder(sell3);

    engine.processOrder(
        sell3,
        trades
    );

    printTrades(trades);


    // =================================================
    // 2. SUBMIT CROSSING BUY ORDER
    // =================================================

    std::cout << "\n";
    std::cout << "--- Submitting Crossing Buy Order ---\n\n";


    Order buy =
        makeLimitOrder(
            4,
            Side::BUY,
            10'300,
            30
        );

    printOrder(buy);


    engine.processOrder(
        buy,
        trades
    );


    std::cout << "\nGenerated trades:\n";

    printTrades(trades);


    // =================================================
    // 3. EXPLAIN RESULT
    // =================================================

    std::cout << "\n";
    std::cout << "--- Matching Result ---\n\n";

    std::cout
        << "BUY 4 requested 30 units at a limit of $103.00.\n"
        << "The engine matched the best available prices first.\n\n"

        << "SELL 1: 10 units at $102.00\n"
        << "SELL 2: 15 units at $102.00\n"
        << "SELL 3:  5 units at $103.00\n\n"

        << "Orders 1 and 2 share the same price, so Order 1\n"
        << "executes first because it arrived first (FIFO).\n\n"

        << "SELL 3 originally contained 20 units, so after\n"
        << "executing 5 units it still has 15 units resting.\n";


    // =================================================
    // 4. CANCEL PARTIALLY FILLED RESTING ORDER
    // =================================================

    std::cout << "\n";
    std::cout << "--- Cancelling Remaining Order ---\n\n";


    if(engine.cancelOrder(3)){

        std::cout
            << "Order 3 cancelled successfully.\n";

    }else{

        std::cout
            << "Order 3 could not be cancelled.\n";
    }


    // =================================================
    // COMPLETE
    // =================================================

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "DEMO COMPLETE\n";
    std::cout << "========================================\n";


    return 0;
}