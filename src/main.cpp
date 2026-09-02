#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>

#include "trading/order_index.hpp"
#include "trading/matching_engine.hpp"
#include "trading/order.hpp"
#include "trading/trade.hpp"


// =====================================================
// ORDER HELPERS
// =====================================================

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


Order makeMarketOrder(
    uint64_t orderID,
    Side side,
    uint32_t quantity)
{
    Order order{};

    order.orderID = orderID;
    order.side = side;
    order.price = 0;
    order.originalQuantity = quantity;
    order.remainingQuantity = quantity;
    order.orderType = OrderType::MARKET;

    return order;
}


// =====================================================
// FIND IDs THAT HASH TO A SPECIFIC BUCKET
// =====================================================

std::vector<uint64_t> findIdsForBucket(
    std::size_t bucket,
    std::size_t mask,
    std::size_t amount)
{
    std::vector<uint64_t> ids;
    ids.reserve(amount);

    uint64_t candidate = 1;

    while(ids.size() < amount){

        const std::size_t home =
            std::hash<uint64_t>{}(candidate) & mask;

        if(home == bucket){
            ids.push_back(candidate);
        }

        candidate++;
    }

    return ids;
}


// =====================================================
// ORDERINDEX TEST 1
// BASIC FIND + ERASEAT
// =====================================================

void testBasicEraseAt()
{
    OrderIndex index(4);

    OrderLocation location{};

    assert(index.insert(100, location));

    FindResult result = index.find(100);

    assert(result.location != nullptr);
    assert(index.contains(100));

    assert(index.eraseAt(
        result.slotIndex,
        100
    ));

    assert(!index.contains(100));

    std::cout
        << "Basic find/eraseAt: PASS\n";
}


// =====================================================
// ORDERINDEX TEST 2
// ERASEAT WRONG ORDER ID
// =====================================================

void testEraseAtWrongID()
{
    OrderIndex index(4);

    OrderLocation location{};

    assert(index.insert(200, location));

    FindResult result = index.find(200);

    assert(result.location != nullptr);

    /*
        Correct slot, wrong expected order ID.

        eraseAt() must reject this and leave
        order 200 untouched.
    */
    assert(!index.eraseAt(
        result.slotIndex,
        999
    ));

    assert(index.contains(200));

    std::cout
        << "eraseAt ID validation: PASS\n";
}


// =====================================================
// ORDERINDEX TEST 3
// NORMAL ERASE(orderID) STILL WORKS
// =====================================================

void testEraseByID()
{
    OrderIndex index(4);

    OrderLocation location{};

    assert(index.insert(300, location));
    assert(index.contains(300));

    assert(index.erase(300));

    assert(!index.contains(300));

    /*
        Erasing the same ID twice must fail.
    */
    assert(!index.erase(300));

    std::cout
        << "erase(orderID): PASS\n";
}


// =====================================================
// ORDERINDEX TEST 4
// COLLISION CHAIN USING ERASEAT
// =====================================================

void testCollisionEraseAt()
{
    /*
        expectedOrders = 4

        target capacity = 8
        therefore mask = 7
    */
    OrderIndex index(4);

    constexpr std::size_t MASK = 7;
    constexpr std::size_t BUCKET = 3;

    const auto ids =
        findIdsForBucket(
            BUCKET,
            MASK,
            3
        );

    OrderLocation location{};

    assert(index.insert(ids[0], location));
    assert(index.insert(ids[1], location));
    assert(index.insert(ids[2], location));

    assert(index.contains(ids[0]));
    assert(index.contains(ids[1]));
    assert(index.contains(ids[2]));


    /*
        Delete the MIDDLE order using the new
        V3 find() -> eraseAt() path.
    */

    FindResult middle =
        index.find(ids[1]);

    assert(middle.location != nullptr);

    assert(index.eraseAt(
        middle.slotIndex,
        ids[1]
    ));


    assert(index.contains(ids[0]));
    assert(!index.contains(ids[1]));
    assert(index.contains(ids[2]));

    std::cout
        << "Collision eraseAt deletion: PASS\n";
}


// =====================================================
// ORDERINDEX TEST 5
// WRAPAROUND CHAIN USING ERASEAT
// =====================================================

void testWraparoundEraseAt()
{
    /*
        Capacity = 8.

        Find three IDs whose home bucket is 7.

        They should occupy a chain similar to:

            slot 7
            slot 0
            slot 1
    */

    OrderIndex index(4);

    constexpr std::size_t MASK = 7;
    constexpr std::size_t BUCKET = 7;

    const auto ids =
        findIdsForBucket(
            BUCKET,
            MASK,
            3
        );

    OrderLocation location{};

    assert(index.insert(ids[0], location));
    assert(index.insert(ids[1], location));
    assert(index.insert(ids[2], location));


    assert(index.contains(ids[0]));
    assert(index.contains(ids[1]));
    assert(index.contains(ids[2]));


    /*
        Remove the first element in the
        wraparound probe chain.
    */

    FindResult first =
        index.find(ids[0]);

    assert(first.location != nullptr);

    assert(index.eraseAt(
        first.slotIndex,
        ids[0]
    ));


    /*
        The other two must remain reachable.
    */

    assert(!index.contains(ids[0]));
    assert(index.contains(ids[1]));
    assert(index.contains(ids[2]));

    std::cout
        << "Wraparound eraseAt deletion: PASS\n";
}


