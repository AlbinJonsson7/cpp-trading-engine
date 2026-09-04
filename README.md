# C++ Trading Engine

A low-latency limit order book and matching engine written in modern C++20.

The project models the core execution path of an electronic exchange, including limit and market orders, price-time priority, partial fills, cancellations, order lookup, and trade generation.

The implementation was developed with an emphasis on both **correctness** and **data-oriented performance**, with profiling and benchmarking used to guide architectural decisions.

---

## Features

- Limit and market orders
- BUY and SELL order books
- Price-time priority
- FIFO execution within each price level
- Partial fills
- Multi-order matching
- Order cancellation
- Resting-price trade execution
- Monotonically increasing order IDs
- Quantity and price validation
- Capacity handling
- Dedicated correctness test suite
- Repeatable benchmarks
- Cache-conscious order storage

---

## Architecture

The engine is organized into several layers:

```text
MatchingEngine
     |
     v
OrderBook
     |
     +-----------------------+
     |                       |
     v                       v
Bids                     Asks
std::map                 std::map
     |                       |
     v                       v
PriceLevel               PriceLevel
     |
     v
OrderNodePool
     |
     +---- orders[]
     |
     +---- links[]
     |
     +---- freeIndices[]

OrderBook
     |
     v
OrderIndex
     |
     v
Paged direct-address lookup
```

### MatchingEngine

`MatchingEngine` is the main public interface.

It:

- validates incoming orders
- enforces monotonically increasing order IDs
- determines whether an incoming order crosses the book
- performs matching
- generates `Trade` records
- rests unfilled limit-order quantity
- discards unfilled market-order quantity
- forwards cancellation requests to the order book

---

## Matching Rules

### Price Priority

BUY orders execute against the lowest available ask.

SELL orders execute against the highest available bid.

### Time Priority

Orders at the same price execute FIFO.

For example:

```text
SELL #1: 10 @ $102.00
SELL #2: 15 @ $102.00
```

If a BUY crosses that price level, Order #1 executes before Order #2.

### Execution Price

Trades execute at the price of the **resting order**.

Example:

```text
Resting SELL: $102.00
Incoming BUY limit: $103.00
```

Execution occurs at:

```text
$102.00
```

### Partial Fills

If an incoming order cannot completely consume or be consumed by another order, the remaining quantity is updated.

For a partially filled limit order, the remaining quantity may rest in the book.

For a market order, any unmatched remainder is discarded.

---

## Price Representation

Prices are stored as integers rather than floating-point values.

Example:

```text
10200 -> $102.00
```

Using integer prices avoids floating-point rounding errors in matching logic.

---

## Order Book

The book maintains separate bid and ask price maps:

```cpp
std::map<int64_t, PriceLevel, std::greater<int64_t>> bids;
std::map<int64_t, PriceLevel, std::less<int64_t>> asks;
```

This keeps:

- highest bid at the front of the bid map
- lowest ask at the front of the ask map

Each `PriceLevel` maintains FIFO order using intrusive node indices rather than an STL linked list.

---

## Hot / Cold Order Storage

An early version used `std::pmr::list<Order>` for each price level.

Profiling showed that random cancellation was heavily affected by pointer chasing and cache misses.

The final implementation uses a centralized node pool with separate arrays:

```text
orders[]
    Full Order payload

links[]
    previous
    next
    active

freeIndices[]
    Recycled node indices
```

The same index identifies an entry in both `orders[]` and `links[]`.

This separates relatively cold order data from the small linkage structure used during cancellation and FIFO maintenance.

### Compact Link Structure

```cpp
struct OrderLinks {
    uint32_t previous;
    uint32_t next;
    bool active;
};
```

This reduced the amount of memory touched during intrusive-list operations.

The final `Order` structure is arranged to occupy **32 bytes** on the development build.

---

## Order Lookup

Active orders are located using a custom paged direct-address index rather than a hash table.

Order IDs are mapped using:

```text
zeroBasedID = orderID - 1

pageIndex = zeroBasedID / 4096
offset    = zeroBasedID % 4096
```

Each page contains 4096 `OrderLocation` entries.

An `OrderLocation` stores:

```text
node index
PriceLevel pointer
```

This allows cancellation to directly locate both the order node and its containing price level.

### Page Recycling

Empty historical pages are recycled through a page pool.

The current/latest page is retained to avoid repeated allocation around page boundaries.

This design reduced allocation-related latency spikes observed during benchmarking.

---

## Cancellation Path

A cancellation follows approximately:

```text
Order ID
   |
   v
OrderIndex
   |
   v
OrderLocation
   |
   +--> nodeIndex
   |
   +--> PriceLevel*
            |
            v
      unlink OrderLinks
            |
            v
      release node index
```

The order index avoids searching through the book for the requested order.

---

## Project Structure

