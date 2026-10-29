////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "handle.hpp"
#include "loop.hpp"

#include <functional>
#include <memory>

#include <uv.h>

////////////////////////////////////////////////////////////////////////////////
namespace uv
{

class signal : public handle<uv_signal_t>
{
public:
    using signal_callback = std::function<void(int signal)>;

    signal(loop&, int signal);
    void on_signal(signal_callback cb) { *cb_ = std::move(cb); }

private:
    std::unique_ptr<signal_callback> cb_ = std::make_unique<signal_callback>();
};

}
