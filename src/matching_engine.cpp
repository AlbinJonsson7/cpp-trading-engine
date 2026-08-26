#include <iostream>
#include <algorithm>
#include "trading/matching_engine.hpp"



ProcessResult MatchingEngine::processOrder(Order incomingOrder){

    if (orderBook.containsOrder(incomingOrder.orderID)){
        return {ProcessStatus::REJECTED_DUPLICATE_ID,{}};
    }else if(incomingOrder.remainingQuantity == 0){
        return {ProcessStatus::REJECTED_ZERO_QUANTITY,{}};
    }else if(incomingOrder.remainingQuantity > MAX_ORDER_QUANTITY){
        return {ProcessStatus::REJECTED_QUANTITY_TOO_LARGE,{}};
    }else if(incomingOrder.orderType == OrderType::LIMIT && incomingOrder.price <= 0){
        return {ProcessStatus::REJECTED_INVALID_PRICE,{}};
    }
    ProcessResult tradeResult = {.status = ProcessStatus::ACCEPTED,.trades = {}};

    if(incomingOrder.side == Side::BUY){
        while(incomingOrder.remainingQuantity > 0){
            if(!orderBook.hasAsks()){
                if(incomingOrder.orderType == OrderType::MARKET){
                    return tradeResult;
                }
                orderBook.addOrder(incomingOrder);
                return tradeResult;
            }
            std::optional<int64_t> bestAskPrice = orderBook.getBestAsk();

            if(incomingOrder.price < *bestAskPrice && incomingOrder.orderType == OrderType::LIMIT){
                orderBook.addOrder(incomingOrder);
                return tradeResult;
            }

            std::optional<Order> bestAskOrderOptional = orderBook.getBestAskOrder();

            if (!bestAskOrderOptional) {
                return tradeResult;
            }
            Order bestAskOrder = *bestAskOrderOptional;

            uint32_t tradeQuantity = std::min(incomingOrder.remainingQuantity,bestAskOrder.remainingQuantity);

            Trade trade = {.buyOrderID = incomingOrder.orderID, .sellOrderID = bestAskOrder.orderID, .price = bestAskOrder.price, .quantity = tradeQuantity};
            tradeResult.trades.push_back(trade);

            orderBook.fillBestOrder(Side::SELL,tradeQuantity);

            incomingOrder.remainingQuantity -= tradeQuantity;
        }
    }else if (incomingOrder.side == Side::SELL){
        while(incomingOrder.remainingQuantity > 0){
            if(!orderBook.hasBids()){
                if(incomingOrder.orderType == OrderType::MARKET){
                    return tradeResult;
                }
                orderBook.addOrder(incomingOrder);
                return tradeResult;
            }

            std::optional<int64_t> bestBidPrice = orderBook.getBestBid();
            
            if(incomingOrder.price > *bestBidPrice && incomingOrder.orderType == OrderType::LIMIT){
                orderBook.addOrder(incomingOrder);
                return tradeResult;
            }

            std::optional<Order> bestBidOrderOptional = orderBook.getBestBidOrder();

            if (!bestBidOrderOptional) {
                return tradeResult;
            }

            Order bestBidOrder = *bestBidOrderOptional;

            uint32_t tradeQuantity = std::min(incomingOrder.remainingQuantity,bestBidOrder.remainingQuantity);

            Trade trade = {.buyOrderID = bestBidOrder.orderID, .sellOrderID = incomingOrder.orderID, .price = bestBidOrder.price, .quantity = tradeQuantity};
            tradeResult.trades.push_back(trade);

            orderBook.fillBestOrder(Side::BUY, tradeQuantity);

            incomingOrder.remainingQuantity -= tradeQuantity;
        }
    }
    return tradeResult;
}


bool MatchingEngine::cancelOrder(uint64_t orderID){
    return orderBook.cancelOrder(orderID);
}