#pragma once

#define access_ptr(obj, type, offset) (reinterpret_cast<type*>(((uintptr_t) obj) + offset))
#define access(obj, type, offset) (*access_ptr(obj, type, offset))