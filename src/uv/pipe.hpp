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

#include <memory>
#include <string>
#include <utility>

#include <uv.h>

////////////////////////////////////////////////////////////////////////////////
namespace uv
{

class pipe : public handle<uv_pipe_t>
{
public:
    pipe() = default;
    explicit pipe(loop& loop)
    {
        init([&](auto* h){ return uv_pipe_init(loop.get(), h, 0); }, "uv_pipe_init");
    }

    void write(std::string data)
    {
        struct request : uv_write_t { std::string payload; };

        auto req = std::make_unique<request>();
        req->payload = std::move(data);

        auto buf = uv_buf_init(req->payload.data(), req->payload.size());
        auto cb = [](uv_write_t* w, int){ delete static_cast<request*>(w); };

        if (auto rc = uv_write(req.get(), stream(), &buf, 1, cb); rc < 0) throw error{ rc, "uv_write" };
        req.release(); // NOLINT(bugprone-unused-return-value) - freed inside cb()
    }

private:
    uv_stream_t* stream() noexcept { return reinterpret_cast<uv_stream_t*>(get()); }
};

}
