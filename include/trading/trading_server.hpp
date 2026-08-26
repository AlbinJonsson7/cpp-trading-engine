#ifndef TRADING_SERVER_HPP
#define TRADING_SERVER_HPP


#include "server_response.hpp"
#include "matching_engine.hpp"
#include "client_requests.hpp"


class TradingServer {
    private:
        MatchingEngine matchingEngine;

    public:
        ServerResponse handleRequest(const ClientRequest& request);

};


#endif // TRADING_SERVER_HPP