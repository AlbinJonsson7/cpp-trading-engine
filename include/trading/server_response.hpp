#ifndef SERVER_RESPONSE_HPP
#define SERVER_RESPONSE_HPP

#include <cstdint>
#include "client_requests.hpp"
#include "process_result.hpp"


struct ServerResponse{
    RequestType type;
    ProcessResult processResult;
    bool cancelSuccess;

};


#endif // SERVER_RESPONSE_HPP