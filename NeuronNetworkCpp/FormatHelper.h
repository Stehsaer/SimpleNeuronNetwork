#pragma once

#ifndef _FORMAT_HELPER_H_
#define _FORMAT_HELPER_H_

#include <string>
#include <format>

inline std::string GenID(std::string title, void* ptr)
{
	return std::format("{}##{:#x}", title, reinterpret_cast<uintptr_t>(ptr));
}

#endif