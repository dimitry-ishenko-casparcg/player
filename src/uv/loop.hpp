////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "error.hpp"

#include <exception> // std::terminate
#include <memory>

#include <uv.h>

////////////////////////////////////////////////////////////////////////////////
namespace uv
{

struct loop_delete
{
    void operator()(uv_loop_t* p) const noexcept
    {
        while (uv_run(p, UV_RUN_ONCE)) { } // drain pending close callbacks
        if (uv_loop_close(p)) std::terminate();
        delete p;
    }
};

class loop
{
public:
    uv_loop_t* get() noexcept { return p_.get(); }

    bool run(uv_run_mode mode = UV_RUN_DEFAULT) { return uv_run(get(), mode); }
    void stop() noexcept { uv_stop(get()); }

private:
    static uv_loop_t* create()
    {
        // create with standard deleter in case uv_loop_init() fails
        auto p = std::make_unique<uv_loop_t>();
        if (auto rc = uv_loop_init(p.get()); rc < 0) throw error{rc, "uv_loop_init"};
        return p.release();
    }

    std::unique_ptr<uv_loop_t, loop_delete> p_{ create() };
};

}
