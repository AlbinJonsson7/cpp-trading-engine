#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <cstddef>
#include <cstdint>
#include <array>
#include <vector>
#include "order.hpp"
#include "trade.hpp"
#include "process_result.hpp"


inline constexpr std::size_t NEW_ORDER_REQUEST_SIZE = 27;
inline constexpr std::size_t CANCEL_ORDER_REQUEST_SIZE = 9;
inline constexpr std::size_t TRADE_SERIALIZED_SIZE = 28;
inline constexpr std::size_t NEW_ORDER_RESPONSE_HEADER_SIZE = 6;
inline constexpr std::size_t CANCEL_ORDER_RESPONSE_SIZE = 2;


std::array<uint8_t,NEW_ORDER_REQUEST_SIZE> serializeNewOrderRequest(const Order& order);
std::array<uint8_t,CANCEL_ORDER_REQUEST_SIZE> serializeCancellationRequest(uint64_t orderID);
Order deserializeNewOrderRequest(const std::array<uint8_t,NEW_ORDER_REQUEST_SIZE>& data);
uint64_t deserializeCancellationRequest(const std::array<uint8_t,CANCEL_ORDER_REQUEST_SIZE>& data);
std::array<uint8_t, TRADE_SERIALIZED_SIZE> serializeTrade(const Trade& trade);
Trade deserializeTrade(const std::array<uint8_t, TRADE_SERIALIZED_SIZE>& data);
std::vector<uint8_t> serializeProcessResult(const ProcessResult& processResult);
ProcessResult deserializeProcessResult(const std::vector<uint8_t>& data);
std::array<uint8_t,CANCEL_ORDER_RESPONSE_SIZE> serializeCancellationResponse(bool cancelSuccess);
bool deserializeCancellationResponse(const std::array<uint8_t,CANCEL_ORDER_RESPONSE_SIZE>& data);


#endif // PROTOCOL_HPP