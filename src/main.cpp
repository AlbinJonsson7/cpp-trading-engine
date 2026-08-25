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
    // TEST 1: Cancel an order from the middle of a PriceLevel
    //
    // FIFO before cancellation:
    // #1 -> #2 -> #3
    //
    // Cancel #2
    //
    // Expected:
    // #1 -> #3
    // =====================================================
    {
        cout << "\n--- Test 1: Cancel Middle Order ---" << endl;

        OrderBook book;

        Order order1 {1, Side::BUY, 10000, 10, 10};
        Order order2 {2, Side::BUY, 10000, 20, 20};
        Order order3 {3, Side::BUY, 10000, 30, 30};

        book.addOrder(order1);
        book.addOrder(order2);
        book.addOrder(order3);

        bool cancelled = book.cancelOrder(2);

        cout << "Cancel Order #2: "
             << (cancelled ? "success" : "failed") << endl;

        // #1 should still be first
        auto firstOrder = book.getBestBidOrder();

        if(firstOrder){
            cout << "Front Order after cancellation: #"
                 << firstOrder->orderID << endl;
        }

        // Remove #1 completely.
        // If #2 was really cancelled, #3 should now be first.
        book.fillBestOrder(Side::BUY, 10);

        auto nextOrder = book.getBestBidOrder();

        if(nextOrder){
            cout << "Next Order: #"
                 << nextOrder->orderID << endl;
        }
    }


    // =====================================================
    // TEST 2: Cancel the only order at a price
    //
    // Expected:
    // Order removed
    // PriceLevel removed
    // No best ask remains
    // =====================================================
    {
        cout << "\n--- Test 2: Remove Entire Price Level ---" << endl;

        OrderBook book;

        Order order {10, Side::SELL, 10200, 25, 25};

        book.addOrder(order);

        bool cancelled = book.cancelOrder(10);

        cout << "Cancel Order #10: "
             << (cancelled ? "success" : "failed") << endl;

        auto bestAsk = book.getBestAsk();

        if(!bestAsk){
            cout << "Ask PriceLevel removed successfully." << endl;
        }else{
            cout << "ERROR: Ask still exists at "
                 << *bestAsk << endl;
        }
    }


    // =====================================================
    // TEST 3: Cancel an order that doesn't exist
    //
    // Expected:
    // cancelOrder() returns false
    // =====================================================
    {
        cout << "\n--- Test 3: Nonexistent Order ---" << endl;

        OrderBook book;

        Order order {20, Side::BUY, 9900, 15, 15};

        book.addOrder(order);

        bool cancelled = book.cancelOrder(999);

        cout << "Cancel Order #999: "
             << (cancelled ? "success" : "not found") << endl;
    }

    return 0;
}
