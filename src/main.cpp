#include <iostream>
#include <vector>
#include <string>
#include "trading/order.hpp"
#include "trading/price_level.hpp"

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
    order2.price = 10000;
    order2.originalQuantity = 30;
    order2.remainingQuantity = 30;

    Order order3;
    order3.orderID = 3;
    order3.side = Side::BUY;
    order3.price = 10000;
    order3.originalQuantity = 40;
    order3.remainingQuantity = 40;

    PriceLevel priceLevel(10000);
    priceLevel.addOrder(order1);
    priceLevel.addOrder(order2);
    priceLevel.addOrder(order3);

    cout << "Front Order ID: " << priceLevel.getFrontOrder().orderID;
    priceLevel.removeFrontOrder();
    cout << "\nFront Order ID after removal: " << priceLevel.getFrontOrder().orderID;
    priceLevel.removeOrder(3);
    cout << "\nFront Order ID after removal: " << priceLevel.getFrontOrder().orderID;
    priceLevel.removeFrontOrder();
    cout << "\nIs empty: " << priceLevel.isEmpty();

}