// =====================================================
// ORDERINDEX TEST 6
// CAPACITY
// =====================================================

void testCapacity()
{
    /*
        expectedOrders = 2
        target = 4
        capacity becomes 4
    */

    OrderIndex index(2);

    OrderLocation location{};

    assert(index.insert(1, location));
    assert(index.insert(2, location));
    assert(index.insert(3, location));
    assert(index.insert(4, location));

    /*
        Table is now physically full.
    */
    assert(!index.insert(5, location));

    assert(index.contains(1));
    assert(index.contains(2));
    assert(index.contains(3));
    assert(index.contains(4));
    assert(!index.contains(5));

    std::cout
        << "Capacity handling: PASS\n";
}


// =====================================================
// MATCHING ENGINE TEST 1
// PRICE-TIME PRIORITY
// =====================================================

void testPriceTimePriority()
{
    MatchingEngine engine(100);

    std::vector<Trade> trades;


    /*
        Resting sells:

        #10: 10 @ 10200
        #11: 15 @ 10200
        #12: 20 @ 10300
    */

    Order sell10 =
        makeLimitOrder(
            10,
            Side::SELL,
            10'200,
            10
        );

    Order sell11 =
        makeLimitOrder(
            11,
            Side::SELL,
            10'200,
            15
        );

    Order sell12 =
        makeLimitOrder(
            12,
            Side::SELL,
            10'300,
            20
        );


    assert(
        engine.processOrder(
            sell10,
            trades
        )
        == ProcessStatus::ACCEPTED
    );

    assert(trades.empty());


    assert(
        engine.processOrder(
            sell11,
            trades
        )
        == ProcessStatus::ACCEPTED
    );

    assert(trades.empty());


    assert(
        engine.processOrder(
            sell12,
            trades
        )
        == ProcessStatus::ACCEPTED
    );

    assert(trades.empty());


    /*
        BUY #20 for 30 @ 10300

        Expected:

        10 from #10 @ 10200
        15 from #11 @ 10200
         5 from #12 @ 10300
    */

    Order buy20 =
        makeLimitOrder(
            20,
            Side::BUY,
            10'300,
            30
        );


    assert(
        engine.processOrder(
            buy20,
            trades
        )
        == ProcessStatus::ACCEPTED
    );

    assert(trades.size() == 3);


    assert(trades[0].buyOrderID == 20);
    assert(trades[0].sellOrderID == 10);
    assert(trades[0].price == 10'200);
    assert(trades[0].quantity == 10);


    assert(trades[1].buyOrderID == 20);
    assert(trades[1].sellOrderID == 11);
    assert(trades[1].price == 10'200);
    assert(trades[1].quantity == 15);


    assert(trades[2].buyOrderID == 20);
    assert(trades[2].sellOrderID == 12);
    assert(trades[2].price == 10'300);
    assert(trades[2].quantity == 5);


    /*
        #12 should still have 15 remaining.

        BUY #21 consumes the remainder.
    */

    Order buy21 =
        makeLimitOrder(
            21,
            Side::BUY,
            10'300,
            15
        );


    assert(
        engine.processOrder(
            buy21,
            trades
        )
        == ProcessStatus::ACCEPTED
    );

    assert(trades.size() == 1);

    assert(trades[0].buyOrderID == 21);
    assert(trades[0].sellOrderID == 12);
    assert(trades[0].price == 10'300);
    assert(trades[0].quantity == 15);


    std::cout
        << "Price-time priority / matching: PASS\n";
}


// =====================================================
// MATCHING ENGINE TEST 2
// PARTIAL FILL + V3 CANCELLATION PATH
// =====================================================

void testPartialFillAndCancellation()
{
    MatchingEngine engine(100);

    std::vector<Trade> trades;


    Order buy100 =
        makeLimitOrder(
            100,
            Side::BUY,
            10'000,
            100
        );


    assert(
        engine.processOrder(
            buy100,
            trades
        )
        == ProcessStatus::ACCEPTED
    );

    assert(trades.empty());


    /*
        Sell 40 into the resting BUY.

        BUY #100 should still have 60 remaining.
    */

    Order sell200 =
        makeLimitOrder(
            200,
            Side::SELL,
            10'000,
            40
        );


    assert(
        engine.processOrder(
            sell200,
            trades
        )
        == ProcessStatus::ACCEPTED
    );

    assert(trades.size() == 1);

    assert(trades[0].buyOrderID == 100);
    assert(trades[0].sellOrderID == 200);
    assert(trades[0].price == 10'000);
    assert(trades[0].quantity == 40);


    /*
        This is especially important for V3.

        cancelOrder() now uses:

            find()
              ->
            slotIndex
              ->
            eraseAt()
    */

    assert(engine.cancelOrder(100));

    /*
        Already cancelled.
    */
    assert(!engine.cancelOrder(100));


    std::cout
        << "Partial fill + V3 cancellation: PASS\n";
}


// =====================================================
// MATCHING ENGINE TEST 3
// VALIDATION
// =====================================================

void testValidation()
{
    MatchingEngine engine(100);

    std::vector<Trade> trades;


    // ---------------------------------------------
    // Zero quantity
    // ---------------------------------------------

    Order zeroQuantity =
        makeLimitOrder(
            300,
            Side::BUY,
            10'000,
            0
        );

    assert(
        engine.processOrder(
            zeroQuantity,
            trades
        )
        == ProcessStatus::REJECTED_ZERO_QUANTITY
    );


    // ---------------------------------------------
    // Quantity above engine limit
    // ---------------------------------------------

    Order tooLarge =
        makeLimitOrder(
            301,
            Side::BUY,
            10'000,
            1'000'001
        );

    assert(
        engine.processOrder(
            tooLarge,
            trades
        )
        == ProcessStatus::REJECTED_QUANTITY_TOO_LARGE
    );


    // ---------------------------------------------
    // Invalid LIMIT price
    // ---------------------------------------------

    Order badPrice =
        makeLimitOrder(
            302,
            Side::BUY,
            0,
            100
        );

    assert(
        engine.processOrder(
            badPrice,
            trades
        )
        == ProcessStatus::REJECTED_INVALID_PRICE
    );


    // ---------------------------------------------
    // Duplicate active ID
    // ---------------------------------------------

    Order valid =
        makeLimitOrder(
            303,
            Side::BUY,
            10'000,
            100
        );


    assert(
        engine.processOrder(
            valid,
            trades
        )
        == ProcessStatus::ACCEPTED
    );


    Order duplicate =
        makeLimitOrder(
            303,
            Side::BUY,
            9'900,
            50
        );


    assert(
        engine.processOrder(
            duplicate,
            trades
        )
        == ProcessStatus::REJECTED_DUPLICATE_ID
    );


    // ---------------------------------------------
    // MARKET price is ignored
    // ---------------------------------------------

    Order market =
        makeMarketOrder(
            304,
            Side::BUY,
            10
        );


    assert(
        engine.processOrder(
            market,
            trades
        )
        == ProcessStatus::ACCEPTED
    );


    std::cout
        << "Validation: PASS\n";
}


// =====================================================
// MATCHING ENGINE TEST 4
// CAPACITY REJECTION + ROLLBACK
// =====================================================

void testCapacityRollback()
{
    /*
        expectedOrders = 1

        OrderIndex target capacity = 2.

        Therefore only two active orders can physically
        occupy the index.
    */

    MatchingEngine engine(1);

    std::vector<Trade> trades;


    Order order1 =
        makeLimitOrder(
            1000,
            Side::BUY,
            9'900,
            10
        );

    Order order2 =
        makeLimitOrder(
            1001,
            Side::BUY,
            9'800,
            10
        );

    Order order3 =
        makeLimitOrder(
            1002,
            Side::BUY,
            9'700,
            10
        );


    assert(
        engine.processOrder(
            order1,
            trades
        )
        == ProcessStatus::ACCEPTED
    );


    assert(
        engine.processOrder(
            order2,
            trades
        )
        == ProcessStatus::ACCEPTED
    );


    /*
        Index is full.

        The third resting order should fail and its
        PriceLevel insertion must be rolled back.
    */

    assert(
        engine.processOrder(
            order3,
            trades
        )
        == ProcessStatus::REJECTED_CAPACITY
    );


    /*
        The failed order must NOT exist in the book.
    */

    assert(!engine.cancelOrder(1002));


    /*
        Existing orders must still be valid.
    */

    assert(engine.cancelOrder(1000));
    assert(engine.cancelOrder(1001));


    std::cout
        << "Capacity rejection + rollback: PASS\n";
}


// =====================================================
// MAIN
// =====================================================

int main()
{
    std::cout << "========================================\n";
    std::cout << "V3 CORRECTNESS TESTS\n";
    std::cout << "========================================\n\n";


    // =================================================
    // ORDERINDEX
    // =================================================

    std::cout << "--- OrderIndex Tests ---\n";

    testBasicEraseAt();
    testEraseAtWrongID();
    testEraseByID();
    testCollisionEraseAt();
    testWraparoundEraseAt();
    testCapacity();

    std::cout
        << "All OrderIndex tests passed.\n\n";


    // =================================================
    // MATCHING ENGINE
    // =================================================

    std::cout
        << "--- Matching Engine Regression Tests ---\n";

    testPriceTimePriority();
    testPartialFillAndCancellation();
    testValidation();
    testCapacityRollback();


    std::cout
        << "\nAll MatchingEngine regression tests passed.\n";


    std::cout << "\n========================================\n";
    std::cout << "ALL V3 CORRECTNESS TESTS PASSED\n";
    std::cout << "========================================\n";


    return 0;
}