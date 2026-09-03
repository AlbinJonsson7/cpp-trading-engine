#include <cstdint>
#include <iostream>

#include "trading/order.hpp"
#include "trading/order_node_pool.hpp"


Order makeOrder(uint64_t orderID, int64_t price)
{
    Order order{};

    order.orderID = orderID;
    order.side = Side::BUY;
    order.price = price;
    order.originalQuantity = 100;
    order.remainingQuantity = 100;
    order.orderType = OrderType::LIMIT;

    return order;
}


int main()
{
    std::cout << "========================================\n";
    std::cout << "ORDER NODE POOL CORRECTNESS TESTS\n";
    std::cout << "========================================\n\n";


    OrderNodePool pool(100);


    // -------------------------------------------------
    // Test 1: first acquire should use index 0
    // -------------------------------------------------

    Order order1 = makeOrder(1, 10000);

    uint32_t index1 = pool.acquire(order1);

    if(index1 != 0){
        std::cout << "First acquire index: FAIL\n";
        return 1;
    }

    std::cout << "First acquire index: PASS\n";


    // -------------------------------------------------
    // Test 2: second acquire should use index 1
    // -------------------------------------------------

    Order order2 = makeOrder(2, 10100);

    uint32_t index2 = pool.acquire(order2);

    if(index2 != 1){
        std::cout << "Second acquire index: FAIL\n";
        return 1;
    }

    std::cout << "Second acquire index: PASS\n";


    // -------------------------------------------------
    // Test 3: stored Order data is correct
    // -------------------------------------------------

    OrderNode& node1 = pool.get(index1);

    if(
        node1.order.orderID != 1 ||
        node1.order.price != 10000 ||
        node1.active != true
    ){
        std::cout << "Stored Order data: FAIL\n";
        return 1;
    }

    std::cout << "Stored Order data: PASS\n";


    // -------------------------------------------------
    // Test 4: fresh node starts detached
    // -------------------------------------------------

    if(
        node1.previous != INVALID_NODE_INDEX ||
        node1.next != INVALID_NODE_INDEX
    ){
        std::cout << "Fresh node links reset: FAIL\n";
        return 1;
    }

    std::cout << "Fresh node links reset: PASS\n";


    // -------------------------------------------------
    // Give the node fake links.
    //
    // This lets us verify release() clears them.
    // -------------------------------------------------

    node1.previous = 50;
    node1.next = 60;


    // -------------------------------------------------
    // Test 5: releasing active node succeeds
    // -------------------------------------------------

    if(!pool.release(index1)){
        std::cout << "Release active node: FAIL\n";
        return 1;
    }

    std::cout << "Release active node: PASS\n";


    // -------------------------------------------------
    // Test 6: double release must fail
    // -------------------------------------------------

    if(pool.release(index1)){
        std::cout << "Double release rejection: FAIL\n";
        return 1;
    }

    std::cout << "Double release rejection: PASS\n";


    // -------------------------------------------------
    // Test 7: invalid index must fail
    // -------------------------------------------------

    if(pool.release(999)){
        std::cout << "Invalid index rejection: FAIL\n";
        return 1;
    }

    std::cout << "Invalid index rejection: PASS\n";


    // -------------------------------------------------
    // Test 8: released index should be reused
    // -------------------------------------------------

    Order order3 = makeOrder(3, 10200);

    uint32_t index3 = pool.acquire(order3);

    if(index3 != index1){
        std::cout << "Released index reuse: FAIL\n";
        return 1;
    }

    std::cout << "Released index reuse: PASS\n";


    // -------------------------------------------------
    // Test 9: reused node contains NEW Order
    // -------------------------------------------------

    OrderNode& reusedNode = pool.get(index3);

    if(
        reusedNode.order.orderID != 3 ||
        reusedNode.order.price != 10200
    ){
        std::cout << "Reused node Order replacement: FAIL\n";
        return 1;
    }

    std::cout << "Reused node Order replacement: PASS\n";


    // -------------------------------------------------
    // Test 10: reused node is active again
    // -------------------------------------------------

    if(!reusedNode.active){
        std::cout << "Reused node active state: FAIL\n";
        return 1;
    }

    std::cout << "Reused node active state: PASS\n";


    // -------------------------------------------------
    // Test 11: old links must not survive reuse
    // -------------------------------------------------

    if(
        reusedNode.previous != INVALID_NODE_INDEX ||
        reusedNode.next != INVALID_NODE_INDEX
    ){
        std::cout << "Reused node link reset: FAIL\n";
        return 1;
    }

    std::cout << "Reused node link reset: PASS\n";


    // -------------------------------------------------
    // Test 12: second original node is unaffected
    // -------------------------------------------------

    const OrderNode& node2 = pool.get(index2);

    if(
        node2.order.orderID != 2 ||
        node2.order.price != 10100 ||
        !node2.active
    ){
        std::cout << "Other node isolation: FAIL\n";
        return 1;
    }

    std::cout << "Other node isolation: PASS\n";


    std::cout << "\n========================================\n";
    std::cout << "ALL ORDER NODE POOL TESTS PASSED\n";
    std::cout << "========================================\n";

    return 0;
}