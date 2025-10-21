# High-Performance Limit Order Book

A high-performance limit order book engine built in C++17, demonstrating the core order matching logic used in stock exchanges and trading platforms. Features price-time priority matching, comprehensive testing, and modern C++ architecture.

## Why I Built This

Order books are fundamental to all trading systems - they're the core infrastructure that powers every stock exchange and trading platform. I built this project to demonstrate my understanding of:

- **High-performance C++ systems programming** - choosing efficient data structures for microsecond-level operations
- **Financial market microstructure** - how real trading systems match buyers and sellers
- **Algorithm optimization** - achieving O(log n) performance for order operations
- **Production-quality engineering** - comprehensive testing, clean architecture, modern practices

This project simulates the matching engine that handles billions of orders per day on exchanges like NYSE, NASDAQ, and TSX.

## Features

- ✅ **Efficient Order Matching**: O(log n) insertion and retrieval using std::map
- ✅ **Price-Time Priority**: Orders matched by best price first, then FIFO within price levels
- ✅ **Full Order Management**: Add, cancel, and modify orders with automatic re-matching
- ✅ **Real-Time Execution**: Automatic trade execution when compatible orders cross
- ✅ **Trade Notifications**: Configurable callbacks for executed trades
- ✅ **Modern C++17**: Smart pointers, move semantics, RAII, const correctness
- ✅ **Comprehensive Testing**: 55 unit tests with 92.7% pass rate using Google Test
- ✅ **Clean Architecture**: Professional code structure and separation of concerns

## Architecture

### Core Components

**Order Structure**: Represents individual limit orders with price, quantity, side, and timestamp

**OrderBook Class**: Main matching engine managing bid/ask sides and order execution

**Data Structures**:
- **Bids**: `std::map<double, std::vector<OrderPtr>>` sorted descending (highest price first)
- **Asks**: `std::map<double, std::vector<OrderPtr>>` sorted ascending (lowest price first)  
- **Order Lookup**: `std::map<uint64_t, OrderPtr>` for O(1) order access by ID

### Key Design Decisions

- **Price-Time Priority**: Industry-standard matching algorithm - best price wins, ties broken by timestamp (FIFO)
- **Smart Pointers**: Automatic memory management with `std::shared_ptr`
- **Immutable Orders**: Orders can't be modified after creation (modify = cancel + new order)
- **Mid-Price Execution**: Trades execute at the midpoint between bid and ask for fairness

## Building the Project

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.15 or higher
- Google Test (automatically downloaded via CMake FetchContent)

### Build Instructions

**Linux/macOS:**
```bash
git clone https://github.com/27av03/limit-orderbook-cpp.git
cd limit-orderbook-cpp
chmod +x build.sh
./build.sh
```

**Windows (PowerShell):**
```powershell
git clone https://github.com/27av03/limit-orderbook-cpp.git
cd limit-orderbook-cpp
.\build.bat
```

**Manual Build:**
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./orderbook_tests    # Run tests
./orderbook_main     # Run demo
```

## Usage Example
```cpp
#include "OrderBook.h"
#include <iostream>

using namespace orderbook;

int main() {
    OrderBook book;
    
    // Set up trade notification callback
    book.setTradeCallback([](const Trade& trade) {
        std::cout << "Trade: " << trade.quantity 
                  << " shares @ $" << trade.price << std::endl;
    });
    
    // Add buy order at $100.50 for 100 shares
    auto buyOrder = std::make_shared<Order>(1, OrderSide::BUY, 100.50, 100);
    book.addOrder(buyOrder);
    
    // Add sell order at $100.25 for 100 shares - this crosses the spread!
    auto sellOrder = std::make_shared<Order>(2, OrderSide::SELL, 100.25, 100);
    book.addOrder(sellOrder);  // Triggers automatic matching
    
    // Query book state
    std::cout << "Best Bid: $" << *book.getBestBid() << std::endl;
    std::cout << "Best Ask: $" << *book.getBestAsk() << std::endl;
    std::cout << "Spread: $" << *book.getSpread() << std::endl;
    
    return 0;
}
```

## Demo Output
```
OrderBook Demo Application
=========================

Adding sample orders...

*** TRADE EXECUTED ***
Buy Order ID: 1
Sell Order ID: 6
Price: 100.00
Quantity: 50
Timestamp: 1761087118983187

=== Order Book ===
Best Bid: 100.500000
Best Ask: 101.000000
Spread: 0.500000
Total Orders: 5

Adding market-crossing sell order...

*** TRADE EXECUTED ***
Buy Order ID: 1
Sell Order ID: 7
Price: 100.25
Quantity: 50

*** TRADE EXECUTED ***
Buy Order ID: 2
Sell Order ID: 7
Price: 100.12
Quantity: 25

=== Order Book ===
Best Bid: 100.250000
Best Ask: 101.000000
Spread: 0.750000
Total Orders: 4

