////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <system_error>

#include <uv.h>

////////////////////////////////////////////////////////////////////////////////
namespace uv
{

class error_category : public std::error_category
{
public:
    const char* name() const noexcept override { return "uv"; }
    std::string message(int ec) const override
    {
        char data[128];
        uv_strerror_r(ec, data, sizeof(data));
        return data;
    }
};

inline const std::error_category& category() noexcept
{
    static const error_category instance{};
    return instance;
}

inline std::error_code make_error_code(int rc) noexcept { return { rc, category() }; }

////////////////////////////////////////////////////////////////////////////////
class error : public std::system_error
{
public:
    error(int rc, const char* what) : std::system_error(rc, category(), what) { }
    error(int rc, const std::string& what) : std::system_error(rc, category(), what) { }
};

}
