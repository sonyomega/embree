// ======================================================================== //
// Copyright 2009-2012 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#ifndef __EMBREE_LINEAR_SPACE3_H__
#define __EMBREE_LINEAR_SPACE3_H__

#include "vec3.h"
#include "quaternion.h"

namespace embree
{
  ////////////////////////////////////////////////////////////////////////////////
  /// 3D Linear Transform (3x3 Matrix)
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> struct LinearSpace3
  {
    typedef Vec3<T> Vector;

    /*! default matrix constructor */
    __forceinline LinearSpace3           ( ) {}
    __forceinline LinearSpace3           ( const LinearSpace3& other ) { vx = other.vx; vy = other.vy; vz = other.vz; }
    __forceinline LinearSpace3& operator=( const LinearSpace3& other ) { vx = other.vx; vy = other.vy; vz = other.vz; return *this; }

    template<typename L1> __forceinline explicit LinearSpace3( const LinearSpace3<L1>& s ) : vx(s.vx), vy(s.vy), vz(s.vz) {}

    /*! matrix construction from column vectors */
    __forceinline LinearSpace3(const Vec3<T>& vx, const Vec3<T>& vy, const Vec3<T>& vz)
      : vx(vx), vy(vy), vz(vz) {}

    /*! construction from quaternion */
    __forceinline LinearSpace3( const QuaternionT<T>& q )
      : vx((q.r*q.r + q.i*q.i - q.j*q.j - q.k*q.k), 2.0f*(q.i*q.j + q.r*q.k), 2.0f*(q.i*q.k - q.r*q.j))
      , vy(2.0f*(q.i*q.j - q.r*q.k), (q.r*q.r - q.i*q.i + q.j*q.j - q.k*q.k), 2.0f*(q.j*q.k + q.r*q.i))
      , vz(2.0f*(q.i*q.k + q.r*q.j), 2.0f*(q.j*q.k - q.r*q.i), (q.r*q.r - q.i*q.i - q.j*q.j + q.k*q.k)) {}

    /*! matrix construction from row mayor data */
    __forceinline LinearSpace3(const T& m00, const T& m01, const T& m02,
                               const T& m10, const T& m11, const T& m12,
                               const T& m20, const T& m21, const T& m22)
      : vx(m00,m10,m20), vy(m01,m11,m21), vz(m02,m12,m22) {}

    /*! compute the determinant of the matrix */
    __forceinline const T det() const { return dot(vx,cross(vy,vz)); }

    /*! compute adjoint matrix */
    __forceinline const LinearSpace3 adjoint() const { return LinearSpace3(cross(vy,vz),cross(vz,vx),cross(vx,vy)).transposed(); }

    /*! compute inverse matrix */
    __forceinline const LinearSpace3 inverse() const { return rcp(det())*adjoint(); }

    /*! compute transposed matrix */
    __forceinline const LinearSpace3 transposed() const { return LinearSpace3<T>(vx.x,vx.y,vx.z,vy.x,vy.y,vy.z,vz.x,vz.y,vz.z); }

    /*! returns first row of matrix */
    __forceinline const Vec3<T> row0() const { return Vec3<T>(vx.x,vy.x,vz.x); }

    /*! returns second row of matrix */
    __forceinline const Vec3<T> row1() const { return Vec3<T>(vx.y,vy.y,vz.y); }

    /*! returns third row of matrix */
    __forceinline const Vec3<T> row2() const { return Vec3<T>(vx.z,vy.z,vz.z); }

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline LinearSpace3( ZeroTy ) : vx(zero), vy(zero), vz(zero) {}
    __forceinline LinearSpace3( OneTy ) : vx(one, zero, zero), vy(zero, one, zero), vz(zero, zero, one) {}

    /*! return matrix for scaling */
    static __forceinline LinearSpace3 scale(const Vec3<T>& s) {
      return LinearSpace3(s.x,   0,   0,
                          0  , s.y,   0,
                          0  ,   0, s.z);
    }

    /*! return matrix for rotation around arbitrary axis */
    static __forceinline LinearSpace3 rotate(const Vec3<T>& _u, const T& r) {
      Vec3<T> u = normalize(_u);
      T s = sin(r), c = cos(r);
      return LinearSpace3(u.x*u.x+(1-u.x*u.x)*c,  u.x*u.y*(1-c)-u.z*s,    u.x*u.z*(1-c)+u.y*s,
                          u.x*u.y*(1-c)+u.z*s,    u.y*u.y+(1-u.y*u.y)*c,  u.y*u.z*(1-c)-u.x*s,
                          u.x*u.z*(1-c)-u.y*s,    u.y*u.z*(1-c)+u.x*s,    u.z*u.z+(1-u.z*u.z)*c);
    }

  public:

    /*! the column vectors of the matrix */
    Vec3<T> vx,vy,vz;
  };

