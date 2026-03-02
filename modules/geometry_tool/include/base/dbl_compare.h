/**
 * @file dbl_compare.h
 * @author HUANG He (he.hu4ng@outlook.com)
 * @brief define float number compare functions
 * @version 0.1
 * @date 2025-09-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <cmath>
#include <utility>
#include <limits>
#include <algorithm>

namespace tools_2D 
{
    template<std::floating_point T>
    inline auto dbl_delta(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
    ->std::pair<T, T>
    {
        return { a - b, std::max(std::fabs(a), std::fabs(b)) * epsilon };
    }

    template<std::floating_point T>
    inline auto dbl_inf(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
    {
        auto [diff, rel_eps] = dbl_delta(a, b, epsilon);
        return -diff > rel_eps;
    }

    template<std::floating_point T>
    inline auto dbl_sup(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
    {
        auto [diff, rel_eps] = dbl_delta(a, b, epsilon);
        return diff > rel_eps;
    }

    template<std::floating_point T>
    inline auto dbl_eq(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
    {
        return !dbl_inf(a, b, epsilon) && !dbl_sup(a, b, epsilon);
    }

    template<std::floating_point T>
    inline auto dbl_neq(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
    {
        return !dbl_eq(a, b, epsilon);
    }

    template<std::floating_point T>
    inline auto dbl_infeq(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
    {
        return dbl_inf(a, b, epsilon) || dbl_eq(a, b, epsilon);
    }

    template<std::floating_point T>
    inline auto dbl_supeq(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
    {
        return dbl_sup(a, b, epsilon) || dbl_eq(a, b, epsilon);
    }
} // namespace tools_2D
