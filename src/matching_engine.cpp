#include <iostream>
#include <algorithm>
#include "trading/trade.hpp"
#include "trading/matching_engine.hpp"


MatchingEngine::MatchingEngine(std::size_t expectedOrders):orderBook(expectedOrders){}


ProcessStatus MatchingEngine::processOrder(const Order& incomingOrder, std::vector<Trade>& tradeBuffer){
    tradeBuffer.clear();

    if (orderBook.containsOrder(incomingOrder.orderID)){
        return ProcessStatus::REJECTED_DUPLICATE_ID;
    }else if(incomingOrder.remainingQuantity == 0){
        return ProcessStatus::REJECTED_ZERO_QUANTITY;
    }else if(incomingOrder.remainingQuantity > MAX_ORDER_QUANTITY){
        return ProcessStatus::REJECTED_QUANTITY_TOO_LARGE;
    }else if(incomingOrder.orderType == OrderType::LIMIT && incomingOrder.price <= 0){
        return ProcessStatus::REJECTED_INVALID_PRICE;
    }

    auto remainingQuantity = incomingOrder.remainingQuantity;

    if(incomingOrder.side == Side::BUY){
        while(remainingQuantity > 0){
            auto bestAskOrder = orderBook.getBestAskOrder();

            if(bestAskOrder == nullptr){
                if(incomingOrder.orderType == OrderType::MARKET){
                    return ProcessStatus::ACCEPTED;
                }
                if(remainingQuantity == incomingOrder.remainingQuantity){
                    
                    if(!orderBook.addOrder(incomingOrder)){
                        return ProcessStatus::REJECTED_CAPACITY;
                    }
                    return ProcessStatus::ACCEPTED;
                }
                Order localOrder = incomingOrder;
                localOrder.remainingQuantity = remainingQuantity;
                if(!orderBook.addOrder(localOrder)){
                    return ProcessStatus::PARTIALLY_FILLED_CAPACITY;
                }
                return ProcessStatus::ACCEPTED;
            }
            
            if(incomingOrder.price < bestAskOrder->price && incomingOrder.orderType == OrderType::LIMIT){
                if(remainingQuantity == incomingOrder.remainingQuantity){
                    if(!orderBook.addOrder(incomingOrder)){
                        return ProcessStatus::REJECTED_CAPACITY;
                    }
                    return ProcessStatus::ACCEPTED;
                }
                Order localOrder = incomingOrder;
                localOrder.remainingQuantity = remainingQuantity;
                if(!orderBook.addOrder(localOrder)){
                    return ProcessStatus::PARTIALLY_FILLED_CAPACITY;
                }
                return ProcessStatus::ACCEPTED;
            }

            uint32_t tradeQuantity = std::min(remainingQuantity,bestAskOrder->remainingQuantity);

            Trade trade = {.buyOrderID = incomingOrder.orderID, .sellOrderID = bestAskOrder->orderID, .price = bestAskOrder->price, .quantity = tradeQuantity};
            tradeBuffer.push_back(trade);

            orderBook.fillBestOrder(Side::SELL,tradeQuantity);

            remainingQuantity -= tradeQuantity;
        }
        return ProcessStatus::ACCEPTED;
    }else if (incomingOrder.side == Side::SELL){
        while(remainingQuantity > 0){
            auto bestBidOrder = orderBook.getBestBidOrder();
            

            if(bestBidOrder == nullptr){
                if(incomingOrder.orderType == OrderType::MARKET){
                    return ProcessStatus::ACCEPTED;
                }
                if(remainingQuantity == incomingOrder.remainingQuantity){
                    if(!orderBook.addOrder(incomingOrder)){
                        return ProcessStatus::REJECTED_CAPACITY;
                    }
                    return ProcessStatus::ACCEPTED;
                }
                Order localOrder = incomingOrder;
                localOrder.remainingQuantity = remainingQuantity;
                if(!orderBook.addOrder(localOrder)){
                    return ProcessStatus::PARTIALLY_FILLED_CAPACITY;
                }
                return ProcessStatus::ACCEPTED;
            }

            if(incomingOrder.price > bestBidOrder->price && incomingOrder.orderType == OrderType::LIMIT){
                if(remainingQuantity == incomingOrder.remainingQuantity){
                    if(!orderBook.addOrder(incomingOrder)){
                        return ProcessStatus::REJECTED_CAPACITY;
                    }
                    return ProcessStatus::ACCEPTED;
                }
                Order localOrder = incomingOrder;
                localOrder.remainingQuantity = remainingQuantity;
                if(!orderBook.addOrder(localOrder)){
                    return ProcessStatus::PARTIALLY_FILLED_CAPACITY;
                }
                return ProcessStatus::ACCEPTED;
            }

            uint32_t tradeQuantity = std::min(remainingQuantity,bestBidOrder->remainingQuantity);

            Trade trade = {.buyOrderID = bestBidOrder->orderID, .sellOrderID = incomingOrder.orderID, .price = bestBidOrder->price, .quantity = tradeQuantity};
            tradeBuffer.push_back(trade);

            orderBook.fillBestOrder(Side::BUY, tradeQuantity);

            remainingQuantity -= tradeQuantity;
        }
        return ProcessStatus::ACCEPTED;
    }
    return ProcessStatus::REJECTED_INVALID_SIDE;
}


bool MatchingEngine::cancelOrder(uint64_t orderID){
    return orderBook.cancelOrder(orderID);
}