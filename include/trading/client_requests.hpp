#ifndef CLIENT_REQUESTS_HPP
#define CLIENT_REQUESTS_HPP

#include <cstdint>
#include "order.hpp"

enum class RequestType:uint8_t{
    NEW_ORDER,
    CANCEL_ORDER
};


struct ClientRequest{
    RequestType type;
    Order order;
    uint64_t orderID;
};


#endif // CLIENT_REQUESTS_HPP