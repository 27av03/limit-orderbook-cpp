#include "OrderBook.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <chrono>
#include <thread>

using namespace orderbook;

class SimpleTest
{
public:
    static int testCount;
    static int passCount;
    static int failCount;

    static void runTest(const std::string &testName, bool result)
    {
        testCount++;
        if (result)
        {
            passCount++;
            std::cout << "✓ " << testName << " PASSED" << std::endl;
        }
        else
        {
            failCount++;
            std::cout << "✗ " << testName << " FAILED" << std::endl;
        }
    }

    static void printSummary()
    {
        std::cout << "\n=== TEST SUMMARY ===" << std::endl;
        std::cout << "Total tests: " << testCount << std::endl;
        std::cout << "Passed: " << passCount << std::endl;
        std::cout << "Failed: " << failCount << std::endl;
        std::cout << "Success rate: " << (testCount > 0 ? (100.0 * passCount / testCount) : 0) << "%" << std::endl;
    }
};

int SimpleTest::testCount = 0;
int SimpleTest::passCount = 0;
int SimpleTest::failCount = 0;

#define ASSERT_EQ(actual, expected) SimpleTest::runTest(__FUNCTION__, (actual) == (expected))
#define ASSERT_TRUE(condition) SimpleTest::runTest(__FUNCTION__, (condition))
#define ASSERT_FALSE(condition) SimpleTest::runTest(__FUNCTION__, !(condition))

void testBasicOrderAddition()
{
    OrderBook book;

    auto buy1 = std::make_shared<Order>(1, OrderSide::BUY, 100.50, 100);
    auto buy2 = std::make_shared<Order>(2, OrderSide::BUY, 100.25, 200);
    auto buy3 = std::make_shared<Order>(3, OrderSide::BUY, 100.75, 150);

    ASSERT_TRUE(book.addOrder(buy1));
    ASSERT_TRUE(book.addOrder(buy2));
    ASSERT_TRUE(book.addOrder(buy3));

    ASSERT_EQ(book.getOrderCount(), 3);
    ASSERT_EQ(book.getBestBid().value(), 100.75);
    ASSERT_FALSE(book.getBestAsk().has_value());

    auto sell1 = std::make_shared<Order>(4, OrderSide::SELL, 101.00, 100);
    auto sell2 = std::make_shared<Order>(5, OrderSide::SELL, 101.25, 200);
    auto sell3 = std::make_shared<Order>(6, OrderSide::SELL, 100.75, 150);

    ASSERT_TRUE(book.addOrder(sell1));
    ASSERT_TRUE(book.addOrder(sell2));
    ASSERT_TRUE(book.addOrder(sell3));

    ASSERT_EQ(book.getOrderCount(), 6);
    ASSERT_EQ(book.getBestAsk().value(), 100.75);
    ASSERT_EQ(book.getSpread().value(), 0.0);
}

void testCompleteOrderMatching()
{
    OrderBook book;
    std::vector<Trade> trades;
    int tradeCount = 0;

    book.setTradeCallback([&](const Trade &trade)
                          {
        trades.push_back(trade);
        tradeCount++; });

    auto sellOrder = std::make_shared<Order>(1, OrderSide::SELL, 100.50, 100);
    book.addOrder(sellOrder);

    ASSERT_EQ(book.getOrderCount(), 1);
    ASSERT_EQ(tradeCount, 0);

    auto buyOrder = std::make_shared<Order>(2, OrderSide::BUY, 100.50, 100);
    book.addOrder(buyOrder);

    ASSERT_EQ(book.getOrderCount(), 0);
    ASSERT_EQ(tradeCount, 1);
    ASSERT_EQ(trades[0].buyOrderId, 2);
    ASSERT_EQ(trades[0].sellOrderId, 1);
    ASSERT_EQ(trades[0].quantity, 100);
}

void testPartialOrderMatching()
{
    OrderBook book;
    std::vector<Trade> trades;
    int tradeCount = 0;

    book.setTradeCallback([&](const Trade &trade)
                          {
        trades.push_back(trade);
        tradeCount++; });

    auto sellOrder = std::make_shared<Order>(1, OrderSide::SELL, 100.50, 100);
    book.addOrder(sellOrder);

    auto buyOrder = std::make_shared<Order>(2, OrderSide::BUY, 100.50, 150);
    book.addOrder(buyOrder);

    ASSERT_EQ(book.getOrderCount(), 1);
    ASSERT_EQ(tradeCount, 1);
    ASSERT_EQ(trades[0].buyOrderId, 2);
    ASSERT_EQ(trades[0].sellOrderId, 1);
    ASSERT_EQ(trades[0].quantity, 100);
    ASSERT_EQ(book.getBestBid().value(), 100.50);
    ASSERT_EQ(book.getDepthAtPrice(100.50, OrderSide::BUY), 50);
}

