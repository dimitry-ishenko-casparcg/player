////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "signal.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace uv
{

signal::signal(loop& loop, int num)
{
    init([&](uv_signal_t* h){ return uv_signal_init(loop.get(), h); }, "uv_signal_init");

    get()->data = cb_.get();
    auto fn = [](uv_signal_t* h, int num)
    {
        auto& cb = *static_cast<signal_callback*>(h->data);
        if (cb) cb(num);
    };
    if (int rc = uv_signal_start(get(), fn, num); rc < 0) throw error{rc, "uv_signal_start"};
}

}
