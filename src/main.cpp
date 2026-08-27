#include <iostream>
#include <iomanip>
#include "trading/trading_server.hpp"
#include "trading/client_requests.hpp"
#include "trading/server_response.hpp"
#include "trading/order.hpp"
#include "trading/protocol.hpp"

using namespace std;


int main() {

    ProcessResult originalResult;

    originalResult.status = ProcessStatus::ACCEPTED;

    originalResult.trades.push_back({
        0x0102030405060708ULL,
        0x1112131415161718ULL,
        10000,
        20
    });

    originalResult.trades.push_back({
        0x2122232425262728ULL,
        0x3132333435363738ULL,
        10100,
        30
    });

    // Serialize
    std::vector<uint8_t> serialized =
        serializeProcessResult(originalResult);

    // Deserialize
    ProcessResult recoveredResult =
        deserializeProcessResult(serialized);


    std::cout << "--- ProcessResult Round-Trip Test ---\n";

    std::cout << "Status match: "
              << (originalResult.status == recoveredResult.status)
              << '\n';

    std::cout << "Trade count match: "
              << (originalResult.trades.size() ==
                  recoveredResult.trades.size())
              << '\n';


    for (std::size_t i = 0; i < originalResult.trades.size(); i++) {

        const Trade& original = originalResult.trades[i];
        const Trade& recovered = recoveredResult.trades[i];

        std::cout << "\nTrade #" << i + 1 << '\n';

        std::cout << "Buy Order ID match: "
                  << (original.buyOrderID == recovered.buyOrderID)
                  << '\n';

        std::cout << "Sell Order ID match: "
                  << (original.sellOrderID == recovered.sellOrderID)
                  << '\n';

        std::cout << "Price match: "
                  << (original.price == recovered.price)
                  << '\n';

        std::cout << "Quantity match: "
                  << (original.quantity == recovered.quantity)
                  << '\n';
    }

    return 0;
}