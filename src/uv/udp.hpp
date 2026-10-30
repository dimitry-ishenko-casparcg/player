////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "address.hpp"
#include "handle.hpp"
#include "loop.hpp"

#include <array>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include <uv.h>

namespace uv
{

class udp : public handle<uv_udp_t>
{
public:
    using recv_callback = std::function<void(std::string_view, const address&)>;

    explicit udp(loop&);

    void bind(const address&);
    void connect(const address&);
    void disconnect();

    void on_recv(recv_callback);

    void send(std::string data, const address& to) { send(std::move(data), to.get()); }
    void send(std::string data) { send(std::move(data), nullptr); }

private:
    void send(std::string, const sockaddr*);

    struct state
    {
        recv_callback cb;
        std::array<char, 64 * 1024> buf; // 64K
    };
    std::unique_ptr<state> state_ = std::make_unique<state>();
};

}
