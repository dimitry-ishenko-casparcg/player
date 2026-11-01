////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "handle.hpp"
#include "loop.hpp"
#include "pipe.hpp"
#include "timer.hpp"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <uv.h>

////////////////////////////////////////////////////////////////////////////////
namespace uv
{

class process : public handle<uv_process_t>
{
public:
    using exit_callback = std::function<void(int status, int signal)>;

    process() = default;
    process(loop&, std::vector<std::string> args, std::vector<std::string> env = {});
    ~process() { teardown(); }

    process(process&&) noexcept = default;
    process& operator=(process&&) noexcept;

    void on_exit(exit_callback cb) { *exit_cb_ = std::move(cb); }

    ////////////////////
    void write(std::string data) { stdin_.write(std::move(data)); }

    int pid() const noexcept { return get()->pid; }
    int signal(int n) noexcept { return uv_process_kill(get(), n); }

    void stop(msec timeout = 1000ms);

private:
    std::unique_ptr<exit_callback> exit_cb_ = std::make_unique<exit_callback>();

    timer timer_;
    void teardown() noexcept;

    pipe stdin_, stdout_, stderr_;
};

}
