#pragma once

/*
Capture into lambda by moving:

auto object = get_obj();
auto lambda = [mv(object)] {};

*/
#define mv(name) name{std::move(name)}
