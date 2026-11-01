////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "error.hpp"
#include "udp.hpp"

#include <memory>

////////////////////////////////////////////////////////////////////////////////
namespace uv
{

udp::udp(loop& loop)
{
    init([&](uv_udp_t* h){ return uv_udp_init(loop.get(), h); }, "uv_udp_init");
    get()->data = state_.get();
}

void udp::bind(const address& addr)
{
    if (auto rc = uv_udp_bind(get(), addr.get(), 0); rc < 0) throw error{rc, "uv_udp_bind"};
}

address udp::local() const
{
    address addr;
    int len = sizeof(addr.addr_);

    auto rc = uv_udp_getsockname(get(), addr.get(), &len);
    if (rc < 0) throw error{rc, "uv_udp_getsockname"};

    return addr;
}

void udp::connect(const address& peer)
{
    if (auto rc = uv_udp_connect(get(), peer.get()); rc < 0) throw error{rc, "uv_udp_connect"};
}

void udp::disconnect()
{
    if (auto rc = uv_udp_connect(get(), nullptr); rc < 0) throw error{rc, "uv_udp_connect"};
}

void udp::on_recv(recv_callback cb)
{
    state_->cb = std::move(cb);

    auto alloc = [](uv_handle_t* h, size_t, uv_buf_t* b)
    {
        auto& s = *static_cast<state*>(h->data);
        *b = uv_buf_init(s.buf.data(), s.buf.size());
    };
    auto recv = [](uv_udp_t* h, ssize_t n, const uv_buf_t* b, const sockaddr* from, unsigned)
    {
        auto& s = *static_cast<state*>(h->data);
        if (n < 0 || !from) return;
        if (s.cb) s.cb({ b->base, static_cast<std::size_t>(n) }, address{from});
    };

    auto rc = uv_udp_recv_start(get(), alloc, recv);
    if (rc < 0 && rc != UV_EALREADY) throw error{rc, "uv_udp_recv_start"};
}

void udp::send(std::string data, const sockaddr* to)
{
    struct request : uv_udp_send_t { std::string payload; };

    auto req = std::make_unique<request>();
    req->payload = std::move(data);

    auto buf = uv_buf_init(req->payload.data(), req->payload.size());
    auto cb = [](uv_udp_send_t* s, int){ delete static_cast<request*>(s); };

    if (auto rc = uv_udp_send(req.get(), get(), &buf, 1, to, cb); rc < 0) throw error{rc, "uv_udp_send"};
    req.release(); // NOLINT(bugprone-unused-return-value) - freed inside cb()
}

}
