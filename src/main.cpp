#include <iostream>
#include <vector>
#include <string>
#include "trading/trade.hpp"
#include "trading/order.hpp"
#include "trading/price_level.hpp"
#include "trading/order_book.hpp"
#include "trading/matching_engine.hpp"

using namespace std;


int main(){

    // =====================================================
    // TEST 1: Quantity exactly at the limit
    // Expected: ACCEPTED
    // =====================================================
    {
        cout << "\n--- Test 1: Quantity At Limit ---" << endl;

        MatchingEngine engine;

        Order order;
        order.orderID = 400;
        order.side = Side::BUY;
        order.price = 10000;
        order.originalQuantity = 1000000;
        order.remainingQuantity = 1000000;
        order.orderType = OrderType::LIMIT;

        ProcessResult result = engine.processOrder(order);

        cout << "Trades created: " << result.trades.size() << endl;

        if(result.status == ProcessStatus::ACCEPTED){
            cout << "Status: ACCEPTED" << endl;
        }
    }


    // =====================================================
    // TEST 2: Quantity above the limit
    // Expected: REJECTED_QUANTITY_TOO_LARGE
    // =====================================================
    {
        cout << "\n--- Test 2: Quantity Above Limit ---" << endl;

        MatchingEngine engine;

        Order tooLarge;
        tooLarge.orderID = 500;
        tooLarge.side = Side::BUY;
        tooLarge.price = 10000;
        tooLarge.originalQuantity = 1000001;
        tooLarge.remainingQuantity = 1000001;
        tooLarge.orderType = OrderType::LIMIT;

        ProcessResult result = engine.processOrder(tooLarge);

        cout << "Trades created: " << result.trades.size() << endl;

        if(result.status == ProcessStatus::REJECTED_QUANTITY_TOO_LARGE){
            cout << "Status: REJECTED_QUANTITY_TOO_LARGE" << endl;
        }


        // This SELL would cross BUY #500 if it had incorrectly
        // entered the order book.
        Order sell;
        sell.orderID = 501;
        sell.side = Side::SELL;
        sell.price = 9000;
        sell.originalQuantity = 10;
        sell.remainingQuantity = 10;
        sell.orderType = OrderType::LIMIT;

        ProcessResult secondResult = engine.processOrder(sell);

        cout << "Trades against rejected order: "
             << secondResult.trades.size() << endl;
    }

    return 0;
}