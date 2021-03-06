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

#ifndef __EMBREE_LAMBERTIAN_BRDF_H__
#define __EMBREE_LAMBERTIAN_BRDF_H__

#include "brdfs/brdf.h"

namespace embree
{
  /*! Lambertian BRDF. A lambertian surface is a surface that reflects
   *  the same intensity independent of the viewing direction. The
   *  BRDF has a reflectivity parameter that determines the color of
   *  the surface. */
  class Lambertian : public BRDF
  {
  public:

    /*! Lambertian BRDF constructor. This is a diffuse reflection BRDF. */
    __forceinline Lambertian(const Col3f& R, const BRDFType type=DIFFUSE_REFLECTION) : BRDF(type), R(R) {}

    __forceinline Col3f eval(const Vec3f& wo, const DifferentialGeometry& dg, const Vec3f& wi) const {
      return R * (1.0f/float(pi)) * clamp(dot(wi,dg.Ns));
    }

    Col3f sample(const Vec3f& wo, const DifferentialGeometry& dg, Sample3f& wi, const Vec2f& s) const {
      return eval(wo, dg, wi = cosineSampleHemisphere(s.x,s.y,dg.Ns));
    }

    float pdf(const Vec3f& wo, const DifferentialGeometry& dg, const Vec3f& wi) const {
      return cosineSampleHemispherePDF(wi,dg.Ns);
    }

  private:

    /*! The reflectivity parameter. The vale 0 means no reflection,
     *  and 1 means full reflection. */
    Col3f R;
  };

  /*! Lambertian BRDF. A lambertian surface is a surface that reflects
   *  the same intensity independent of the viewing direction. The
   *  BRDF has a reflectivity parameter that determines the color of
   *  the surface. */
  class LambertianTransmission : public BRDF
  {
  public:

    /*! Lambertian BRDF constructor. This is a diffuse reflection BRDF. */
    __forceinline LambertianTransmission(const Vec3f& N, const Col3f& T) : BRDF(DIFFUSE_TRANSMISSION), N(N), T(T) {}

    __forceinline Col3f eval(const Vec3f& wo, const DifferentialGeometry& dg, const Vec3f& wi) const {
      if (dot(wi,-dg.Ng) <= 0) return zero;
      return T * float(one_over_pi) * clamp(dot(wi,-N));
    }

    Col3f sample(const Vec3f& wo, const DifferentialGeometry& dg, Sample3f& wi, const Vec2f& s) const {
      return eval(wo, dg, wi = cosineSampleHemisphere(s.x,s.y,-N));
    }

    float pdf(const Vec3f& wo, const DifferentialGeometry& dg, const Vec3f& wi) const {
      return cosineSampleHemispherePDF(wi,-N);
    }

  private:

    Vec3f N;

    /*! The reflectivity parameter. The vale 0 means no transmission,
     *  and 1 means full transmission. */
    Col3f T;
  };

  /*! Lambertian BRDF. A lambertian surface is a surface that reflects
   *  the same intensity independent of the viewing direction. The
   *  BRDF has a reflectivity parameter that determines the color of
   *  the surface. */
  class LambertianHorizon : public BRDF
  {
  public:

    /*! Lambertian BRDF constructor. This is a diffuse reflection BRDF. */
    __forceinline LambertianHorizon(const Vec3f& dx, const Vec3f& dy, const Vec3f& dz, const Col3f& R, 
                                    const float nhx, const float phx,
                                    const float nhy, const float phy,
                                    const float blend, const float strength) 
      : BRDF(DIFFUSE_REFLECTION), dx(dx), dy(dy), dz(dz), R(R), nhx(nhx), phx(phx), nhy(nhy), phy(phy), blend(blend), strength(strength) {}

    __forceinline Col3f eval(const Vec3f& wo, const DifferentialGeometry& dg, const Vec3f& wi) const 
    {
      if (dot(wi,dg.Ng) <= 0) return zero;
      const float znx = clamp((dot(wi,dg.Ns)-strength*nhx+blend)/(2.0f*blend)); const float wnx = sqr(clamp(dot(wi,-dx)));
      const float zpx = clamp((dot(wi,dg.Ns)-strength*phx+blend)/(2.0f*blend)); const float wpx = sqr(clamp(dot(wi,+dx)));
      const float zny = clamp((dot(wi,dg.Ns)-strength*nhy+blend)/(2.0f*blend)); const float wny = sqr(clamp(dot(wi,-dy)));
      const float zpy = clamp((dot(wi,dg.Ns)-strength*phy+blend)/(2.0f*blend)); const float wpy = sqr(clamp(dot(wi,+dy)));
      const float w = wnx + wpx + wny + wpy;
      if (w == 0.0f) return R * float(one_over_pi) * clamp(dot(wi,dz));
      const float z = (wnx*znx + wpx*zpx + wny*zny + wpy*zpy)/w;
      return z * R * float(one_over_pi) * clamp(dot(wi,dz));
    }

    Col3f sample(const Vec3f& wo, const DifferentialGeometry& dg, Sample3f& wi, const Vec2f& s) const {
      return eval(wo, dg, wi = cosineSampleHemisphere(s.x,s.y,dz));
    }
    
    float pdf(const Vec3f& wo, const DifferentialGeometry& dg, const Vec3f& wi) const {
      return cosineSampleHemispherePDF(wi,dz);
    }

  private:

    Vec3f dx,dy,dz;

    /*! The reflectivity parameter. The vale 0 means no reflection,
     *  and 1 means full reflection. */
    Col3f R;
        
    float nhx,phx;
    float nhy,phy;

    float blend;
    float strength;
  };
}

#endif
