//
//  SOFAAmbisonicsNormalization.h
//  libsofa
//
//  Created by Andres Pérez López on 05/04/18.
//  Based on SOFACoordinates.h
//

/*
 Copyright (c) 2013--2017, UMR STMS 9912 - Ircam-Centre Pompidou / CNRS / UPMC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the <organization> nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 
 Spatial acoustic data file format - AES69-2015 - Standard for File Exchange - Spatial Acoustic Data File Format
 http://www.aes.org
 
 SOFA (Spatially Oriented Format for Acoustics)
 http://www.sofaconventions.org
 
 */


#ifndef _SOFA_AMBISONICS_NORMALIZATION_H__
#define _SOFA_AMBISONICS_NORMALIZATION_H__

#include "../src/SOFAPlatform.h"
#include "netcdf.h"
#include "ncFile.h"

namespace sofa
{
    
    /************************************************************************************/
    /*!
     *  @class          AmbisonicsNormalization
     *  @brief          Static class to represent information about AmbisonicsDRIR normalization
     *
     *  @details        valid strings: "acn", "sid" and "fuma"
     */
    /************************************************************************************/
    class SOFA_API AmbisonicsNormalization
    {
    public:
        
        enum Type
        {
            kSn3d                               = 0,    ///< SN3D
            kN3d                                = 1,    ///< N3D
            kFuma                               = 2,    ///< FUMA
            kMaxn                               = 3,    ///< MAXN
            kNumAmbisonicsNormalizationTypes    = 4
        };
        
    public:
        static std::string GetName(const sofa::AmbisonicsNormalization::Type &type_);
        static sofa::AmbisonicsNormalization::Type GetType(const std::string &name);
        
        static bool IsValid(const std::string &name);
        
        static bool IsValid(const netCDF::NcAtt & attr);
        
    private:
        AmbisonicsNormalization() SOFA_DELETED_FUNCTION;
    };
    
}

#endif /* _SOFA_AMBISONICS_NORMALIZATION_H__ */
