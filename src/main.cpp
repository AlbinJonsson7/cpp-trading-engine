#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>

#include "trading/matching_engine.hpp"
#include "trading/order.hpp"
#include "trading/order_index.hpp"
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
// FIND IDS THAT HASH TO SAME BUCKET
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
// V3.5 SENTINEL TEST 1
// ORDERINDEX MUST REJECT ID 0
// =====================================================

void testOrderIndexRejectsZeroID()
{
    OrderIndex index(4);

    OrderLocation location{};

    assert(!index.insert(0, location));

    assert(!index.contains(0));

    FindResult result = index.find(0);

    assert(result.location == nullptr);

    std::cout
        << "V3.5 OrderIndex rejects ID 0: PASS\n";
}


// =====================================================
// V3.5 SENTINEL TEST 2
// DELETED SLOT MUST BECOME REUSABLE
// =====================================================

void testDeletedSlotReusable()
{
    /*
        expectedOrders = 2
        table capacity = 4

        Fill the entire table first.
    */

    OrderIndex index(2);

    OrderLocation location{};

    assert(index.insert(1, location));
    assert(index.insert(2, location));
    assert(index.insert(3, location));
    assert(index.insert(4, location));

    assert(!index.insert(5, location));


    /*
        Remove one entry.

        eraseAt() should eventually restore an empty
        slot by writing orderID = 0.
    */

    assert(index.erase(2));

    assert(!index.contains(2));


    /*
        The table must now accept another order.

        If the final deletion hole was not restored to
        the zero sentinel, this insertion would fail.
    */

    assert(index.insert(5, location));

    assert(index.contains(5));

    std::cout
        << "V3.5 deleted slot sentinel reuse: PASS\n";
}


// =====================================================
// ORDERINDEX TEST 3
// BASIC FIND / ERASEAT
// =====================================================

void testBasicEraseAt()
{
    OrderIndex index(4);

    OrderLocation location{};

    assert(index.insert(100, location));

    FindResult result =
        index.find(100);

    assert(result.location != nullptr);
    assert(index.contains(100));

    assert(
        index.eraseAt(
            result.slotIndex,
            100
        )
    );

    assert(!index.contains(100));

    std::cout
        << "Basic find/eraseAt: PASS\n";
}


// =====================================================
// ORDERINDEX TEST 4
// WRONG EXPECTED ID
// =====================================================

void testEraseAtWrongID()
{
    OrderIndex index(4);

    OrderLocation location{};

    assert(index.insert(200, location));

    FindResult result =
        index.find(200);

    assert(result.location != nullptr);

    assert(
        !index.eraseAt(
            result.slotIndex,
            999
        )
    );

    assert(index.contains(200));

    std::cout
        << "eraseAt ID validation: PASS\n";
}


// =====================================================
// ORDERINDEX TEST 5
// NORMAL ERASE
// =====================================================

void testEraseByID()
{
    OrderIndex index(4);

    OrderLocation location{};

    assert(index.insert(300, location));

    assert(index.contains(300));

    assert(index.erase(300));

    assert(!index.contains(300));

    assert(!index.erase(300));

    std::cout
        << "erase(orderID): PASS\n";
}


// =====================================================
// ORDERINDEX TEST 6
// COLLISION DELETION
// =====================================================

