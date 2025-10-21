#pragma once

#include "Order.h"
#include <map>
#include <unordered_map>
#include <memory>
#include <vector>
#include <optional>
#include <functional>

namespace orderbook
{

    struct Trade
    {
        std::uint64_t buyOrderId;
        std::uint64_t sellOrderId;
        double price;
        std::uint64_t quantity;
        std::uint64_t timestamp;

        Trade(std::uint64_t buyId, std::uint64_t sellId, double p, std::uint64_t qty)
            : buyOrderId(buyId), sellOrderId(sellId), price(p), quantity(qty),
              timestamp(std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::high_resolution_clock::now().time_since_epoch())
                            .count()) {}
    };

    class OrderBook
    {
    public:
        using OrderPtr = std::shared_ptr<Order>;
        using OrderMap = std::map<std::uint64_t, OrderPtr>;
        using PriceLevelMap = std::map<double, std::vector<OrderPtr>>;
        using TradeCallback = std::function<void(const Trade &)>;

        OrderBook() = default;
        ~OrderBook() = default;

        // Disable copy constructor and assignment operator
        OrderBook(const OrderBook &) = delete;
        OrderBook &operator=(const OrderBook &) = delete;

        // Move constructor and assignment operator
        OrderBook(OrderBook &&other) noexcept;
        OrderBook &operator=(OrderBook &&other) noexcept;

        /**
         * Add an order to the order book
         * @param order The order to add
         * @return true if order was added successfully, false otherwise
         */
        bool addOrder(OrderPtr order);

        /**
         * Cancel an existing order
         * @param orderId The ID of the order to cancel
         * @return true if order was found and cancelled, false otherwise
         */
        bool cancelOrder(std::uint64_t orderId);

        /**
         * Modify an existing order (cancel and re-add with new parameters)
         * @param orderId The ID of the order to modify
         * @param newPrice The new price for the order
         * @param newQuantity The new quantity for the order
         * @return true if order was modified successfully, false otherwise
         */
        bool modifyOrder(std::uint64_t orderId, double newPrice, std::uint64_t newQuantity);

        /**
         * Get the best bid price
         * @return The highest bid price, or nullopt if no bids exist
         */
        std::optional<double> getBestBid() const;

        /**
         * Get the best ask price
         * @return The lowest ask price, or nullopt if no asks exist
         */
        std::optional<double> getBestAsk() const;

        /**
         * Get the spread between best bid and ask
         * @return The spread, or nullopt if either side is empty
         */
        std::optional<double> getSpread() const;

        /**
         * Get market depth for a given price level
         * @param price The price level to query
         * @param side The side (buy/sell) to query
         * @return The total quantity at the price level
         */
        std::uint64_t getDepthAtPrice(double price, OrderSide side) const;

        /**
         * Get total number of orders in the book
         * @return Total order count
         */
        std::size_t getOrderCount() const;

        /**
         * Set callback function for trade notifications
         * @param callback Function to call when a trade occurs
         */
        void setTradeCallback(TradeCallback callback);

        /**
         * Clear all orders from the book
         */
        void clear();

    private:
        // Data structures
        OrderMap orders_;    // All orders by ID for O(1) lookup
        PriceLevelMap bids_; // Buy orders by price level
        PriceLevelMap asks_; // Sell orders by price level

        // Trade callback
        TradeCallback tradeCallback_;

        // Helper methods
        void matchOrders(OrderPtr newOrder);
        void removeOrderFromPriceLevel(OrderPtr order);
        void addOrderToPriceLevel(OrderPtr order);
        PriceLevelMap &getPriceLevelMap(OrderSide side);
        const PriceLevelMap &getPriceLevelMap(OrderSide side) const;
        void executeTrade(OrderPtr buyOrder, OrderPtr sellOrder, std::uint64_t quantity);
    };

} // namespace orderbook