```text
cpp-trading-engine/
|
├── include/
│   └── trading/
│       ├── matching_engine.hpp
│       ├── order.hpp
│       ├── order_book.hpp
│       ├── order_index.hpp
│       ├── order_location.hpp
│       ├── order_node_pool.hpp
│       ├── price_level.hpp
│       ├── process_status.hpp
│       └── trade.hpp
│
├── src/
│   ├── main.cpp
│   ├── matching_engine.cpp
│   ├── order_book.cpp
│   ├── order_index.cpp
│   ├── order_node_pool.cpp
│   └── price_level.cpp
│
├── tests/
│   └── trading_engine_tests.cpp
│
├── benchmarks/
│   └── matching_engine_benchmark.cpp
│
├── CMakeLists.txt
└── README.md
```

---

## Build

### Requirements

- C++20 compiler
- CMake 3.20+

Configure the project:

```bash
cmake -S . -B build
```

Build:

```bash
cmake --build build --config Release
```

---

## Run the Demo

Build the `trading_engine` target:

```bash
cmake --build build --config Release --target trading_engine
```

On a Windows multi-configuration build:

```bash
build/Release/trading_engine.exe
```

The demo shows:

- resting sell orders
- FIFO execution
- price priority
- a crossing BUY order
- multiple generated trades
- a partial fill
- cancellation of the remaining resting order

Example matching sequence:

```text
SELL #1: 10 @ $102.00
SELL #2: 15 @ $102.00
SELL #3: 20 @ $103.00

BUY #4: 30 @ $103.00
            |
            v
Trade 1: 10 @ $102.00 with SELL #1
Trade 2: 15 @ $102.00 with SELL #2
Trade 3:  5 @ $103.00 with SELL #3
            |
            v
SELL #3 has 15 remaining
            |
            v
Cancel SELL #3
```

---

## Correctness Tests

The project includes a dedicated correctness executable integrated with CTest.

Build the tests:

```bash
cmake --build build --config Debug --target trading_engine_tests
```

Run:

```bash
ctest --test-dir build -C Debug --output-on-failure
```

Current result:

```text
100% tests passed, 0 tests failed
```

The suite covers:

- node acquisition and recycling
- hot/cold link state
- FIFO ordering
- arbitrary node removal
- price-level removal
- cancellations
- best bid / ask behavior
- price-time priority
- partial fills
- resting remainders
- repeated node reuse through the matching engine

---

## Benchmarks

Benchmarks are run using optimized builds and use:

- `std::chrono::steady_clock`
- pre-generated workloads
- fresh engine instance per run
- 10 measured runs
- median latency
- reusable trade buffers
- setup outside the timed region where appropriate

Performance results are hardware-dependent and should be interpreted as relative measurements rather than exchange-grade latency guarantees.

### Current Results

| Workload | Median Cost | Throughput |
|---|---:|---:|
| ADD - 1 price level | 32.14 ns/order | 31.11 M orders/s |
| ADD - 100 price levels | 36.49 ns/order | 27.41 M orders/s |
| CANCEL - 1 level, sequential | 18.14 ns/cancel | 55.12 M cancels/s |
| CANCEL - 100 levels, sequential | 18.54 ns/cancel | 53.94 M cancels/s |
| CANCEL - 100 levels, random | 198.95 ns/cancel | 5.03 M cancels/s |
| MATCH 1-to-1 - 1 level | 19.84 ns/incoming | 50.39 M incoming/s |
| MATCH 1-to-1 - 100 levels | 27.75 ns/incoming | 36.04 M incoming/s |
| MATCH 1-to-10 - 100 levels | 28.01 ns/trade | 35.70 M trades/s |
| Mixed ADD / MATCH / CANCEL | 25.55 ns/op | 39.14 M ops/s |

---

## Performance Optimization

Performance work was measurement-driven rather than based only on theoretical complexity.

One example was random cancellation.

### Initial Production Result

```text
~226.77 ns/cancel
```

Profiling and targeted benchmarks showed that random linked-list node access and removal were major contributors.

A first intrusive implementation using a combined `OrderNode` did not improve the real engine:

```text
Combined intrusive node:
~235.94 ns/cancel
```

This was an important result: replacing the STL list with an intrusive structure alone was not enough.

Further experiments compared a full order node with compact linkage metadata.

The combined node occupied approximately:

```text
56 bytes
```

while the compact linkage structure occupied approximately:

```text
12 bytes
```

A targeted random-unlink benchmark showed a substantial locality improvement with the compact structure.

The final hot/cold architecture therefore separated:

```text
Order payload
```

from:

```text
previous / next / active
```

The resulting end-to-end production benchmark reached approximately:

```text
198.95 ns/cancel
```

while also improving ADD and 1-to-1 matching performance.

The optimization process followed:

```text
Profile
   |
   v
Identify bottleneck
   |
   v
Build targeted microbenchmark
   |
   v
Change data layout
   |
   v
Run correctness suite
   |
   v
Validate with full engine benchmark
```

---

## Selected Optimization Results

The final hot/cold representation improved several real engine workloads compared with the preceding production architecture.

