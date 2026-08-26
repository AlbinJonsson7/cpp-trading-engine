#include <iostream>
#include "trading/trading_server.hpp"
#include "trading/client_requests.hpp"
#include "trading/server_response.hpp"
#include "trading/order.hpp"

using namespace std;


int main() {

    std::cout << "--- Test 1: NEW_ORDER Request ---\n";

    TradingServer server;

    ClientRequest newOrderRequest = {
        RequestType::NEW_ORDER,
        {
            10,
            Side::BUY,
            10000,
            20,
            20,
            OrderType::LIMIT
        },
        0
    };

    ServerResponse newOrderResponse =
        server.handleRequest(newOrderRequest);

    std::cout << "Response type is NEW_ORDER: "
              << (newOrderResponse.type == RequestType::NEW_ORDER)
              << '\n';

    std::cout << "Order status is ACCEPTED: "
              << (newOrderResponse.processResult.status
                  == ProcessStatus::ACCEPTED)
              << '\n';

    std::cout << "Trades created: "
              << newOrderResponse.processResult.trades.size()
              << '\n';


    std::cout << "\n--- Test 2: CANCEL_ORDER Request ---\n";

    ClientRequest cancelRequest = {
        RequestType::CANCEL_ORDER,
        {},
        10
    };

    ServerResponse cancelResponse =
        server.handleRequest(cancelRequest);

    std::cout << "Response type is CANCEL_ORDER: "
              << (cancelResponse.type == RequestType::CANCEL_ORDER)
              << '\n';

    std::cout << "Cancel success: "
              << cancelResponse.cancelSuccess
              << '\n';


    std::cout << "\n--- Test 3: Cancel Same Order Again ---\n";

    ServerResponse cancelAgainResponse =
        server.handleRequest(cancelRequest);

    std::cout << "Cancel success: "
              << cancelAgainResponse.cancelSuccess
              << '\n';


    return 0;
}