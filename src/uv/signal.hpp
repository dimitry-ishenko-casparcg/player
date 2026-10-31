////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "handle.hpp"
#include "loop.hpp"

#include <concepts>
#include <functional>
#include <memory>
#include <span>
#include <vector>

#include <uv.h>

////////////////////////////////////////////////////////////////////////////////
namespace uv
{

class signal : public handle<uv_signal_t>
{
public:
    using signal_callback = std::function<void(int signal)>;

    signal(loop&, int num);
    void on_signal(signal_callback cb) { *cb_ = std::move(cb); }

private:
    std::unique_ptr<signal_callback> cb_ = std::make_unique<signal_callback>();
};

////////////////////////////////////////////////////////////////////////////////
class signals
{
public:
    using signal_callback = uv::signal::signal_callback;

    template<std::convertible_to<int>... Vs>
    signals(loop& loop, Vs... nums)
    {
        signals_.reserve(sizeof...(Vs));
        (signals_.emplace_back(loop, nums), ...);
    }

    signals(loop& loop, std::span<const int> nums)
    {
        signals_.reserve(nums.size());
        for (int num : nums) signals_.emplace_back(loop, num);
    }

    void on_signal(const signal_callback& cb)
    {
        for (auto& s : signals_) s.on_signal(cb);
    }

private:
    std::vector<signal> signals_;
};

}
