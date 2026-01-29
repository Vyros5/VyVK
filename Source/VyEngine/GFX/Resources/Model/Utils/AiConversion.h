#pragma once

#include <VyLib/VyLib.h>
#include <assimp/types.h>

namespace Vy::Utils
{
    static Mat4 AiToGlm(const aiMatrix4x4& m)
    {
        Mat4 r;
        {
            r[0][0] = m.a1;  r[1][0] = m.a2;  r[2][0] = m.a3;  r[3][0] = m.a4;
            r[0][1] = m.b1;  r[1][1] = m.b2;  r[2][1] = m.b3;  r[3][1] = m.b4;
            r[0][2] = m.c1;  r[1][2] = m.c2;  r[2][2] = m.c3;  r[3][2] = m.c4;
            r[0][3] = m.d1;  r[1][3] = m.d2;  r[2][3] = m.d3;  r[3][3] = m.d4;
        }

        return r;
    }


    static Vec2 AiToGlm(const aiVector2D& v) 
    {
        Vec2 r;
        {
            r.x = v.x;
            r.y = v.y;
        }

        return r;
    }


    static Vec3 AiToGlm(const aiVector3D& v) 
    {
        Vec3 r;
        {
            r.x = v.x;
            r.y = v.y;
            r.z = v.z;
        }

        return r;
    }


    static Vec3 AiToGlm(const aiColor4D& c) 
    {
        Vec3 r;
        {
            r.x = c.r;
            r.y = c.g;
            r.z = c.b;
        }

        return r;
    }


    // static Vec4 AiToGlm(const aiColor4D& c) 
    // {
    //     Vec4 r;
    //     {
    //         r.x = c.r;
    //         r.y = c.g;
    //         r.z = c.b;
    //         r.w = c.a;
    //     }

    //     return r;
    // }
}