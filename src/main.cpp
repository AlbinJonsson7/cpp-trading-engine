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
    // TEST 1: MARKET BUY walks across multiple ask levels
    //
    // Asks:
    // 10200 -> 10
    // 10300 -> 15
    // 10600 -> 20
    //
    // MARKET BUY 30
    //
    // Expected:
    // 10 @ 10200
    // 15 @ 10300
    // 5  @ 10600
    // =====================================================
    {
        cout << "\n--- Test 1: Market BUY Across Levels ---" << endl;

        MatchingEngine engine;

        Order sell1 {1, Side::SELL, 10200, 10, 10};
        Order sell2 {2, Side::SELL, 10300, 15, 15};
        Order sell3 {3, Side::SELL, 10600, 20, 20};

        engine.processOrder(sell1);
        engine.processOrder(sell2);
        engine.processOrder(sell3);

        Order marketBuy;
        marketBuy.orderID = 50;
        marketBuy.side = Side::BUY;
        marketBuy.price = 0; // ignored for MARKET
        marketBuy.originalQuantity = 30;
        marketBuy.remainingQuantity = 30;
        marketBuy.orderType = OrderType::MARKET;

        vector<Trade> trades = engine.processOrder(marketBuy);

        cout << "Number of trades: " << trades.size() << endl;

        for(const Trade& trade : trades){
            cout << "Buy Order ID: " << trade.buyOrderID << endl;
            cout << "Sell Order ID: " << trade.sellOrderID << endl;
            cout << "Price: " << trade.price << endl;
            cout << "Quantity: " << trade.quantity << endl;
            cout << "-------------------" << endl;
        }
    }


    // =====================================================
    // TEST 2: MARKET BUY with insufficient liquidity
    //
    // Available:
    // 10200 -> 10
    // 10300 -> 15
    //
    // Total available = 25
    // MARKET BUY = 40
    //
    // Expected:
    // executes 25
    // remaining 15 is cancelled
    // =====================================================
    {
        cout << "\n--- Test 2: Market BUY Insufficient Liquidity ---" << endl;

        MatchingEngine engine;

        Order sell1 {10, Side::SELL, 10200, 10, 10};
        Order sell2 {11, Side::SELL, 10300, 15, 15};

        engine.processOrder(sell1);
        engine.processOrder(sell2);

        Order marketBuy;
        marketBuy.orderID = 60;
        marketBuy.side = Side::BUY;
        marketBuy.price = 0;
        marketBuy.originalQuantity = 40;
        marketBuy.remainingQuantity = 40;
        marketBuy.orderType = OrderType::MARKET;

        vector<Trade> trades = engine.processOrder(marketBuy);

        cout << "Number of trades: " << trades.size() << endl;

        for(const Trade& trade : trades){
            cout << "Buy Order ID: " << trade.buyOrderID << endl;
            cout << "Sell Order ID: " << trade.sellOrderID << endl;
            cout << "Price: " << trade.price << endl;
            cout << "Quantity: " << trade.quantity << endl;
            cout << "-------------------" << endl;
        }

        // If the leftover 15 was correctly cancelled,
        // this SELL should NOT find BUY #60 resting in the book.
        Order testSell;
        testSell.orderID = 61;
        testSell.side = Side::SELL;
        testSell.price = 1;
        testSell.originalQuantity = 15;
        testSell.remainingQuantity = 15;

        vector<Trade> leftoverTest = engine.processOrder(testSell);

        cout << "Trades against cancelled remainder: "
             << leftoverTest.size() << endl;
    }


    // =====================================================
    // TEST 3: MARKET SELL walks across bid levels
    //
    // Bids:
    // 10100 -> 10
    // 10000 -> 15
    //  9800 -> 20
    //
    // MARKET SELL 30
    //
    // Expected:
    // 10 @ 10100
    // 15 @ 10000
    // 5  @ 9800
    // =====================================================
    {
        cout << "\n--- Test 3: Market SELL Across Levels ---" << endl;

        MatchingEngine engine;

        Order buy1 {20, Side::BUY, 10100, 10, 10};
        Order buy2 {21, Side::BUY, 10000, 15, 15};
        Order buy3 {22, Side::BUY, 9800, 20, 20};

        engine.processOrder(buy1);
        engine.processOrder(buy2);
        engine.processOrder(buy3);

        Order marketSell;
        marketSell.orderID = 70;
        marketSell.side = Side::SELL;
        marketSell.price = 999999; // ignored for MARKET
        marketSell.originalQuantity = 30;
        marketSell.remainingQuantity = 30;
        marketSell.orderType = OrderType::MARKET;

        vector<Trade> trades = engine.processOrder(marketSell);

        cout << "Number of trades: " << trades.size() << endl;

        for(const Trade& trade : trades){
            cout << "Buy Order ID: " << trade.buyOrderID << endl;
            cout << "Sell Order ID: " << trade.sellOrderID << endl;
            cout << "Price: " << trade.price << endl;
            cout << "Quantity: " << trade.quantity << endl;
            cout << "-------------------" << endl;
        }
    }

    return 0;
}
