#include <cstdint>
#include <algorithm>
#include "trading/protocol.hpp"
#include "trading/client_requests.hpp"


std::array<uint8_t,NEW_ORDER_REQUEST_SIZE> serializeNewOrderRequest(const Order& order){

    std::array<uint8_t,NEW_ORDER_REQUEST_SIZE> serializeArray{};

    serializeArray[0] = static_cast<uint8_t>(RequestType::NEW_ORDER);

    for(int i = 0;i < 8; i++){
        serializeArray[i+1] = static_cast<uint8_t>((order.orderID >> (56-i*8)) & 0xFF);
    }

    serializeArray[9] = static_cast<uint8_t>(order.side);

    for(int i = 0;i < 8; i++){
        serializeArray[i+10] = static_cast<uint8_t>((order.price >> (56-i*8)) & 0xFF);
    }

    for(int i = 0;i < 4; i++){
        serializeArray[i+18] = static_cast<uint8_t>((order.originalQuantity >> (24-i*8)) & 0xFF);
    }

    for(int i = 0;i < 4; i++){
        serializeArray[i+22] = static_cast<uint8_t>((order.remainingQuantity >> (24-i*8)) & 0xFF);
    }

    serializeArray[26] = static_cast<uint8_t>(order.orderType);

    return serializeArray;
}


std::array<uint8_t,CANCEL_ORDER_REQUEST_SIZE> serializeCancellationRequest(uint64_t orderID){

    std::array<uint8_t,CANCEL_ORDER_REQUEST_SIZE> serializeArray{};

    serializeArray[0] = static_cast<uint8_t>(RequestType::CANCEL_ORDER);

    for(int i = 0; i<8;i++){
        serializeArray[i+1] = static_cast<uint8_t>((orderID >> (56-i*8)) & 0xFF);
    }

    return serializeArray;
}


Order deserializeNewOrderRequest(const std::array<uint8_t,NEW_ORDER_REQUEST_SIZE>& data){
    Order order{};

    for(int i = 1; i < 9; i++){
        order.orderID = (static_cast<uint64_t>(order.orderID << 8) | data[i]);
    }

    if(data[9] == static_cast<uint8_t>(Side::BUY)){
        order.side = Side::BUY;
    }else if(data[9] == static_cast<uint8_t>(Side::SELL)){
        order.side = Side::SELL;
    }

    for(int i = 10; i < 18; i++){
        order.price = (static_cast<uint64_t>(order.price << 8) | data[i]);
    }

    for(int i = 18; i < 22; i++){
        order.originalQuantity = (static_cast<uint32_t>(order.originalQuantity << 8) | data[i]);
    }

    for(int i = 22; i < 26; i++){
        order.remainingQuantity = (static_cast<uint32_t>(order.remainingQuantity << 8) | data[i]);
    }

    if(data[26] == 0x00){
        order.orderType = OrderType::MARKET;
    }else if(data[26] == 0x01){
        order.orderType = OrderType::LIMIT;
    }

    return order;
}


uint64_t deserializeCancellationRequest(const std::array<uint8_t,CANCEL_ORDER_REQUEST_SIZE>& data){
    uint64_t deserializedData = 0;
    for(int i = 1; i < 9; i++){
        deserializedData = (static_cast<uint64_t>(deserializedData << 8) | data[i]);
    }
    return deserializedData;
}


std::array<uint8_t, TRADE_SERIALIZED_SIZE> serializeTrade(const Trade& trade){
    std::array<uint8_t, TRADE_SERIALIZED_SIZE> serializeArray{};

    for(int i = 0;i<8;i++){
        serializeArray[i] = static_cast<uint8_t>((trade.buyOrderID >> (56 - 8*i)) & 0xFF);
    }

    for(int i = 0;i<8;i++){
        serializeArray[i+8] = static_cast<uint8_t>((trade.sellOrderID >> (56 - 8*i)) & 0xFF);
    }

    for(int i = 0;i<8;i++){
        serializeArray[i+16] = static_cast<uint8_t>((trade.price >> (56 - 8*i)) & 0xFF);
    }

    for(int i = 0;i<4;i++){
        serializeArray[i+24] = static_cast<uint8_t>((trade.quantity >> (24 - 8*i)) & 0xFF);
    }

    return serializeArray;
}


