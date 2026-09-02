#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory_resource>
#include <vector>

#include "trading/matching_engine.hpp"
#include "trading/order.hpp"
#include "trading/order_index.hpp"
#include "trading/price_level.hpp"
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


OrderLocation makeIndexedLocation(
    PriceLevel& priceLevel,
    uint64_t orderID)
{
    Order order =
        makeLimitOrder(
            orderID,
            Side::BUY,
            priceLevel.getPrice(),
            10
        );

    OrderLocation location{};
    location.orderIterator = priceLevel.addOrder(order);
    location.priceLevel = &priceLevel;

    return location;
}


// =====================================================
// PAGED ORDERINDEX TEST 1
// ID 0 MUST BE REJECTED
// =====================================================

void testOrderIndexRejectsZeroID()
{
    OrderIndex index(4);
    OrderLocation location{};

    assert(!index.insert(0, location));
    assert(!index.contains(0));
    assert(index.find(0) == nullptr);
    assert(!index.erase(0));

    std::cout
        << "Paged OrderIndex rejects ID 0: PASS\n";
}


// =====================================================
// PAGED ORDERINDEX TEST 2
// BASIC INSERT / FIND / ERASE
// =====================================================

void testBasicInsertFindErase()
{
    OrderIndex index(4);
    PriceLevel priceLevel(
        10'000,
        std::pmr::get_default_resource()
    );

    OrderLocation location =
        makeIndexedLocation(priceLevel, 100);

    assert(index.insert(100, location));
    assert(index.contains(100));

    OrderLocation* result = index.find(100);

    assert(result != nullptr);
    assert(result->priceLevel == &priceLevel);
    assert(result->orderIterator->orderID == 100);

    assert(index.erase(100));
    assert(!index.contains(100));
    assert(index.find(100) == nullptr);
    assert(!index.erase(100));

    std::cout
        << "Basic paged insert/find/erase: PASS\n";
}


// =====================================================
// PAGED ORDERINDEX TEST 3
// ACTIVE DUPLICATE MUST BE REJECTED
// =====================================================

void testActiveDuplicateRejected()
{
    OrderIndex index(4);

    PriceLevel firstLevel(
        10'000,
        std::pmr::get_default_resource()
    );

    PriceLevel secondLevel(
        9'900,
        std::pmr::get_default_resource()
    );

    OrderLocation firstLocation =
        makeIndexedLocation(firstLevel, 200);

    OrderLocation duplicateLocation =
        makeIndexedLocation(secondLevel, 200);

    assert(index.insert(200, firstLocation));
    assert(!index.insert(200, duplicateLocation));

    OrderLocation* result = index.find(200);

    assert(result != nullptr);
    assert(result->priceLevel == &firstLevel);

    std::cout
        << "Active duplicate rejection: PASS\n";
}


// =====================================================
// PAGED ORDERINDEX TEST 4
// PAGE BOUNDARIES
// =====================================================

void testPageBoundaries()
{
    OrderIndex index(4);
    PriceLevel priceLevel(
        10'000,
        std::pmr::get_default_resource()
    );

    const uint64_t ids[] = {
        1,
        4096,
        4097,
        8192,
        8193
    };

    for(uint64_t id : ids){
        OrderLocation location =
            makeIndexedLocation(priceLevel, id);

        assert(index.insert(id, location));
    }

    for(uint64_t id : ids){
        OrderLocation* result = index.find(id);

        assert(result != nullptr);
        assert(result->orderIterator->orderID == id);
        assert(index.contains(id));
    }

    assert(!index.contains(4095));
    assert(!index.contains(4098));

    std::cout
        << "Page boundary mapping: PASS\n";
}


// =====================================================
// PAGED ORDERINDEX TEST 5
// SPARSE DIRECTORY GROWTH
// =====================================================

