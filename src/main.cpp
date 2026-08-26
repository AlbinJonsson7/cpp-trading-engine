#include <iostream>
#include <vector>
#include <string>
#include "trading/trade.hpp"
#include "trading/order.hpp"
#include "trading/price_level.hpp"
#include "trading/order_book.hpp"
#include "trading/matching_engine.hpp"

using namespace std;


int main() {

    std::cout << "--- Test: Matching Engine Regression ---\n";

    MatchingEngine engine;

    // Resting SELL #10: 10 @ 10200
    Order sell10 = {
        10,
        Side::SELL,
        10200,
        10,
        10,
        OrderType::LIMIT
    };

    // Resting SELL #11: 15 @ 10200
    Order sell11 = {
        11,
        Side::SELL,
        10200,
        15,
        15,
        OrderType::LIMIT
    };

    // Resting SELL #12: 20 @ 10300
    Order sell12 = {
        12,
        Side::SELL,
        10300,
        20,
        20,
        OrderType::LIMIT
    };

    engine.processOrder(sell10);
    engine.processOrder(sell11);
    engine.processOrder(sell12);


    // Incoming BUY #20: 30 @ 10300
    Order buy20 = {
        20,
        Side::BUY,
        10300,
        30,
        30,
        OrderType::LIMIT
    };

    ProcessResult result = engine.processOrder(buy20);


    std::cout << "Trades created: "
              << result.trades.size() << '\n';

    for (const Trade& trade : result.trades) {

        std::cout << "Buy Order ID: "
                  << trade.buyOrderID << '\n';

        std::cout << "Sell Order ID: "
                  << trade.sellOrderID << '\n';

        std::cout << "Price: "
                  << trade.price << '\n';

        std::cout << "Quantity: "
                  << trade.quantity << '\n';

        std::cout << "-------------------\n";
    }


    // Now send another BUY to verify that #12
    // remained in the book with 15 quantity remaining.

    std::cout << "\n--- Follow-up Test: Remaining Quantity ---\n";

    Order buy21 = {
        21,
        Side::BUY,
        10300,
        15,
        15,
        OrderType::LIMIT
    };

    ProcessResult result2 = engine.processOrder(buy21);

    std::cout << "Trades created: "
              << result2.trades.size() << '\n';

    for (const Trade& trade : result2.trades) {

        std::cout << "Buy Order ID: "
                  << trade.buyOrderID << '\n';

        std::cout << "Sell Order ID: "
                  << trade.sellOrderID << '\n';

        std::cout << "Price: "
                  << trade.price << '\n';

        std::cout << "Quantity: "
                  << trade.quantity << '\n';

        std::cout << "-------------------\n";
    }

    return 0;
}