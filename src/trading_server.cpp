#include <stdexcept>
#include "trading/trading_server.hpp"


TradingServer::TradingServer(std::size_t expectedOrders):matchingEngine(expectedOrders){}


ServerResponse TradingServer::handleRequest(const ClientRequest& request){
    if (request.type == RequestType::NEW_ORDER){
        return {request.type, matchingEngine.processOrder(request.order), false};
    }else if (request.type == RequestType::CANCEL_ORDER){
        return {request.type, {ProcessStatus::ACCEPTED, {}}, matchingEngine.cancelOrder(request.orderID)};
    }
    throw std::runtime_error("Invalid Request");
}

