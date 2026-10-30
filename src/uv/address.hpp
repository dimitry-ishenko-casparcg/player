////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "error.hpp"

#include <cstring> // std::memcpy
#include <string>

#include <uv.h>

////////////////////////////////////////////////////////////////////////////////
namespace uv
{

class address
{
    sockaddr_in addr_{ };

public:
    address() = default;

    address(const std::string& ip, int port)
    {
        auto rc = uv_ip4_addr(ip.data(), port, &addr_);
        if (rc < 0) throw error{rc, "uv_ip4_addr"};
    }

    std::string ip() const
    {
        char data[16];
        uv_ip4_name(&addr_, data, sizeof(data));
        return data;
    }

    int port() const noexcept { return ntohs(addr_.sin_port); }

private:
    friend class udp;

    explicit address(const sockaddr* addr) noexcept
    {
        if (addr->sa_family == AF_INET) std::memcpy(&addr_, addr, sizeof(addr_));
    }

    auto get() const noexcept { return reinterpret_cast<const sockaddr*>(&addr_); }
    auto get() noexcept { return reinterpret_cast<sockaddr*>(&addr_); }
};

}