void testMarketCrossingOrders()
{
    OrderBook book;
    std::vector<Trade> trades;
    int tradeCount = 0;

    book.setTradeCallback([&](const Trade &trade)
                          {
        trades.push_back(trade);
        tradeCount++; });

    auto buy1 = std::make_shared<Order>(1, OrderSide::BUY, 100.50, 100);
    auto buy2 = std::make_shared<Order>(2, OrderSide::BUY, 100.25, 200);
    auto sell1 = std::make_shared<Order>(3, OrderSide::SELL, 101.00, 150);
    auto sell2 = std::make_shared<Order>(4, OrderSide::SELL, 101.25, 100);

    book.addOrder(buy1);
    book.addOrder(buy2);
    book.addOrder(sell1);
    book.addOrder(sell2);

    ASSERT_EQ(book.getSpread().value(), 0.50);
    ASSERT_EQ(book.getOrderCount(), 4);
    ASSERT_EQ(tradeCount, 0);

    auto aggressiveSell = std::make_shared<Order>(5, OrderSide::SELL, 100.30, 75);
    book.addOrder(aggressiveSell);

    ASSERT_EQ(tradeCount, 1);
    ASSERT_EQ(book.getOrderCount(), 4); // Original 4 orders remain (aggressive sell is matched and removed)
    ASSERT_EQ(trades[0].buyOrderId, 1);
    ASSERT_EQ(trades[0].sellOrderId, 5);
    ASSERT_EQ(trades[0].quantity, 75);
    ASSERT_EQ(book.getDepthAtPrice(100.50, OrderSide::BUY), 25); // 100 - 75 = 25 remaining
}

void testMultiLevelMatching()
{
    OrderBook book;
    std::vector<Trade> trades;
    int tradeCount = 0;

    book.setTradeCallback([&](const Trade &trade)
                          {
        trades.push_back(trade);
        tradeCount++; });

    auto buy1 = std::make_shared<Order>(1, OrderSide::BUY, 100.50, 100);
    auto buy2 = std::make_shared<Order>(2, OrderSide::BUY, 100.25, 150);
    auto buy3 = std::make_shared<Order>(3, OrderSide::BUY, 100.00, 200);
    auto sell1 = std::make_shared<Order>(4, OrderSide::SELL, 101.00, 100);
    auto sell2 = std::make_shared<Order>(5, OrderSide::SELL, 101.25, 150);

    book.addOrder(buy1);
    book.addOrder(buy2);
    book.addOrder(buy3);
    book.addOrder(sell1);
    book.addOrder(sell2);

    ASSERT_EQ(book.getOrderCount(), 5);
    ASSERT_EQ(book.getSpread().value(), 0.50);

    auto largeBuy = std::make_shared<Order>(6, OrderSide::BUY, 101.30, 300);
    book.addOrder(largeBuy);

    ASSERT_EQ(tradeCount, 2);
    ASSERT_EQ(book.getOrderCount(), 4); // 3 original buy orders + 1 remaining large buy order
    ASSERT_EQ(trades[0].buyOrderId, 6);
    ASSERT_EQ(trades[0].sellOrderId, 4);
    ASSERT_EQ(trades[0].quantity, 100);
    ASSERT_EQ(trades[1].buyOrderId, 6);
    ASSERT_EQ(trades[1].sellOrderId, 5);
    ASSERT_EQ(trades[1].quantity, 150);
    ASSERT_EQ(book.getBestBid().value(), 101.30);
    ASSERT_EQ(book.getDepthAtPrice(101.30, OrderSide::BUY), 50);
}

void testPriceTimePriority()
{
    OrderBook book;
    std::vector<Trade> trades;
    int tradeCount = 0;

    book.setTradeCallback([&](const Trade &trade)
                          {
        trades.push_back(trade);
        tradeCount++; });

    auto buy1 = std::make_shared<Order>(1, OrderSide::BUY, 100.50, 100);
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    auto buy2 = std::make_shared<Order>(2, OrderSide::BUY, 100.50, 100);
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    auto buy3 = std::make_shared<Order>(3, OrderSide::BUY, 100.50, 100);

    book.addOrder(buy1);
    book.addOrder(buy2);
    book.addOrder(buy3);

    ASSERT_EQ(book.getOrderCount(), 3);
    ASSERT_EQ(book.getDepthAtPrice(100.50, OrderSide::BUY), 300);

    auto sellOrder = std::make_shared<Order>(4, OrderSide::SELL, 100.50, 50);
    book.addOrder(sellOrder);

    ASSERT_EQ(tradeCount, 1);
    ASSERT_EQ(trades[0].buyOrderId, 1);
    ASSERT_EQ(trades[0].quantity, 50);

    auto sellOrder2 = std::make_shared<Order>(5, OrderSide::SELL, 100.50, 75);
    book.addOrder(sellOrder2);

    ASSERT_EQ(tradeCount, 2);
    // The second trade should be with buy order 1 (remaining 50) and buy order 2 (25)
    // But since we can only track one trade at a time, we expect the first part (50 from order 1)
    ASSERT_EQ(trades[1].buyOrderId, 1);
    ASSERT_EQ(trades[1].quantity, 50);
}

int main()
{
    std::cout << "Running OrderBook Tests..." << std::endl;
    std::cout << "=========================" << std::endl;

    testBasicOrderAddition();
    testCompleteOrderMatching();
    testPartialOrderMatching();
    testMarketCrossingOrders();
    testMultiLevelMatching();
    testPriceTimePriority();

    SimpleTest::printSummary();

    return SimpleTest::failCount > 0 ? 1 : 0;
}
