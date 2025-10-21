#include "OrderBook.h"
#include <algorithm>
#include <stdexcept>

namespace orderbook
{

    OrderBook::OrderBook(OrderBook &&other) noexcept
        : orders_(std::move(other.orders_)),
          bids_(std::move(other.bids_)),
          asks_(std::move(other.asks_)),
          tradeCallback_(std::move(other.tradeCallback_))
    {
    }

    OrderBook &OrderBook::operator=(OrderBook &&other) noexcept
    {
        if (this != &other)
        {
            orders_ = std::move(other.orders_);
            bids_ = std::move(other.bids_);
            asks_ = std::move(other.asks_);
            tradeCallback_ = std::move(other.tradeCallback_);
        }
        return *this;
    }

    bool OrderBook::addOrder(OrderPtr order)
    {
        if (!order || order->quantity == 0)
        {
            return false;
        }

        // Check if order already exists
        if (orders_.find(order->orderId) != orders_.end())
        {
            return false;
        }

        // Add order to the book
        orders_[order->orderId] = order;
        addOrderToPriceLevel(order);

        // Attempt to match orders
        matchOrders(order);

        // If the order was fully matched, it should already be removed from orders_ and price level
        // by the matchOrders function, so we don't need to do anything else here

        return true;
    }

    bool OrderBook::cancelOrder(std::uint64_t orderId)
    {
        auto it = orders_.find(orderId);
        if (it == orders_.end())
        {
            return false;
        }

        OrderPtr order = it->second;
        removeOrderFromPriceLevel(order);
        orders_.erase(it);

        return true;
    }

    bool OrderBook::modifyOrder(std::uint64_t orderId, double newPrice, std::uint64_t newQuantity)
    {
        auto it = orders_.find(orderId);
        if (it == orders_.end())
        {
            return false;
        }

        OrderPtr order = it->second;

        // Remove from price level
        removeOrderFromPriceLevel(order);

        // Update order parameters
        order->price = newPrice;
        order->quantity = newQuantity;
        order->timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                               std::chrono::high_resolution_clock::now().time_since_epoch())
                               .count();

        // Re-add to price level
        addOrderToPriceLevel(order);

        // Attempt to match orders
        matchOrders(order);

