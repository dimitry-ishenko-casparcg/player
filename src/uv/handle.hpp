////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "error.hpp"

#include <memory>
#include <uv.h>

////////////////////////////////////////////////////////////////////////////////
namespace uv
{

template<typename V>
struct handle_delete
{
    void operator()(V* p) const noexcept
    {
        auto h = reinterpret_cast<uv_handle_t*>(p);
        // check if init() ever ran (eg, in failed uv_spawn())
        if (uv_handle_get_type(h) == UV_UNKNOWN_HANDLE) delete p;
        else uv_close(h, [](uv_handle_t* h){ delete reinterpret_cast<V*>(h); });
    }
};

template<typename V>
class handle
{
public:
    V* get() noexcept { return p_.get(); }
    const V* get() const noexcept { return p_.get(); }

protected:
    void init(auto&& fn, const char* name)
    {
        if (auto rc = fn(p_.get()); rc < 0) throw error{rc, name};
    }

private:
    std::unique_ptr<V, handle_delete<V>> p_{ new V{} };
};

}
