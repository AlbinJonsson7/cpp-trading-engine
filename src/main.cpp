#include <iostream>
#include <vector>
#include <string>
#include "trading/order.hpp"

using namespace std;

int main(){

    Order myOrder;

    myOrder.orderID = 1;
    myOrder.side = Side::BUY;
    myOrder.price = 125;
    myOrder.originalQuantity = 40;
    myOrder.remainingQuantity = 30;

    std::cout << "OrderID: " << myOrder.orderID
              << " Price: " << myOrder.price
              << " Side: " << toString(myOrder.side)
              << " Original Quantity: " << myOrder.originalQuantity
              << " Remaining Quantity: " << myOrder.remainingQuantity
              << '\n';

    std::cout << std::endl;
}