| Workload | Previous | Hot / Cold | Approx. Change |
|---|---:|---:|---:|
| ADD - 1 level | 39.06 ns | 32.14 ns | 17.7% faster |
| ADD - 100 levels | 44.29 ns | 36.49 ns | 17.6% faster |
| CANCEL - 100 levels, random | 226.77 ns | 198.95 ns | 12.3% faster |
| MATCH 1-to-1 - 1 level | 21.67 ns | 19.84 ns | 8.4% faster |
| MATCH 1-to-1 - 100 levels | 30.38 ns | 27.75 ns | 8.7% faster |

Sequential cancellation remained approximately unchanged.

---

## Design Decisions

### Why `std::map` for price levels?

The book currently prioritizes a clear ordered-price representation.

The maps provide direct access to:

```text
highest bid
lowest ask
```

while keeping arbitrary price levels ordered.

A production exchange could replace this with a more specialized price-level structure depending on the expected price range and latency requirements.

### Why Intrusive Node Indices?

Traditional linked containers require pointer-based nodes and typically involve less predictable memory access.

The node pool instead uses integer indices into contiguous arrays.

This provides:

- centralized storage
- reusable node slots
- compact linkage metadata
- improved memory locality
- direct previous/next access without owning pointers

### Why Hot / Cold Storage?

Cancellation primarily needs linkage information, while matching needs the full order payload.

Keeping those separately allows operations that only need linkage metadata to touch a much smaller working set.

```text
Hot:
previous
next
active

Cold:
orderID
price
quantity
side
order type
```

### Why a Custom OrderIndex?

Cancellation requires locating an arbitrary active order quickly.

Rather than searching through the order book or using `std::unordered_map`, this implementation exploits monotonically increasing numeric order IDs and uses paged direct addressing.

This allows an order ID to be mapped directly to a page and offset.

### Why Page Recycling?

Direct-address pages can become empty as orders are filled or cancelled.

Instead of repeatedly destroying and reallocating these pages, empty historical pages are returned to a reusable pool.

The latest page is retained to avoid repeated allocation around page boundaries.

### Why Integer Prices?

Floating-point values can introduce rounding ambiguity.

Prices are therefore represented using scaled integers:

```text
10000 = $100.00
10235 = $102.35
```

Comparison and matching are then performed entirely using integer arithmetic.

### Why a Reusable Trade Buffer?

`processOrder()` accepts a caller-owned `std::vector<Trade>`.

The vector is cleared and reused instead of returning a newly allocated container for every incoming order.

This reduces unnecessary allocation activity in the processing path.

---

## Order Representation

The final `Order` layout is organized to reduce alignment padding:

```cpp
struct Order {
    uint64_t orderID;
    int64_t price;
    uint32_t originalQuantity;
    uint32_t remainingQuantity;
    Side side;
    OrderType orderType;
};
```

On the development compiler this occupies:

```text
32 bytes
```

The layout was intentionally reordered from an earlier version to reduce the structure's memory footprint.

---

## Order ID Policy

The matching engine requires submitted order IDs to be strictly monotonically increasing.

An incoming ID must therefore be greater than every previously submitted ID.

Rejected submissions also consume their ID because the engine records the highest submitted order ID before performing normal validation.

This models an exchange-style sequencing assumption and also supports efficient paged direct addressing.

---

## Capacity Handling

The order book can reject a new resting order if its indexing/storage capacity cannot accommodate it.

If an incoming limit order has already partially executed but its remainder cannot be stored, the matching engine reports:

```text
PARTIALLY_FILLED_CAPACITY
```

If no execution has occurred and the order cannot be stored, it reports:

```text
REJECTED_CAPACITY
```

Insertion failures are rolled back so an unsuccessful order does not remain inside a price level without a corresponding order-index entry.

---

## Current Limitations

This is an educational and portfolio matching engine rather than a production exchange.

Current limitations include:

- single-threaded execution
- in-memory operation only
- no networking layer
- no persistence or recovery
- no market-data feed handler
- no concurrency control
- no exchange protocol implementation
- no advanced order types such as stop or iceberg orders
- price levels currently use `std::map`
- the direct-address order index assumes reasonably dense, monotonically increasing numeric IDs

These constraints are intentional so the project can focus on matching behavior, order-book data structures, correctness, memory layout, and performance analysis.

---

## Technologies

- C++20
- CMake
- STL
- Data-oriented design
- Intrusive data structures
- Performance profiling
- Microbenchmarking
- Cache-conscious memory layout

---

## Status

Core matching-engine implementation: **Complete**

- [x] Limit orders
- [x] Market orders
- [x] BUY / SELL matching
- [x] Price priority
- [x] FIFO time priority
- [x] Partial fills
- [x] Multi-order fills
- [x] Resting-price execution
- [x] Order cancellation
- [x] Input validation
- [x] Capacity handling
- [x] Monotonically increasing order IDs
- [x] Custom paged order index
- [x] Page recycling
- [x] Intrusive FIFO nodes
- [x] Hot / cold order storage
- [x] Node recycling
- [x] Correctness test suite
- [x] CTest integration
- [x] Benchmark suite
- [x] Executable demonstration