Demo completed!
```

## API Reference

### Order Structure
```cpp
struct Order {
    uint64_t orderId;      // Unique identifier
    OrderSide side;        // BUY or SELL
    double price;          // Limit price
    uint64_t quantity;     // Order size
    uint64_t timestamp;    // Creation time (microseconds)
};
```

### OrderBook Class

**Core Operations:**
- `bool addOrder(OrderPtr order)` - Add order to book
- `bool cancelOrder(uint64_t orderId)` - Cancel existing order
- `bool modifyOrder(uint64_t orderId, double newPrice, uint64_t newQty)` - Modify order
- `void clear()` - Clear all orders

**Query Methods:**
- `std::optional<double> getBestBid()` - Highest bid price
- `std::optional<double> getBestAsk()` - Lowest ask price
- `std::optional<double> getSpread()` - Bid-ask spread
- `uint64_t getDepthAtPrice(double price, OrderSide side)` - Quantity at price level
- `size_t getOrderCount()` - Total active orders

**Configuration:**
- `void setTradeCallback(TradeCallback callback)` - Set trade notification handler

### Trade Structure
```cpp
struct Trade {
    uint64_t buyOrderId;    // Buy order ID
    uint64_t sellOrderId;   // Sell order ID
    double price;           // Execution price
    uint64_t quantity;      // Trade size
    uint64_t timestamp;     // Trade time
};
```

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| Add Order | O(log n) | Map insertion at price level |
| Cancel Order | O(log n) | Price level lookup + vector search |
| Match Orders | O(log n + k) | Price level access + matching loop |
| Query Best Bid/Ask | O(log n) | Map begin/end operations |
| Memory Usage | O(n) | Linear in number of active orders |

## Testing

Built with test-driven development principles:

- **55 unit tests** covering core functionality and edge cases
- **92.7% pass rate** (51/55 passing - 4 advanced scenarios in active development)
- **Google Test framework** for professional-grade testing infrastructure
- **Test coverage includes**: 
  - Basic order operations (add, cancel, modify)
  - Complete and partial order matching
  - Multi-level matching across price levels
  - Price-time priority enforcement
  - Market-crossing orders
  - Edge cases and error handling

**Run tests:**
```bash
./orderbook_tests
```

## What I Learned

Building this project taught me valuable lessons about:

**Performance Optimization**: Choosing the right data structures matters - I explored std::map vs hash tables vs priority queues, ultimately choosing std::map for its O(log n) ordered operations that match how order books work in practice.

**Financial Systems Design**: Understanding how microsecond-level performance impacts real trading - exchanges process billions of orders daily, so even small inefficiencies compound.

**Modern C++17**: Practical application of smart pointers for memory safety, move semantics for efficiency, RAII for resource management, and const correctness for API clarity.

**Test-Driven Development**: Writing tests first helped me design better APIs and catch edge cases early (like handling partial fills across multiple price levels).

**Build Systems**: Setting up CMake with FetchContent to automatically download dependencies makes the project portable and easy for others to build.

## Roadmap

### Phase 1: Core Engine ✅ (Completed)
- ✅ Price-time priority matching algorithm
- ✅ Order management (add, cancel, modify)
- ✅ Comprehensive test suite (55 tests)
- ✅ Clean CMake build system with cross-platform support
- ✅ Professional code structure and documentation

### Phase 2: Message Processing 🔄 (In Progress)
- 🔄 FIX protocol message parser for industry-standard order formats
- 🔄 Performance benchmarking suite (target: 10,000+ orders/second)
- 🔄 Resolve remaining 4 test edge cases

### Phase 3: Distributed Systems ⏳ (Planned)
- ⏳ Apache Kafka integration for distributed order processing
- ⏳ Market orders and additional order types (stop-loss, iceberg)
- ⏳ Order book depth visualization
- ⏳ Multi-threading support for concurrent order processing

## Design Patterns & Best Practices

- **RAII**: Automatic resource management with smart pointers
- **Move Semantics**: Efficient object transfer avoiding unnecessary copies
- **Const Correctness**: Clear API contracts with const methods and parameters
- **Exception Safety**: Strong exception guarantees throughout
- **Single Responsibility**: Each class has one clear purpose
- **Dependency Injection**: Configurable callbacks for extensibility

## About

Built by **Anay Varma** as part of exploring career opportunities in financial technology and capital markets. This project demonstrates my passion for high-performance systems and understanding of how technology powers modern financial markets.

I'm a CS/BBA student at Wilfrid Laurier University combining technical skills (C++, Python, Java) with business acumen. Actively seeking Summer 2026 software engineering internships in financial technology.

**Connect with me:**
- LinkedIn: [linkedin.com/in/anay-varma](https://linkedin.com/in/anay-varma)
- GitHub: [github.com/27av03](https://github.com/27av03)
- Email: varm9594@mylaurier.ca

## License

This project is intended for educational and portfolio purposes. Feel free to explore the code and learn from it. If you're a recruiter or hiring manager, I'm happy to discuss the implementation details and design decisions!

---

*Built with ❤️ for financial technology*
