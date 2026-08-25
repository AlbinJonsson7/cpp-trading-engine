#include <iostream>
#include <algorithm>
#include "trading/matching_engine.hpp"



std::vector<Trade> MatchingEngine::processOrder(Order incomingOrder){

    std::vector<Trade> currentVector;

    if(incomingOrder.side == Side::BUY){
        while(incomingOrder.remainingQuantity > 0){
            if(!orderBook.hasAsks()){
                if(incomingOrder.orderType == OrderType::MARKET) return currentVector;
                orderBook.addOrder(incomingOrder);
                return currentVector;
            }
            std::optional<int64_t> bestAskPrice = orderBook.getBestAsk();

            if(incomingOrder.price < *bestAskPrice && incomingOrder.orderType == OrderType::LIMIT){
                orderBook.addOrder(incomingOrder);
                return currentVector;
            }

            std::optional<Order> bestAskOrderOptional = orderBook.getBestAskOrder();

            if (!bestAskOrderOptional) {
                return currentVector;
            }
            Order bestAskOrder = *bestAskOrderOptional;

            uint32_t tradeQuantity = std::min(incomingOrder.remainingQuantity,bestAskOrder.remainingQuantity);

            Trade trade = {.buyOrderID = incomingOrder.orderID, .sellOrderID = bestAskOrder.orderID, .price = bestAskOrder.price, .quantity = tradeQuantity};
            currentVector.push_back(trade);

            orderBook.fillBestOrder(Side::SELL,tradeQuantity);

            incomingOrder.remainingQuantity -= tradeQuantity;
        }
    }else if (incomingOrder.side == Side::SELL){
        while(incomingOrder.remainingQuantity > 0){
            if(!orderBook.hasBids()){
                if(incomingOrder.orderType == OrderType::MARKET) return currentVector;
                orderBook.addOrder(incomingOrder);
                return currentVector;
            }

            std::optional<int64_t> bestBidPrice = orderBook.getBestBid();
            
            if(incomingOrder.price > *bestBidPrice && incomingOrder.orderType == OrderType::LIMIT){
                orderBook.addOrder(incomingOrder);
                return currentVector;
            }

            std::optional<Order> bestBidOrderOptional = orderBook.getBestBidOrder();

            if (!bestBidOrderOptional) {
                return currentVector;
            }

            Order bestBidOrder = *bestBidOrderOptional;

            uint32_t tradeQuantity = std::min(incomingOrder.remainingQuantity,bestBidOrder.remainingQuantity);

            Trade trade = {.buyOrderID = bestBidOrder.orderID, .sellOrderID = incomingOrder.orderID, .price = bestBidOrder.price, .quantity = tradeQuantity};
            currentVector.push_back(trade);

            orderBook.fillBestOrder(Side::BUY, tradeQuantity);

            incomingOrder.remainingQuantity -= tradeQuantity;
        }
    }
    

    return currentVector;

}