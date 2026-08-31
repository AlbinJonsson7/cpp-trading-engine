#include <iostream>
#include <algorithm>
#include "trading/matching_engine.hpp"


MatchingEngine::MatchingEngine(std::size_t expectedOrders):orderBook(expectedOrders){}


ProcessResult MatchingEngine::processOrder(const Order& incomingOrder){

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
    auto remainingQuantity = incomingOrder.remainingQuantity;

    if(incomingOrder.side == Side::BUY){
        while(remainingQuantity > 0){
            auto bestAskOrder = orderBook.getBestAskOrder();

            if(bestAskOrder == nullptr){
                if(incomingOrder.orderType == OrderType::MARKET){
                    return tradeResult;
                }
                if(remainingQuantity == incomingOrder.remainingQuantity){
                    orderBook.addOrder(incomingOrder);
                    return tradeResult;
                }
                Order localOrder = incomingOrder;
                localOrder.remainingQuantity = remainingQuantity;
                orderBook.addOrder(localOrder);
                return tradeResult;
            }
            
            if(incomingOrder.price < bestAskOrder->price && incomingOrder.orderType == OrderType::LIMIT){
                if(remainingQuantity == incomingOrder.remainingQuantity){
                    orderBook.addOrder(incomingOrder);
                    return tradeResult;
                }
                Order localOrder = incomingOrder;
                localOrder.remainingQuantity = remainingQuantity;
                orderBook.addOrder(localOrder);
                return tradeResult;
            }

            uint32_t tradeQuantity = std::min(remainingQuantity,bestAskOrder->remainingQuantity);

            Trade trade = {.buyOrderID = incomingOrder.orderID, .sellOrderID = bestAskOrder->orderID, .price = bestAskOrder->price, .quantity = tradeQuantity};
            tradeResult.trades.push_back(trade);

            orderBook.fillBestOrder(Side::SELL,tradeQuantity);

            remainingQuantity -= tradeQuantity;
        }
    }else if (incomingOrder.side == Side::SELL){
        while(remainingQuantity > 0){
            auto bestBidOrder = orderBook.getBestBidOrder();
            

            if(bestBidOrder == nullptr){
                if(incomingOrder.orderType == OrderType::MARKET){
                    return tradeResult;
                }
                if(remainingQuantity == incomingOrder.remainingQuantity){
                    orderBook.addOrder(incomingOrder);
                    return tradeResult;
                }
                Order localOrder = incomingOrder;
                localOrder.remainingQuantity = remainingQuantity;
                orderBook.addOrder(localOrder);
                return tradeResult;
            }

            if(incomingOrder.price > bestBidOrder->price && incomingOrder.orderType == OrderType::LIMIT){
                if(remainingQuantity == incomingOrder.remainingQuantity){
                    orderBook.addOrder(incomingOrder);
                    return tradeResult;
                }
                Order localOrder = incomingOrder;
                localOrder.remainingQuantity = remainingQuantity;
                orderBook.addOrder(localOrder);
                return tradeResult;
            }

            uint32_t tradeQuantity = std::min(remainingQuantity,bestBidOrder->remainingQuantity);

            Trade trade = {.buyOrderID = bestBidOrder->orderID, .sellOrderID = incomingOrder.orderID, .price = bestBidOrder->price, .quantity = tradeQuantity};
            tradeResult.trades.push_back(trade);

            orderBook.fillBestOrder(Side::BUY, tradeQuantity);

            remainingQuantity -= tradeQuantity;
        }
    }
    return tradeResult;
}


bool MatchingEngine::cancelOrder(uint64_t orderID){
    return orderBook.cancelOrder(orderID);
}