void testCollisionEraseAt()
{
    /*
        expectedOrders = 4
        capacity = 8
        mask = 7
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


    FindResult middle =
        index.find(ids[1]);

    assert(middle.location != nullptr);


    assert(
        index.eraseAt(
            middle.slotIndex,
            ids[1]
        )
    );


    assert(index.contains(ids[0]));
    assert(!index.contains(ids[1]));
    assert(index.contains(ids[2]));

    std::cout
        << "Collision eraseAt deletion: PASS\n";
}


// =====================================================
// ORDERINDEX TEST 7
// WRAPAROUND DELETION
// =====================================================

void testWraparoundEraseAt()
{
    /*
        IDs all hash to bucket 7.

        Probe chain:

        7 -> 0 -> 1
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


    FindResult first =
        index.find(ids[0]);

    assert(first.location != nullptr);


    assert(
        index.eraseAt(
            first.slotIndex,
            ids[0]
        )
    );


    assert(!index.contains(ids[0]));

    assert(index.contains(ids[1]));

    assert(index.contains(ids[2]));

    std::cout
        << "Wraparound eraseAt deletion: PASS\n";
}


// =====================================================
// ORDERINDEX TEST 8
// CAPACITY
// =====================================================

void testCapacity()
{
    OrderIndex index(2);

    OrderLocation location{};

    assert(index.insert(1, location));
    assert(index.insert(2, location));
    assert(index.insert(3, location));
    assert(index.insert(4, location));

    assert(!index.insert(5, location));

    assert(index.contains(1));
    assert(index.contains(2));
    assert(index.contains(3));
    assert(index.contains(4));

    std::cout
        << "Capacity handling: PASS\n";
}


// =====================================================
// MATCHING TEST 1
// V3.5 INVALID ORDER ID
// =====================================================

void testMatchingEngineRejectsZeroID()
{
    MatchingEngine engine(100);

    std::vector<Trade> trades;


    /*
        Give it otherwise valid order data.

        ID 0 alone should cause rejection.
    */

    Order zeroID =
        makeLimitOrder(
            0,
            Side::BUY,
            10'000,
            100
        );


    assert(
        engine.processOrder(zeroID, trades)
        == ProcessStatus::REJECTED_INVALID_ORDER_ID
    );

    assert(trades.empty());


    /*
        Because ID validation is first, even an order
        with ID 0 AND another invalid field should still
        return INVALID_ORDER_ID.
    */

    Order zeroIDZeroQuantity =
        makeLimitOrder(
            0,
            Side::BUY,
            10'000,
            0
        );


    assert(
        engine.processOrder(
            zeroIDZeroQuantity,
            trades
        )
        == ProcessStatus::REJECTED_INVALID_ORDER_ID
    );


    std::cout
        << "V3.5 MatchingEngine rejects ID 0: PASS\n";
}


// =====================================================
// MATCHING TEST 2
// PRICE-TIME PRIORITY
// =====================================================

