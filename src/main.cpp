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

    MatchingEngine engine;

    // Resting BUY #1: 20 @ 10100
    Order buy1;
    buy1.orderID = 1;
    buy1.side = Side::BUY;
    buy1.price = 10100;
    buy1.originalQuantity = 20;
    buy1.remainingQuantity = 20;

    // Resting BUY #2: 25 @ 10000
    Order buy2;
    buy2.orderID = 2;
    buy2.side = Side::BUY;
    buy2.price = 10000;
    buy2.originalQuantity = 25;
    buy2.remainingQuantity = 25;

    // Resting BUY #3: 30 @ 9900
    Order buy3;
    buy3.orderID = 3;
    buy3.side = Side::BUY;
    buy3.price = 9900;
    buy3.originalQuantity = 30;
    buy3.remainingQuantity = 30;

    // These should enter the book because there are no SELL orders yet
    engine.processOrder(buy1);
    engine.processOrder(buy2);
    engine.processOrder(buy3);

    // Incoming SELL #8: 35 @ 10000
    Order incomingSell;
    incomingSell.orderID = 8;
    incomingSell.side = Side::SELL;
    incomingSell.price = 10000;
    incomingSell.originalQuantity = 35;
    incomingSell.remainingQuantity = 35;

    vector<Trade> trades = engine.processOrder(incomingSell);

    cout << "Number of trades: " << trades.size() << endl;

    for(const Trade& trade : trades){
        cout << "Buy Order ID: " << trade.buyOrderID << endl;
        cout << "Sell Order ID: " << trade.sellOrderID << endl;
        cout << "Price: " << trade.price << endl;
        cout << "Quantity: " << trade.quantity << endl;
        cout << "-------------------" << endl;
    }

    return 0;
}