void testSparsePageGrowth()
{
    OrderIndex index(1);
    PriceLevel priceLevel(
        10'000,
        std::pmr::get_default_resource()
    );

    OrderLocation first =
        makeIndexedLocation(priceLevel, 1);

    OrderLocation farAway =
        makeIndexedLocation(priceLevel, 20'000);

    assert(index.insert(1, first));
    assert(index.insert(20'000, farAway));

    assert(index.contains(1));
    assert(index.contains(20'000));

    assert(!index.contains(4097));
    assert(!index.contains(12'000));

    std::cout
        << "Sparse page-directory growth: PASS\n";
}


// =====================================================
// PAGED ORDERINDEX TEST 6
// PAGE RECLAMATION + REALLOCATION
// =====================================================

void testPageReclamationAndReallocation()
{
    OrderIndex index(4);
    PriceLevel priceLevel(
        10'000,
        std::pmr::get_default_resource()
    );

    OrderLocation first =
        makeIndexedLocation(priceLevel, 1);

    OrderLocation second =
        makeIndexedLocation(priceLevel, 2);

    assert(index.insert(1, first));
    assert(index.insert(2, second));

    assert(index.erase(1));
    assert(!index.contains(1));
    assert(index.contains(2));

    assert(index.erase(2));
    assert(!index.contains(2));

    OrderLocation third =
        makeIndexedLocation(priceLevel, 3);

    assert(index.insert(3, third));
    assert(index.contains(3));

    std::cout
        << "Page reclamation / reallocation: PASS\n";
}


// =====================================================
// PAGED ORDERINDEX TEST 7
// RECLAIMING ONE PAGE MUST NOT AFFECT ANOTHER
// =====================================================

void testCrossPageEraseIsolation()
{
    OrderIndex index(4);
    PriceLevel priceLevel(
        10'000,
        std::pmr::get_default_resource()
    );

    OrderLocation pageZero =
        makeIndexedLocation(priceLevel, 4096);

    OrderLocation pageOne =
        makeIndexedLocation(priceLevel, 4097);

    assert(index.insert(4096, pageZero));
    assert(index.insert(4097, pageOne));

    assert(index.erase(4096));

    assert(!index.contains(4096));
    assert(index.contains(4097));

    std::cout
        << "Cross-page erase isolation: PASS\n";
}


// =====================================================
// PAGED ORDERINDEX TEST 8
// expectedOrders IS A RESERVE HINT, NOT A HARD CAPACITY
// =====================================================

void testExpectedOrdersIsReserveHint()
{
    OrderIndex index(1);
    PriceLevel priceLevel(
        10'000,
        std::pmr::get_default_resource()
    );

    const uint64_t ids[] = {
        1,
        4097,
        8193,
        12'289
    };

    for(uint64_t id : ids){
        OrderLocation location =
            makeIndexedLocation(priceLevel, id);

        assert(index.insert(id, location));
        assert(index.contains(id));
    }

    std::cout
        << "expectedOrders reserve-hint behavior: PASS\n";
}


// =====================================================
// MATCHING TEST 1
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
// MATCHING TEST 2
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
// MATCHING TEST 3
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
// MATCHING TEST 4
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
// MATCHING TEST 5
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
// MATCHING TEST 6
// PAGED ORDER-ID BOUNDARIES THROUGH THE FULL ENGINE
// =====================================================

void testPagedOrderIDsThroughMatchingEngine()
{
    MatchingEngine engine(4);
    std::vector<Trade> trades;

    Order order4096 =
        makeLimitOrder(
            4096,
            Side::BUY,
            9'900,
            10
        );

    Order order4097 =
        makeLimitOrder(
            4097,
            Side::BUY,
            9'800,
            10
        );

    Order order8193 =
        makeLimitOrder(
            8193,
            Side::BUY,
            9'700,
            10
        );

    assert(
        engine.processOrder(order4096, trades)
        == ProcessStatus::ACCEPTED
    );

    assert(
        engine.processOrder(order4097, trades)
        == ProcessStatus::ACCEPTED
    );

    assert(
        engine.processOrder(order8193, trades)
        == ProcessStatus::ACCEPTED
    );

    assert(engine.cancelOrder(4096));
    assert(engine.cancelOrder(4097));
    assert(engine.cancelOrder(8193));

    assert(!engine.cancelOrder(4096));
    assert(!engine.cancelOrder(4097));
    assert(!engine.cancelOrder(8193));

    std::cout
        << "Paged IDs through MatchingEngine: PASS\n";
}



void testMonotonicOrderIDs()
{
    MatchingEngine engine(100);
    std::vector<Trade> trades;

    Order order100 = makeLimitOrder(100, Side::BUY, 10'000, 10);
    Order order105 = makeLimitOrder(105, Side::BUY, 9'900, 10);
    Order order104 = makeLimitOrder(104, Side::BUY, 9'800, 10);
    Order duplicate105 = makeLimitOrder(105, Side::BUY, 9'700, 10);
    Order order106 = makeLimitOrder(106, Side::BUY, 9'600, 10);

    assert(engine.processOrder(order100, trades) == ProcessStatus::ACCEPTED);
    assert(engine.processOrder(order105, trades) == ProcessStatus::ACCEPTED);

    // Lower than highest submitted ID.
    assert(engine.processOrder(order104, trades)
           == ProcessStatus::REJECTED_DUPLICATE_ID);

    // Same as highest submitted ID.
    assert(engine.processOrder(duplicate105, trades)
           == ProcessStatus::REJECTED_DUPLICATE_ID);

    // Increasing again.
    assert(engine.processOrder(order106, trades)
           == ProcessStatus::ACCEPTED);

    std::cout << "Monotonic order IDs: PASS\n";
}



void testRejectedOrderConsumesID()
{
    MatchingEngine engine(100);
    std::vector<Trade> trades;

    Order invalid200 =
        makeLimitOrder(200, Side::BUY, 10'000, 0);

    Order reused200 =
        makeLimitOrder(200, Side::BUY, 10'000, 10);

    Order valid201 =
        makeLimitOrder(201, Side::BUY, 10'000, 10);

    assert(engine.processOrder(invalid200, trades)
           == ProcessStatus::REJECTED_ZERO_QUANTITY);

    // ID 200 was already submitted, even though the order was rejected.
    assert(engine.processOrder(reused200, trades)
           == ProcessStatus::REJECTED_DUPLICATE_ID);

    assert(engine.processOrder(valid201, trades)
           == ProcessStatus::ACCEPTED);

    std::cout << "Rejected order consumes ID: PASS\n";
}



// =====================================================
// MAIN
// =====================================================

int main()
{
    std::cout << "========================================\n";
    std::cout << "PAGED ORDER INDEX CORRECTNESS TESTS\n";
    std::cout << "========================================\n\n";


    // =================================================
    // ORDERINDEX
    // =================================================

    std::cout
        << "--- OrderIndex Tests ---\n";


    testOrderIndexRejectsZeroID();

    testBasicInsertFindErase();

    testActiveDuplicateRejected();

    testPageBoundaries();

    testSparsePageGrowth();

    testPageReclamationAndReallocation();

    testCrossPageEraseIsolation();

    testExpectedOrdersIsReserveHint();

    testMonotonicOrderIDs();

    testRejectedOrderConsumesID();


    std::cout
        << "All paged OrderIndex tests passed.\n\n";


    // =================================================
    // MATCHING ENGINE
    // =================================================

    std::cout
        << "--- Matching Engine Regression Tests ---\n";


    testPriceTimePriority();

    testBuyCancellationPriceLevel();

    testSellCancellationPriceLevel();

    testPartialFillAndCancellation();

    testValidation();

    testPagedOrderIDsThroughMatchingEngine();


    std::cout
        << "\nAll MatchingEngine regression tests passed.\n";


    std::cout << "\n========================================\n";
    std::cout << "ALL PAGED-INDEX CORRECTNESS TESTS PASSED\n";
    std::cout << "========================================\n";


    return 0;
}