void testPriceTimePriority()
{
    MatchingEngine engine(100);

    std::vector<Trade> trades;


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
        engine.processOrder(sell10, trades)
        == ProcessStatus::ACCEPTED
    );

    assert(trades.empty());


    assert(
        engine.processOrder(sell11, trades)
        == ProcessStatus::ACCEPTED
    );

    assert(trades.empty());


    assert(
        engine.processOrder(sell12, trades)
        == ProcessStatus::ACCEPTED
    );

    assert(trades.empty());


    Order buy20 =
        makeLimitOrder(
            20,
            Side::BUY,
            10'300,
            30
        );


    assert(
        engine.processOrder(buy20, trades)
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


    Order buy21 =
        makeLimitOrder(
            21,
            Side::BUY,
            10'300,
            15
        );


    assert(
        engine.processOrder(buy21, trades)
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
// MATCHING TEST 3
// BUY CANCELLATION / PRICE LEVEL
// =====================================================

void testBuyCancellationPriceLevel()
{
    MatchingEngine engine(100);

    std::vector<Trade> trades;


    Order buy400 =
        makeLimitOrder(
            400,
            Side::BUY,
            10'100,
            10
        );

    Order buy401 =
        makeLimitOrder(
            401,
            Side::BUY,
            10'000,
            10
        );


    assert(
        engine.processOrder(buy400, trades)
        == ProcessStatus::ACCEPTED
    );

    assert(
        engine.processOrder(buy401, trades)
        == ProcessStatus::ACCEPTED
    );


    assert(engine.cancelOrder(400));

    assert(!engine.cancelOrder(400));


    Order sell402 =
        makeMarketOrder(
            402,
            Side::SELL,
            10
        );


    assert(
        engine.processOrder(sell402, trades)
        == ProcessStatus::ACCEPTED
    );


    assert(trades.size() == 1);

    assert(trades[0].buyOrderID == 401);
    assert(trades[0].sellOrderID == 402);
    assert(trades[0].price == 10'000);
    assert(trades[0].quantity == 10);


    std::cout
        << "BUY cancellation / price-level removal: PASS\n";
}


// =====================================================
// MATCHING TEST 4
// SELL CANCELLATION / PRICE LEVEL
// =====================================================

void testSellCancellationPriceLevel()
{
    MatchingEngine engine(100);

    std::vector<Trade> trades;


    Order sell500 =
        makeLimitOrder(
            500,
            Side::SELL,
            10'200,
            10
        );

    Order sell501 =
        makeLimitOrder(
            501,
            Side::SELL,
            10'300,
            10
        );


    assert(
        engine.processOrder(sell500, trades)
        == ProcessStatus::ACCEPTED
    );

    assert(
        engine.processOrder(sell501, trades)
        == ProcessStatus::ACCEPTED
    );


    assert(engine.cancelOrder(500));

    assert(!engine.cancelOrder(500));


    Order buy502 =
        makeMarketOrder(
            502,
            Side::BUY,
            10
        );


    assert(
        engine.processOrder(buy502, trades)
        == ProcessStatus::ACCEPTED
    );


    assert(trades.size() == 1);

    assert(trades[0].buyOrderID == 502);
    assert(trades[0].sellOrderID == 501);
    assert(trades[0].price == 10'300);
    assert(trades[0].quantity == 10);


    std::cout
        << "SELL cancellation / price-level removal: PASS\n";
}


// =====================================================
// MATCHING TEST 5
// PARTIAL FILL THEN CANCELLATION
// =====================================================

void testPartialFillAndCancellation()
{
    MatchingEngine engine(100);

    std::vector<Trade> trades;


    Order buy600 =
        makeLimitOrder(
            600,
            Side::BUY,
            10'000,
            100
        );


    assert(
        engine.processOrder(buy600, trades)
        == ProcessStatus::ACCEPTED
    );


    Order sell601 =
        makeLimitOrder(
            601,
            Side::SELL,
            10'000,
            40
        );


    assert(
        engine.processOrder(sell601, trades)
        == ProcessStatus::ACCEPTED
    );


    assert(trades.size() == 1);

    assert(trades[0].buyOrderID == 600);
    assert(trades[0].sellOrderID == 601);
    assert(trades[0].quantity == 40);


    assert(engine.cancelOrder(600));

    assert(!engine.cancelOrder(600));


    std::cout
        << "Partial fill + cancellation: PASS\n";
}


// =====================================================
// MATCHING TEST 6
// NORMAL VALIDATION
// =====================================================

void testValidation()
{
    MatchingEngine engine(100);

    std::vector<Trade> trades;


    Order zeroQuantity =
        makeLimitOrder(
            700,
            Side::BUY,
            10'000,
            0
        );


    assert(
        engine.processOrder(zeroQuantity, trades)
        == ProcessStatus::REJECTED_ZERO_QUANTITY
    );


    Order tooLarge =
        makeLimitOrder(
            701,
            Side::BUY,
            10'000,
            1'000'001
        );


    assert(
        engine.processOrder(tooLarge, trades)
        == ProcessStatus::REJECTED_QUANTITY_TOO_LARGE
    );


    Order badPrice =
        makeLimitOrder(
            702,
            Side::BUY,
            0,
            100
        );


    assert(
        engine.processOrder(badPrice, trades)
        == ProcessStatus::REJECTED_INVALID_PRICE
    );


    Order valid =
        makeLimitOrder(
            703,
            Side::BUY,
            10'000,
            100
        );


    assert(
        engine.processOrder(valid, trades)
        == ProcessStatus::ACCEPTED
    );


    Order duplicate =
        makeLimitOrder(
            703,
            Side::BUY,
            9'900,
            50
        );


    assert(
        engine.processOrder(duplicate, trades)
        == ProcessStatus::REJECTED_DUPLICATE_ID
    );


    Order market =
        makeMarketOrder(
            704,
            Side::BUY,
            10
        );


    assert(
        engine.processOrder(market, trades)
        == ProcessStatus::ACCEPTED
    );


    std::cout
        << "Validation: PASS\n";
}


// =====================================================
// MATCHING TEST 7
// CAPACITY + ROLLBACK
// =====================================================

void testCapacityRollback()
{
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
        engine.processOrder(order1, trades)
        == ProcessStatus::ACCEPTED
    );

    assert(
        engine.processOrder(order2, trades)
        == ProcessStatus::ACCEPTED
    );

    assert(
        engine.processOrder(order3, trades)
        == ProcessStatus::REJECTED_CAPACITY
    );


    assert(!engine.cancelOrder(1002));

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
    std::cout << "V3.5 CORRECTNESS TESTS\n";
    std::cout << "========================================\n\n";


    // =================================================
    // ORDERINDEX
    // =================================================

    std::cout
        << "--- OrderIndex Tests ---\n";


    testOrderIndexRejectsZeroID();

    testDeletedSlotReusable();

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


    testMatchingEngineRejectsZeroID();

    testPriceTimePriority();

    testBuyCancellationPriceLevel();

    testSellCancellationPriceLevel();

    testPartialFillAndCancellation();

    testValidation();

    testCapacityRollback();


    std::cout
        << "\nAll MatchingEngine regression tests passed.\n";


    std::cout << "\n========================================\n";
    std::cout << "ALL V3.5 CORRECTNESS TESTS PASSED\n";
    std::cout << "========================================\n";


    return 0;
}