#ifndef PROCESS_RESULT_HPP
#define PROCESS_RESULT_HPP

#include <vector>
#include "trade.hpp"


enum class ProcessStatus{
    ACCEPTED,
    REJECTED_DUPLICATE_ID,
    REJECTED_ZERO_QUANTITY,
    REJECTED_INVALID_PRICE,
    REJECTED_QUANTITY_TOO_LARGE
};

struct ProcessResult{
    ProcessStatus status;
    std::vector<Trade> trades;
};


#endif // PROCESS_RESULT_HPP