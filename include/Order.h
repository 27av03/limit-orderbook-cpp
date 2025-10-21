#pragma once

#include <cstdint>
#include <string>
#include <chrono>

namespace orderbook
{

    enum class OrderSide
    {
        BUY,
        SELL
    };

    struct Order
    {
        std::uint64_t orderId;
        OrderSide side;
        double price;
        std::uint64_t quantity;
        std::uint64_t timestamp;

        Order(std::uint64_t id, OrderSide s, double p, std::uint64_t qty)
            : orderId(id), side(s), price(p), quantity(qty),
              timestamp(std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::high_resolution_clock::now().time_since_epoch())
                            .count()) {}

        // Copy constructor
        Order(const Order &other) = default;

        // Move constructor
        Order(Order &&other) noexcept = default;

        // Assignment operators
        Order &operator=(const Order &other) = default;
        Order &operator=(Order &&other) noexcept = default;

        // Destructor
        ~Order() = default;

        // Comparison operators for price-time priority
        bool operator<(const Order &other) const
        {
            if (side == OrderSide::BUY)
            {
                // For bids: higher price first, then earlier timestamp
                if (price != other.price)
                {
                    return price > other.price;
                }
                return timestamp < other.timestamp;
            }
            else
            {
                // For asks: lower price first, then earlier timestamp
                if (price != other.price)
                {
                    return price < other.price;
                }
                return timestamp < other.timestamp;
            }
        }

        bool operator>(const Order &other) const
        {
            return other < *this;
        }

        bool operator==(const Order &other) const
        {
            return orderId == other.orderId;
        }

        bool operator!=(const Order &other) const
        {
            return !(*this == other);
        }
    };

} // namespace orderbook