  ////////////////////////////////////////////////////////////////////////////////
  // Unary Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline LinearSpace3<T> operator -( const LinearSpace3<T>& a ) { return LinearSpace3<T>(-a.vx,-a.vy,-a.vz); }
  template<typename T> __forceinline LinearSpace3<T> operator +( const LinearSpace3<T>& a ) { return LinearSpace3<T>(+a.vx,+a.vy,+a.vz); }
  template<typename T> __forceinline LinearSpace3<T> rcp       ( const LinearSpace3<T>& a ) { return a.inverse(); }

  /* constructs a coordinate frame form a normal */
  template<typename T> __forceinline LinearSpace3<T> frame(const Vec3<T>& N) {
    Vec3<T> dx0 = cross(Vec3<T>(T(one),T(zero),T(zero)),N);
    Vec3<T> dx1 = cross(Vec3<T>(T(zero),T(one),T(zero)),N);
    Vec3<T> dx = normalize(select(dot(dx0,dx0) > dot(dx1,dx1),dx0,dx1));
    Vec3<T> dy = normalize(cross(N,dx));
    return LinearSpace3<T>(dx,dy,N);
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline LinearSpace3<T> operator +( const LinearSpace3<T>& a, const LinearSpace3<T>& b ) { return LinearSpace3<T>(a.vx+b.vx,a.vy+b.vy,a.vz+b.vz); }
  template<typename T> __forceinline LinearSpace3<T> operator -( const LinearSpace3<T>& a, const LinearSpace3<T>& b ) { return LinearSpace3<T>(a.vx-b.vx,a.vy-b.vy,a.vz-b.vz); }

  template<typename T> __forceinline LinearSpace3<T> operator*(const T              & a, const LinearSpace3<T>& b) { return LinearSpace3<T>(a*b.vx, a*b.vy, a*b.vz); }
  template<typename T> __forceinline Vec3<T>         operator*(const LinearSpace3<T>& a, const Vec3<T>        & b) { return b.x*a.vx + b.y*a.vy + b.z*a.vz; }
  template<typename T> __forceinline LinearSpace3<T> operator*(const LinearSpace3<T>& a, const LinearSpace3<T>& b) { return LinearSpace3<T>(a*b.vx, a*b.vy, a*b.vz); }
  template<typename T> __forceinline LinearSpace3<T> operator/(const LinearSpace3<T>& a, const LinearSpace3<T>& b) { return a * rcp(b); }
  template<typename T> __forceinline LinearSpace3<T> operator/(const LinearSpace3<T>& a, const T              & b) { return a * rcp(b); }

  template<typename T> __forceinline LinearSpace3<T>& operator *=( LinearSpace3<T>& a, const LinearSpace3<T>& b ) { return a = a * b; }
  template<typename T> __forceinline LinearSpace3<T>& operator *=( LinearSpace3<T>& a, const T              & b ) { return a = a * b; }
  template<typename T> __forceinline LinearSpace3<T>& operator /=( LinearSpace3<T>& a, const LinearSpace3<T>& b ) { return a = a / b; }
  template<typename T> __forceinline LinearSpace3<T>& operator /=( LinearSpace3<T>& a, const T              & b ) { return a = a / b; }

  template<typename T> __forceinline Vec3<T> xfmPoint (const LinearSpace3<T>& s, const Vec3<T>& a) { return a.x*s.vx + a.y*s.vy + a.z*s.vz; }
  template<typename T> __forceinline Vec3<T> xfmVector(const LinearSpace3<T>& s, const Vec3<T>& a) { return a.x*s.vx + a.y*s.vy + a.z*s.vz; }
  template<typename T> __forceinline Vec3<T> xfmNormal(const LinearSpace3<T>& s, const Vec3<T>& a) { return xfmVector(s.inverse().transposed(),a); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline bool operator ==( const LinearSpace3<T>& a, const LinearSpace3<T>& b ) { return a.vx == b.vx && a.vy == b.vy && a.vz == b.vz; }
  template<typename T> __forceinline bool operator !=( const LinearSpace3<T>& a, const LinearSpace3<T>& b ) { return a.vx != b.vx || a.vy != b.vy || a.vz != b.vz; }

  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> static std::ostream& operator<<(std::ostream& cout, const LinearSpace3<T>& m) {
    return cout << "{ vx = " << m.vx << ", vy = " << m.vy << ", vz = " << m.vz << "}";
  }

  /*! Shortcuts for common linear spaces. */
  typedef LinearSpace3<float>  LinearSpace3f;
  typedef LinearSpace3<double> LinearSpace3d;
}

#endif
