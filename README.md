# High-Performance Limit Order Book

A production-quality C++ implementation of a limit order book designed for financial technology applications. This implementation provides efficient order matching, price-time priority execution, and comprehensive order management capabilities.

## Features

- **Efficient Data Structures**: Uses `std::map` for O(log n) insertion and retrieval operations
- **Price-Time Priority**: Orders are matched based on price priority, with time as the tiebreaker
- **Order Management**: Support for adding, canceling, and modifying orders
- **Real-time Matching**: Automatic order matching when compatible orders are added
- **Trade Callbacks**: Configurable callbacks for trade execution notifications
- **Modern C++17**: Uses contemporary C++ features and best practices
- **Comprehensive Testing**: Full unit test coverage using Google Test framework
- **Clean Architecture**: Well-structured code with clear separation of concerns

## Architecture

### Core Components

1. **Order Structure**: Represents individual limit orders with metadata
2. **OrderBook Class**: Main engine managing order matching and book state
3. **Data Structures**: 
   - Bids: `std::map<double, std::vector<OrderPtr>>` (highest price first)
   - Asks: `std::map<double, std::vector<OrderPtr>>` (lowest price first)
   - Order lookup: `std::map<uint64_t, OrderPtr>` for O(1) access

### Key Design Decisions

- **Price-Time Priority**: Orders are matched first by price, then by timestamp (FIFO)
- **Smart Pointers**: Uses `std::shared_ptr` for automatic memory management
- **Immutable Orders**: Orders cannot be modified after creation (modify = cancel + add)
- **Trade Execution**: Mid-price execution for matched orders

## Building the Project

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16 or higher
- Google Test framework

### Build Instructions

```bash
# Clone the repository
git clone <repository-url>
cd OrderBook

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make -j$(nproc)

# Run tests
./orderbook_tests

# Run demo
./orderbook_main
```

### Windows (Visual Studio)

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

## Usage Example

```cpp
#include "OrderBook.h"
#include <iostream>

using namespace orderbook;

int main() {
    OrderBook book;
    
    // Set up trade callback
    book.setTradeCallback([](const Trade& trade) {
        std::cout << "Trade executed: " << trade.quantity 
                  << " shares at $" << trade.price << std::endl;
    });
    
    // Add buy order
    auto buyOrder = std::make_shared<Order>(1, OrderSide::BUY, 100.50, 100);
    book.addOrder(buyOrder);
    
    // Add sell order
    auto sellOrder = std::make_shared<Order>(2, OrderSide::SELL, 100.25, 100);
    book.addOrder(sellOrder); // This will trigger a match!
    
    // Check book state
    std::cout << "Best bid: $" << *book.getBestBid() << std::endl;
    std::cout << "Best ask: $" << *book.getBestAsk() << std::endl;
    std::cout << "Spread: $" << *book.getSpread() << std::endl;
    
    return 0;
}
```

## API Reference

### Order Structure

```cpp
struct Order {
    std::uint64_t orderId;     // Unique identifier
    OrderSide side;            // BUY or SELL
    double price;              // Limit price
    std::uint64_t quantity;    // Order quantity
    std::uint64_t timestamp;   // Creation timestamp (microseconds)
};
```

### OrderBook Class

#### Core Methods

- `bool addOrder(OrderPtr order)` - Add order to the book
- `bool cancelOrder(std::uint64_t orderId)` - Cancel existing order
- `bool modifyOrder(std::uint64_t orderId, double newPrice, std::uint64_t newQuantity)` - Modify order
- `void clear()` - Clear all orders

#### Query Methods

- `std::optional<double> getBestBid()` - Get highest bid price
- `std::optional<double> getBestAsk()` - Get lowest ask price
- `std::optional<double> getSpread()` - Get bid-ask spread
- `std::uint64_t getDepthAtPrice(double price, OrderSide side)` - Get quantity at price level
- `std::size_t getOrderCount()` - Get total number of orders

#### Configuration

- `void setTradeCallback(TradeCallback callback)` - Set trade notification callback

### Trade Structure

```cpp
struct Trade {
    std::uint64_t buyOrderId;   // ID of buy order
    std::uint64_t sellOrderId;  // ID of sell order
    double price;               // Execution price (mid-price)
    std::uint64_t quantity;     // Trade quantity
    std::uint64_t timestamp;    // Trade timestamp
};
```

## Performance Characteristics

- **Order Addition**: O(log n) due to map insertion
- **Order Cancellation**: O(log n) for price level lookup + O(m) for vector search
- **Order Matching**: O(log n) for price level access + O(k) for matching loop
- **Memory Usage**: O(n) where n is the number of active orders

## Testing

The project includes comprehensive unit tests covering:

- Order creation and comparison
- Order book operations (add, cancel, modify)
- Order matching scenarios
- Price-time priority enforcement
- Edge cases and error handling

Run tests with:
```bash
./orderbook_tests
```

## Design Patterns and Best Practices

- **RAII**: Automatic resource management with smart pointers
- **Move Semantics**: Efficient object transfer where possible
- **Const Correctness**: Proper use of const methods and parameters
- **Exception Safety**: Strong exception safety guarantees
- **Single Responsibility**: Clear separation of concerns
- **Dependency Injection**: Configurable callbacks for extensibility

## Future Enhancements

- [ ] Iceberg order support
- [ ] Market data feed integration
- [ ] Order book snapshots and incremental updates
- [ ] Performance benchmarking suite
- [ ] Multi-threading support
- [ ] Order book reconstruction from trade log

## License

This project is intended for educational and portfolio purposes. Please ensure compliance with relevant regulations before using in production financial systems.

