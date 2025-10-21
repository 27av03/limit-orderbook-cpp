#include "OrderBook.h"
#include <iostream>
#include <iomanip>

using namespace orderbook;

void printOrderBook(const OrderBook &book)
{
    std::cout << "\n=== Order Book ===" << std::endl;

    auto bestBid = book.getBestBid();
    auto bestAsk = book.getBestAsk();
    auto spread = book.getSpread();

    std::cout << "Best Bid: " << (bestBid ? std::to_string(*bestBid) : "N/A") << std::endl;
    std::cout << "Best Ask: " << (bestAsk ? std::to_string(*bestAsk) : "N/A") << std::endl;
    std::cout << "Spread: " << (spread ? std::to_string(*spread) : "N/A") << std::endl;
    std::cout << "Total Orders: " << book.getOrderCount() << std::endl;
}

void onTrade(const Trade &trade)
{
    std::cout << "\n*** TRADE EXECUTED ***" << std::endl;
    std::cout << "Buy Order ID: " << trade.buyOrderId << std::endl;
    std::cout << "Sell Order ID: " << trade.sellOrderId << std::endl;
    std::cout << "Price: " << std::fixed << std::setprecision(2) << trade.price << std::endl;
    std::cout << "Quantity: " << trade.quantity << std::endl;
    std::cout << "Timestamp: " << trade.timestamp << std::endl;
}

int main()
{
    std::cout << "OrderBook Demo Application" << std::endl;
    std::cout << "=========================" << std::endl;

    OrderBook book;
    book.setTradeCallback(onTrade);

    // Add some sample orders
    std::cout << "\nAdding sample orders..." << std::endl;

    // Add buy orders
    auto buy1 = std::make_shared<Order>(1, OrderSide::BUY, 100.50, 100);
    auto buy2 = std::make_shared<Order>(2, OrderSide::BUY, 100.25, 200);
    auto buy3 = std::make_shared<Order>(3, OrderSide::BUY, 99.75, 150);

    // Add sell orders
    auto sell1 = std::make_shared<Order>(4, OrderSide::SELL, 101.00, 100);
    auto sell2 = std::make_shared<Order>(5, OrderSide::SELL, 101.25, 200);
    auto sell3 = std::make_shared<Order>(6, OrderSide::SELL, 99.50, 50); // This should match with buy orders

    book.addOrder(buy1);
    book.addOrder(buy2);
    book.addOrder(buy3);
    book.addOrder(sell1);
    book.addOrder(sell2);
    book.addOrder(sell3);

    printOrderBook(book);

    // Add a market-crossing order
    std::cout << "\nAdding market-crossing sell order..." << std::endl;
    auto marketSell = std::make_shared<Order>(7, OrderSide::SELL, 100.00, 75);
    book.addOrder(marketSell);

    printOrderBook(book);

    // Cancel an order
    std::cout << "\nCancelling order ID 2..." << std::endl;
    book.cancelOrder(2);
    printOrderBook(book);

    // Modify an order
    std::cout << "\nModifying order ID 1 (price: 100.50 -> 100.75, quantity: 100 -> 125)..." << std::endl;
    book.modifyOrder(1, 100.75, 125);
    printOrderBook(book);

    std::cout << "\nDemo completed!" << std::endl;
    return 0;
}

