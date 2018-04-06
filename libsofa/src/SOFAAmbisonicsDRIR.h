//
//  SOFAAmbisonicsDRIR.h
//  libsofa
//
//  Created by Andres Pérez López on 03/04/18.
//

/************************************************************************************/
/*
 *                                      SPECS v0.1
 *
 *  AmbisonicsDRIR is based on GeneralFIRE, with the following additions:
 *
 *  GLOBAL ATTRIBUTES:
 *
 *  -   Mandatory global attribute "AmbisonicsOrder", with arbitrary string value (e.g. "1", "2v1h", etc)
 *
 *  -   Optional global attribute "MicrophoneModel"
 *
 *  -   Optional global attribute "AmbisonicsConversionMethod"
 *
 *
 *  VARIABLES:
 *
 *  -   Mandatory string attribute "ChannelOrdering" for variable "Data.IR".
 *      Possible values: "acn", "sid" and "fuma". Default to "acn".
 *
 *  -   Mandatory string attribute "Normalization" for variable "Data.IR".
 *      Possible values: "sn3d", "n3d", "fuma" and "maxn". Default to "sn3d".
 *
 *  -   Mandatory variable "ListenerUp", dimension [I,C] or [M,C], default to [0 0 1]
 *      Mandatory attribute "ListenerUp:Type", value "cartesian" or "spherical", default to "cartesian"
 *
 *  -   Mandatory variable "ListenerView", dimension [I,C] or [M,C], default to [1 0 0]
 *      Mandatory attribute "ListenerView", value "cartesian" or "spherical", default to "cartesian"
 *
 *  -   Mandatory variable "EmitterUp", dimension [E, C, I] or [E, C, M], default to [0 0 1]
 *      Mandatory attribute "EmitterUp", value "cartesian" or "spherical", default to "cartesian"
 *
 *  -   Mandatory variable "EmitterView", dimension [E, C, I] or [E, C, M], default to [1 0 0]
 *      Mandatory attribute "EmitterView", value "cartesian" or "spherical", default to "cartesian"
 *
 * 
 *  COMMENTS:
 *
 *  -   "Ambisonics channels" are stored in the Receiver dimension.
 *      Therefore, the values stored in the variable "ReceiverPosition" are ommited,
 *      since the information is already transformed into the Spherical Harmonics domain.
 */
/************************************************************************************/



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


/************************************************************************************/
/*!
 *   @file       SOFAAmbisonicsDRIR.h
 *   @brief      Class for SOFA files with AmbisonicsDRIR convention
 *   @author     Andrés Pérez López, Pompeu Fabra University - Eurecat
 *               based on the work of Thibaut Carpentier, UMR STMS 9912 - Ircam-Centre Pompidou / CNRS / UPMC
 *
 *   @date       03/04/2018
 *
 */
/************************************************************************************/
#ifndef _SOFA_AMBISONICS_DRIR_H__
#define _SOFA_AMBISONICS_DRIR_H__

#include "../src/SOFAFile.h"

namespace sofa
{
    
    /************************************************************************************/
    /*!
     *  @class          SOFAAmbisonicsDRIR
     *  @brief          Class for SOFA files with AmbisonicsDRIR convention
     *
     *  @details        Provides methods specific to SOFA files with AmbisonicsDRIR convention
     */
    /************************************************************************************/
    class SOFA_API AmbisonicsDRIR : public sofa::File
    {
    public:
        static const unsigned int ConventionVersionMajor;
        static const unsigned int ConventionVersionMinor;
        static std::string GetConventionVersion();
        
    public:
        AmbisonicsDRIR(const std::string &path,
                       const netCDF::NcFile::FileMode &mode = netCDF::NcFile::read);
        
        virtual ~AmbisonicsDRIR() {};
        
        virtual bool IsValid() const SOFA_OVERRIDE;
        
        bool GetSamplingRate(double &value) const;
        bool GetSamplingRateUnits(sofa::Units::Type &units) const;
        
        //==============================================================================
        bool GetDataIR(std::vector< double > &values) const;
        bool GetDataIR(double *values, const unsigned long dim1, const unsigned long dim2, const unsigned long dim3, const unsigned long dim4) const;
        
        bool GetDataIRChannelOrdering(sofa::AmbisonicsChannelOrdering::Type &channelOrdering) const;
        bool GetDataIRNormalization(sofa::AmbisonicsNormalization::Type &normalization) const;
        
        //==============================================================================
        bool GetDataDelay(double *values, const unsigned long dim1, const unsigned long dim2, const unsigned long dim3) const;
        
    private:
        //==============================================================================
        bool checkGlobalAttributes() const;
        bool checkListenerVariables() const;
        bool checkEmitterVariables() const;
        bool checkDataIRVariables() const;
        
    private:
        /// avoid shallow and copy constructor
        SOFA_AVOID_COPY_CONSTRUCTOR( AmbisonicsDRIR );
    };
    
}

#endif /* _SOFA_AMBISONICS_DRIR_H__ */


