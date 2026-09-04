#include <cstdint>
#include <iostream>
#include <vector>

#include "trading/order.hpp"
#include "trading/trade.hpp"
#include "trading/order_node_pool.hpp"
#include "trading/price_level.hpp"
#include "trading/order_book.hpp"
#include "trading/matching_engine.hpp"


Order makeLimitOrder(
    uint64_t orderID,
    Side side,
    int64_t price,
    uint32_t quantity
){
    Order order{};

    order.orderID = orderID;
    order.side = side;
    order.price = price;
    order.originalQuantity = quantity;
    order.remainingQuantity = quantity;
    order.orderType = OrderType::LIMIT;

    return order;
}


bool check(bool condition, const char* name)
{
    if(condition){
        std::cout << name << ": PASS\n";
        return true;
    }

    std::cout << name << ": FAIL\n";
    return false;
}


int main()
{
    bool allPassed = true;

    std::cout << "========================================\n";
    std::cout << "HOT/COLD ORDER BOOK CORRECTNESS TESTS\n";
    std::cout << "========================================\n\n";


    // =================================================
    // TEST GROUP 1
    // OrderNodePool
    // =================================================

    std::cout << "--- OrderNodePool Tests ---\n";

    {
        OrderNodePool pool(10);

        Order order1 =
            makeLimitOrder(
                1,
                Side::BUY,
                10000,
                100
            );

        Order order2 =
            makeLimitOrder(
                2,
                Side::BUY,
                10000,
                100
            );


        uint32_t index1 =
            pool.acquire(order1);

        uint32_t index2 =
            pool.acquire(order2);


        allPassed &=
            check(
                index1 == 0 &&
                index2 == 1,
                "Sequential node acquisition"
            );


        allPassed &=
            check(
                pool.getOrder(index1).orderID == 1 &&
                pool.getOrder(index2).orderID == 2,
                "Order payload storage"
            );


        allPassed &=
            check(
                pool.getLinks(index1).previous ==
                    INVALID_NODE_INDEX &&
                pool.getLinks(index1).next ==
                    INVALID_NODE_INDEX &&
                pool.getLinks(index1).active,
                "Fresh link state"
            );


        // ---------------------------------------------
        // Give node 1 fake links so release/reuse can
        // verify that linkage data is reset.
        // ---------------------------------------------

        pool.getLinks(index1).previous = 55;
        pool.getLinks(index1).next = 77;


        allPassed &=
            check(
                pool.release(index1),
                "Node release"
            );


        allPassed &=
            check(
                !pool.release(index1),
                "Double release rejection"
            );


        allPassed &=
            check(
                !pool.release(999),
                "Invalid index rejection"
            );


        Order order3 =
            makeLimitOrder(
                3,
                Side::SELL,
                10100,
                200
            );


        uint32_t reusedIndex =
            pool.acquire(order3);


        allPassed &=
            check(
                reusedIndex == index1,
                "Released node reuse"
            );


        allPassed &=
            check(
                pool.getOrder(reusedIndex).orderID == 3 &&
                pool.getOrder(reusedIndex).price == 10100,
                "Reused Order payload replacement"
            );


        const OrderLinks& reusedLinks =
            pool.getLinks(reusedIndex);


        allPassed &=
            check(
                reusedLinks.previous ==
                    INVALID_NODE_INDEX &&
                reusedLinks.next ==
                    INVALID_NODE_INDEX &&
                reusedLinks.active,
                "Reused link state reset"
            );


        allPassed &=
            check(
                pool.getOrder(index2).orderID == 2 &&
                pool.getLinks(index2).active,
                "Other node isolation"
            );
    }


    std::cout << "\n";


    // =================================================
    // TEST GROUP 2
    // PriceLevel FIFO + link manipulation
    // =================================================

    std::cout << "--- PriceLevel Tests ---\n";

    {
        OrderNodePool pool(20);

        PriceLevel level(
            10000,
            &pool
        );


        allPassed &=
            check(
                level.isEmpty(),
                "New PriceLevel empty"
            );


        uint32_t index1 =
            level.addOrder(
                makeLimitOrder(
                    1,
                    Side::BUY,
                    10000,
                    100
                )
            );


        uint32_t index2 =
            level.addOrder(
                makeLimitOrder(
                    2,
                    Side::BUY,
                    10000,
                    100
                )
            );


        uint32_t index3 =
            level.addOrder(
                makeLimitOrder(
                    3,
                    Side::BUY,
                    10000,
                    100
                )
            );


        allPassed &=
            check(
                level.getFrontOrder().orderID == 1,
                "FIFO front order"
            );


        allPassed &=
            check(
                pool.getLinks(index1).next == index2 &&
                pool.getLinks(index2).previous == index1 &&
                pool.getLinks(index2).next == index3 &&
                pool.getLinks(index3).previous == index2,
                "Intrusive link chain"
            );


        // ---------------------------------------------
        // Remove middle:
        //
        // 1 <-> 2 <-> 3
        //
        // becomes:
        //
        // 1 <-> 3
        // ---------------------------------------------

        allPassed &=
            check(
                level.removeOrder(index2),
                "Remove middle node"
            );


        allPassed &=
            check(
                pool.getLinks(index1).next == index3 &&
                pool.getLinks(index3).previous == index1,
                "Middle unlink reconnects neighbors"
            );


        allPassed &=
            check(
                level.getFrontOrder().orderID == 1,
                "Middle removal preserves head"
            );


        // ---------------------------------------------
        // Remove current head.
        // ---------------------------------------------

        allPassed &=
            check(
                level.removeOrder(index1),
                "Remove head node"
            );


        allPassed &=
            check(
                level.getFrontOrder().orderID == 3 &&
                pool.getLinks(index3).previous ==
                    INVALID_NODE_INDEX,
                "Head removal updates FIFO"
            );


        // ---------------------------------------------
        // Add another Order.
        //
        // A recycled pool index should not break FIFO.
        // ---------------------------------------------

        uint32_t index4 =
            level.addOrder(
                makeLimitOrder(
                    4,
                    Side::BUY,
                    10000,
                    100
                )
            );


        allPassed &=
            check(
                level.getFrontOrder().orderID == 3,
                "Recycled index preserves FIFO"
            );


        allPassed &=
            check(
                pool.getLinks(index3).next == index4 &&
                pool.getLinks(index4).previous == index3,
                "Recycled node linked correctly"
            );


        // ---------------------------------------------
        // Remove front from multi-node level.
        // ---------------------------------------------

        level.removeFrontOrder();


        allPassed &=
            check(
                !level.isEmpty() &&
                level.getFrontOrder().orderID == 4,
                "removeFrontOrder multi-node"
            );


        allPassed &=
            check(
                pool.getLinks(index4).previous ==
                    INVALID_NODE_INDEX,
                "New head previous cleared"
            );


        // ---------------------------------------------
        // Remove final node.
        // ---------------------------------------------

        level.removeFrontOrder();


        allPassed &=
            check(
                level.isEmpty(),
                "removeFrontOrder single-node"
            );
    }


    std::cout << "\n";


    // =================================================
    // TEST GROUP 3
    // OrderBook integration
    // =================================================

    std::cout << "--- OrderBook Tests ---\n";

    {
        OrderBook book(100);


        allPassed &=
            check(
                book.addOrder(
                    makeLimitOrder(
                        1,
                        Side::BUY,
                        10000,
                        100
                    )
                ),
                "OrderBook add BUY 1"
            );


        allPassed &=
            check(
                book.addOrder(
                    makeLimitOrder(
                        2,
                        Side::BUY,
                        10000,
                        100
                    )
                ),
                "OrderBook add BUY 2"
            );


        allPassed &=
            check(
                book.addOrder(
                    makeLimitOrder(
                        3,
                        Side::BUY,
                        10000,
                        100
                    )
                ),
                "OrderBook add BUY 3"
            );


        allPassed &=
            check(
                book.getBestBidOrder() != nullptr &&
                book.getBestBidOrder()->orderID == 1,
                "OrderBook FIFO head"
            );


        // ---------------------------------------------
        // Cancel middle order.
        // ---------------------------------------------

        allPassed &=
            check(
                book.cancelOrder(2),
                "Cancel middle order"
            );


        allPassed &=
            check(
                !book.containsOrder(2) &&
                book.containsOrder(1) &&
                book.containsOrder(3),
                "Middle cancellation isolation"
            );


        allPassed &=
            check(
                book.getBestBidOrder() != nullptr &&
                book.getBestBidOrder()->orderID == 1,
                "Middle cancellation preserves head"
            );


        // ---------------------------------------------
        // Cancel head.
        // ---------------------------------------------

        allPassed &=
            check(
                book.cancelOrder(1),
                "Cancel head order"
            );


        allPassed &=
            check(
                book.getBestBidOrder() != nullptr &&
                book.getBestBidOrder()->orderID == 3,
                "Head cancellation advances FIFO"
            );


        // ---------------------------------------------
        // Add after released indices exist.
        // ---------------------------------------------

        allPassed &=
            check(
                book.addOrder(
                    makeLimitOrder(
                        4,
                        Side::BUY,
                        10000,
                        100
                    )
                ),
                "Add after node recycle"
            );


        allPassed &=
            check(
                book.getBestBidOrder() != nullptr &&
                book.getBestBidOrder()->orderID == 3,
                "Node recycle preserves price-time priority"
            );


        allPassed &=
            check(
                book.cancelOrder(3),
                "Cancel recycled-chain head"
            );


        allPassed &=
            check(
                book.getBestBidOrder() != nullptr &&
                book.getBestBidOrder()->orderID == 4,
                "FIFO after recycled-chain cancel"
            );


        allPassed &=
            check(
                book.cancelOrder(4),
                "Cancel final bid"
            );


        allPassed &=
            check(
                !book.hasBids() &&
                !book.getBestBid().has_value(),
                "Empty price-level removal"
            );
    }


    std::cout << "\n";


    // =================================================
    // TEST GROUP 4
    // Best-price ordering
    // =================================================

    std::cout << "--- Best Price Tests ---\n";

    {
        OrderBook book(100);


        book.addOrder(
            makeLimitOrder(
                1,
                Side::BUY,
                10000,
                100
            )
        );


        book.addOrder(
            makeLimitOrder(
                2,
                Side::BUY,
                10100,
                100
            )
        );


        book.addOrder(
            makeLimitOrder(
                3,
                Side::SELL,
                10300,
                100
            )
        );


        book.addOrder(
            makeLimitOrder(
                4,
                Side::SELL,
                10200,
                100
            )
        );


        allPassed &=
            check(
                book.getBestBid().has_value() &&
                book.getBestBid().value() == 10100,
                "Best bid ordering"
            );


        allPassed &=
            check(
                book.getBestAsk().has_value() &&
                book.getBestAsk().value() == 10200,
                "Best ask ordering"
            );


        book.cancelOrder(2);


        allPassed &=
            check(
                book.getBestBid().has_value() &&
                book.getBestBid().value() == 10000,
                "Best bid fallback after cancel"
            );


        book.cancelOrder(4);


        allPassed &=
            check(
                book.getBestAsk().has_value() &&
                book.getBestAsk().value() == 10300,
                "Best ask fallback after cancel"
            );
    }


    std::cout << "\n";


    // =================================================
    // TEST GROUP 5
    // MatchingEngine price-time regression
    // =================================================

    std::cout << "--- Matching Engine Tests ---\n";

    {
        MatchingEngine engine(100);

        std::vector<Trade> trades;


        engine.processOrder(
            makeLimitOrder(
                1,
                Side::SELL,
                10000,
                100
            ),
            trades
        );


        engine.processOrder(
            makeLimitOrder(
                2,
                Side::SELL,
                10000,
                100
            ),
            trades
        );


        trades.clear();


        engine.processOrder(
            makeLimitOrder(
                3,
                Side::BUY,
                10000,
                150
            ),
            trades
        );


        allPassed &=
            check(
                trades.size() == 2,
                "Two expected trades generated"
            );


        if(trades.size() == 2){

            allPassed &=
                check(
                    trades[0].buyOrderID == 3 &&
                    trades[0].sellOrderID == 1 &&
                    trades[0].price == 10000 &&
                    trades[0].quantity == 100,
                    "First trade respects FIFO"
                );


            allPassed &=
                check(
                    trades[1].buyOrderID == 3 &&
                    trades[1].sellOrderID == 2 &&
                    trades[1].price == 10000 &&
                    trades[1].quantity == 50,
                    "Second trade partial fill"
                );
        }


        allPassed &=
            check(
                engine.cancelOrder(2),
                "Cancel partially-filled resting order"
            );
    }


    std::cout << "\n";


    // =================================================
    // TEST GROUP 6
    // Incoming partial-fill remainder rests
    // =================================================

    std::cout << "--- Partial Fill / Resting Remainder Test ---\n";

    {
        MatchingEngine engine(100);

        std::vector<Trade> trades;


        engine.processOrder(
            makeLimitOrder(
                1,
                Side::SELL,
                10000,
                40
            ),
            trades
        );


        trades.clear();


        engine.processOrder(
            makeLimitOrder(
                2,
                Side::BUY,
                10000,
                100
            ),
            trades
        );


        allPassed &=
            check(
                trades.size() == 1 &&
                trades[0].quantity == 40,
                "Incoming partial fill"
            );


        allPassed &=
            check(
                engine.cancelOrder(2),
                "Incoming remainder rests in book"
            );
    }


    std::cout << "\n";


    // =================================================
    // TEST GROUP 7
    // Repeated recycling through MatchingEngine
    // =================================================

    std::cout << "--- Node Recycling Regression Test ---\n";

    {
        MatchingEngine engine(100);

        std::vector<Trade> trades;

        bool recyclingPassed = true;


        for(uint64_t id = 1; id <= 100; id += 2){

            trades.clear();


            engine.processOrder(
                makeLimitOrder(
                    id,
                    Side::SELL,
                    10000,
                    100
                ),
                trades
            );


            trades.clear();


            engine.processOrder(
                makeLimitOrder(
                    id + 1,
                    Side::BUY,
                    10000,
                    100
                ),
                trades
            );


            if(
                trades.size() != 1 ||
                trades[0].sellOrderID != id ||
                trades[0].buyOrderID != id + 1 ||
                trades[0].quantity != 100
            ){
                recyclingPassed = false;
                break;
            }
        }


        allPassed &=
            check(
                recyclingPassed,
                "Repeated acquire/release through matching"
            );
    }


    // =================================================
    // FINAL RESULT
    // =================================================

    std::cout << "\n========================================\n";

    if(allPassed){

        std::cout
            << "ALL HOT/COLD ORDER BOOK TESTS PASSED\n";

    }else{

        std::cout
            << "ONE OR MORE TESTS FAILED\n";
    }

    std::cout << "========================================\n";


    return allPassed ? 0 : 1;
}