Trade deserializeTrade(const std::array<uint8_t, TRADE_SERIALIZED_SIZE>& data){
    Trade trade{};

    for(int i = 0; i < 8;i++){
        trade.buyOrderID = static_cast<uint64_t>((trade.buyOrderID << 8)| data[i]);
    }

    for(int i = 8; i < 16;i++){
        trade.sellOrderID = static_cast<uint64_t>((trade.sellOrderID << 8)| data[i]);
    }

    for(int i = 16; i < 24;i++){
        trade.price = static_cast<uint64_t>((trade.price << 8)| data[i]);
    }

    for(int i = 24; i < 28;i++){
        trade.quantity = static_cast<uint32_t>((trade.quantity << 8)| data[i]);
    }

    return trade;
}

std::vector<uint8_t> serializeProcessResult(const ProcessResult& processResult){
    std::vector<uint8_t> resultVector(NEW_ORDER_RESPONSE_HEADER_SIZE);

    resultVector[0] = static_cast<uint8_t>(RequestType::NEW_ORDER);

    resultVector[1] = static_cast<uint8_t>(processResult.status);

    uint32_t tradeCount = static_cast<uint32_t>(processResult.trades.size());

    for(int i = 0; i < 4;i++){
        resultVector[i+2] = static_cast<uint8_t>(tradeCount >> (24 - 8*i) & 0xFF);
    }

    for(const auto& trade:processResult.trades){
        auto serializedTrade = serializeTrade(trade);
        resultVector.insert(resultVector.end(),serializedTrade.begin(),serializedTrade.end());
    }

    return resultVector;
}


ProcessResult deserializeProcessResult(const std::vector<uint8_t>& data){
    ProcessResult processResult{};

    if(data[1] == static_cast<uint8_t>(ProcessStatus::ACCEPTED)){
        processResult.status = ProcessStatus::ACCEPTED;
    }else if(data[1] == static_cast<uint8_t>(ProcessStatus::REJECTED_DUPLICATE_ID)){
        processResult.status = ProcessStatus::REJECTED_DUPLICATE_ID;
    }else if(data[1] == static_cast<uint8_t>(ProcessStatus::REJECTED_ZERO_QUANTITY)){
        processResult.status = ProcessStatus::REJECTED_ZERO_QUANTITY;
    }else if(data[1] == static_cast<uint8_t>(ProcessStatus::REJECTED_INVALID_PRICE)){
        processResult.status = ProcessStatus::REJECTED_INVALID_PRICE;
    }else if(data[1] == static_cast<uint8_t>(ProcessStatus::REJECTED_QUANTITY_TOO_LARGE)){
        processResult.status = ProcessStatus::REJECTED_QUANTITY_TOO_LARGE;
    }

    uint32_t tradeCount = 0;

    for(int i = 2; i < 6;i++){
        tradeCount = static_cast<uint32_t>((tradeCount << 8)| data[i]);
    }
    
    for(int i = 0; i<tradeCount;i++){
        std::array<uint8_t,TRADE_SERIALIZED_SIZE> tradeArr;
        std::size_t start = NEW_ORDER_RESPONSE_HEADER_SIZE + i * TRADE_SERIALIZED_SIZE;
        std::copy_n(data.begin() + start, TRADE_SERIALIZED_SIZE, tradeArr.begin());
        processResult.trades.push_back(deserializeTrade(tradeArr));
    }

    return processResult;
}


std::array<uint8_t,CANCEL_ORDER_RESPONSE_SIZE> serializeCancellationResponse(bool cancelSuccess){
    std::array<uint8_t, CANCEL_ORDER_RESPONSE_SIZE> serializeArray{};

    serializeArray[0] = static_cast<uint8_t>(RequestType::CANCEL_ORDER);
    serializeArray[1] = static_cast<uint8_t>(cancelSuccess);

    return serializeArray;
}


bool deserializeCancellationResponse(const std::array<uint8_t,CANCEL_ORDER_RESPONSE_SIZE>& data){
    return data[1] != 0;
}

/*
Trades: 

buyOrderID   → 8 bytes

sellOrderID  → 8 bytes

price        → 8 bytes

quantity     → 4 bytes
*/

/*
New Order:

byte 0        → RequestType

bytes 1–8     → orderID

byte 9        → Side

bytes 10–17   → price

bytes 18–21   → originalQuantity

bytes 22–25   → remainingQuantity

byte 26       → OrderType
*/

/*
Cancellation order:

byte 0      RequestType::CANCEL_ORDER

bytes 1–8   orderID
*/
