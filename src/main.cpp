#include <iostream>
#include <vector>
#include <string>
#include "trading/order.hpp"
#include "trading/price_level.hpp"
#include "trading/order_book.hpp"

using namespace std;

int main(){

    Order order1;
    order1.orderID = 1;
    order1.side = Side::BUY;
    order1.price = 10000;
    order1.originalQuantity = 20;
    order1.remainingQuantity = 20;

    Order order2;
    order2.orderID = 2;
    order2.side = Side::BUY;
    order2.price = 9900;
    order2.originalQuantity = 30;
    order2.remainingQuantity = 30;

    Order order3;
    order3.orderID = 3;
    order3.side = Side::BUY;
    order3.price = 10100;
    order3.originalQuantity = 40;
    order3.remainingQuantity = 40;

    Order order4;
    order4.orderID = 4;
    order4.side = Side::SELL;
    order4.price = 10300;
    order4.originalQuantity = 15;
    order4.remainingQuantity = 15;

    Order order5;
    order5.orderID = 5;
    order5.side = Side::SELL;
    order5.price = 10200;
    order5.originalQuantity = 25;
    order5.remainingQuantity = 25;

    Order order6;
    order6.orderID = 6;
    order6.side = Side::SELL;
    order6.price = 10400;
    order6.originalQuantity = 35;   
    order6.remainingQuantity = 35;

    OrderBook orderBook;
    orderBook.addOrder(order1);
    orderBook.addOrder(order2);
    orderBook.addOrder(order3);
    

    auto bestBid = orderBook.getBestBid();
    auto bestAsk = orderBook.getBestAsk();

    if(bestBid){
        cout << "Best Bid Price: " << *bestBid << endl;
    }

    if(bestAsk){
        cout << "Best Ask Price: " << *bestAsk << endl;
    }else{
        cout << "No asks available." << endl;
    }

}
