#ifndef MANGOS_JAM_TYPES_H
#define MANGOS_JAM_TYPES_H

#include "Common.h"
#include "Util/ByteBuffer.h"

#include <G3D/Vector3.h>

typedef uint8  u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;
typedef int8   i8;
typedef int16  i16;
typedef int32  i32;
typedef int64  i64;

typedef G3D::Vector3 C3Vector;

inline ByteBuffer& operator<<(ByteBuffer& b, C3Vector const& v)
{
    return b << v.x << v.y << v.z;
}

inline ByteBuffer& operator>>(ByteBuffer& b, C3Vector& v)
{
    return b >> v.x >> v.y >> v.z;
}

#endif