        return true;
    }

    std::optional<double> OrderBook::getBestBid() const
    {
        if (bids_.empty())
        {
            return std::nullopt;
        }
        return bids_.rbegin()->first; // Highest price (rbegin = reverse begin)
    }

    std::optional<double> OrderBook::getBestAsk() const
    {
        if (asks_.empty())
        {
            return std::nullopt;
        }
        return asks_.begin()->first; // Lowest price
    }

    std::optional<double> OrderBook::getSpread() const
    {
        auto bestBid = getBestBid();
        auto bestAsk = getBestAsk();

        if (!bestBid || !bestAsk)
        {
            return std::nullopt;
        }

        return *bestAsk - *bestBid;
    }

    std::uint64_t OrderBook::getDepthAtPrice(double price, OrderSide side) const
    {
        const PriceLevelMap &priceMap = getPriceLevelMap(side);
        auto it = priceMap.find(price);

        if (it == priceMap.end())
        {
            return 0;
        }

        std::uint64_t totalQuantity = 0;
        for (const auto &order : it->second)
        {
            totalQuantity += order->quantity;
        }

        return totalQuantity;
    }

    std::size_t OrderBook::getOrderCount() const
    {
        return orders_.size();
    }

    void OrderBook::setTradeCallback(TradeCallback callback)
    {
        tradeCallback_ = std::move(callback);
    }

    void OrderBook::clear()
    {
        orders_.clear();
        bids_.clear();
        asks_.clear();
    }

    void OrderBook::matchOrders(OrderPtr newOrder)
    {
        PriceLevelMap &oppositeSide = (newOrder->side == OrderSide::BUY) ? asks_ : bids_;

        while (newOrder->quantity > 0 && !oppositeSide.empty())
        {
            // Get the best price level on the opposite side
            auto priceLevelIt = (newOrder->side == OrderSide::BUY) ? oppositeSide.begin() : --oppositeSide.end();

            if (priceLevelIt == oppositeSide.end())
            {
                break;
            }

            double oppositePrice = priceLevelIt->first;

            // Check if prices can match
            bool canMatch = (newOrder->side == OrderSide::BUY) ? (newOrder->price >= oppositePrice) : (newOrder->price <= oppositePrice);

            if (!canMatch)
            {
                break;
            }

            // Get orders at this price level
            auto &ordersAtPrice = priceLevelIt->second;

            while (!ordersAtPrice.empty() && newOrder->quantity > 0)
            {
                OrderPtr oppositeOrder = ordersAtPrice.front();

                std::uint64_t tradeQuantity = std::min(newOrder->quantity, oppositeOrder->quantity);

                // Execute the trade - ensure correct order of buy/sell
                if (newOrder->side == OrderSide::BUY)
                {
                    executeTrade(newOrder, oppositeOrder, tradeQuantity);
                }
                else
                {
                    executeTrade(oppositeOrder, newOrder, tradeQuantity);
                }

                // Update quantities
                newOrder->quantity -= tradeQuantity;
                oppositeOrder->quantity -= tradeQuantity;

                // Remove fully filled opposite order
                if (oppositeOrder->quantity == 0)
                {
                    orders_.erase(oppositeOrder->orderId);
                    ordersAtPrice.erase(ordersAtPrice.begin());
                }
            }

            // Remove empty price level
            if (ordersAtPrice.empty())
            {
                oppositeSide.erase(priceLevelIt);
            }
        }

        // Remove fully filled new order
        if (newOrder->quantity == 0)
        {
            orders_.erase(newOrder->orderId);
            removeOrderFromPriceLevel(newOrder);
        }
    }

    void OrderBook::removeOrderFromPriceLevel(OrderPtr order)
    {
        PriceLevelMap &priceMap = getPriceLevelMap(order->side);
        auto priceLevelIt = priceMap.find(order->price);

        if (priceLevelIt != priceMap.end())
        {
            auto &ordersAtPrice = priceLevelIt->second;
            auto orderIt = std::find(ordersAtPrice.begin(), ordersAtPrice.end(), order);

            if (orderIt != ordersAtPrice.end())
            {
                ordersAtPrice.erase(orderIt);

                // Remove empty price level
                if (ordersAtPrice.empty())
                {
                    priceMap.erase(priceLevelIt);
                }
            }
        }
    }

    void OrderBook::addOrderToPriceLevel(OrderPtr order)
    {
        PriceLevelMap &priceMap = getPriceLevelMap(order->side);
        priceMap[order->price].push_back(order);

        // Sort orders at this price level by timestamp (FIFO)
        auto &ordersAtPrice = priceMap[order->price];
        std::sort(ordersAtPrice.begin(), ordersAtPrice.end(),
                  [](const OrderPtr &a, const OrderPtr &b)
                  {
                      return a->timestamp < b->timestamp;
                  });
    }

    OrderBook::PriceLevelMap &OrderBook::getPriceLevelMap(OrderSide side)
    {
        return (side == OrderSide::BUY) ? bids_ : asks_;
    }

    const OrderBook::PriceLevelMap &OrderBook::getPriceLevelMap(OrderSide side) const
    {
        return (side == OrderSide::BUY) ? bids_ : asks_;
    }

    void OrderBook::executeTrade(OrderPtr buyOrder, OrderPtr sellOrder, std::uint64_t quantity)
    {
        // Ensure buyOrder is actually a buy order and sellOrder is a sell order
        if (buyOrder->side != OrderSide::BUY || sellOrder->side != OrderSide::SELL)
        {
            throw std::runtime_error("Invalid trade execution: order sides don't match");
        }

        // Create trade record
        Trade trade(buyOrder->orderId, sellOrder->orderId,
                    (buyOrder->price + sellOrder->price) / 2.0, quantity);

        // Call trade callback if set
        if (tradeCallback_)
        {
            tradeCallback_(trade);
        }
    }

} // namespace orderbook
