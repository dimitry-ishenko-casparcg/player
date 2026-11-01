////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "error.hpp"
#include "handle.hpp"
#include "loop.hpp"

#include <chrono>
#include <functional>
#include <memory>

#include <uv.h>

////////////////////////////////////////////////////////////////////////////////
namespace uv
{

using msec = std::chrono::milliseconds;
using namespace std::chrono_literals;

////////////////////////////////////////////////////////////////////////////////
class timer : public handle<uv_timer_t>
{
public:
    using timer_callback = std::function<void()>;

    timer() = default;
    explicit timer(loop& loop)
    {
        init([&](auto* h){ return uv_timer_init(loop.get(), h); }, "uv_timer_init");
        get()->data = cb_.get();
    }

    void start(msec timeout, timer_callback cb)
    {
        *cb_ = std::move(cb);

        auto fn = [](uv_timer_t* h)
        {
            auto& cb = *static_cast<timer_callback*>(h->data);
            if (cb) cb();
        };
        auto rc = uv_timer_start(get(), fn, timeout.count(), 0);
        if (rc < 0) throw error{rc, "uv_timer_start"};
    }

    void stop() noexcept { uv_timer_stop(get()); }

private:
    std::unique_ptr<timer_callback> cb_ = std::make_unique<timer_callback>();
};

}
