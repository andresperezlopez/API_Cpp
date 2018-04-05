//
//  SOFAAmbisonicsNormalization.cpp
//  libsofa
//
//  Created by Andres Pérez López on 05/04/18.
//  Adapted from SOFACoordinates.cpp
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

#include "../src/SOFAAmbisonicsNormalization.h"
#include "../src/SOFANcUtils.h"
#include <map>

using namespace sofa;


namespace AmbisonicsNormalizationHelper
{
    /************************************************************************************/
    /*!
     *  @brief          Creates a mapping between normalization types and their names
     *
     */
    /************************************************************************************/
    static const std::map< std::string, sofa::AmbisonicsNormalization::Type > & getTypeMap()
    {
        static std::map< std::string, sofa::AmbisonicsNormalization::Type > typeMap;
        
        if( typeMap.empty() == true )
        {
            typeMap["sn3d"]                     = sofa::AmbisonicsNormalization::kSn3d;
            typeMap["n3d"]                      = sofa::AmbisonicsNormalization::kN3d;
            typeMap["fuma"]                     = sofa::AmbisonicsNormalization::kFuma;
            typeMap["maxn"]                     = sofa::AmbisonicsNormalization::kMaxn;
        }
        
        return typeMap;
    }
}

/************************************************************************************/
/*!
 *  @brief          Returns the name of normalization based on its type
 *  @param[in]      type_ : the normalization to query
 *
 */
/************************************************************************************/
std::string sofa::AmbisonicsNormalization::GetName(const sofa::AmbisonicsNormalization::Type &type_)
{
    switch( type_ )
    {
        case sofa::AmbisonicsNormalization::kSn3d                               : return "sn3d";
        case sofa::AmbisonicsNormalization::kN3d                                : return "n3d";
        case sofa::AmbisonicsNormalization::kFuma                               : return "fuma";
        case sofa::AmbisonicsNormalization::kMaxn                               : return "maxn";
            
        default                                                                 : SOFA_ASSERT( false ); return "";
        case sofa::AmbisonicsNormalization::kNumAmbisonicsNormalizationTypes    : SOFA_ASSERT( false ); return "";
    }
}

/************************************************************************************/
/*!
 *  @brief          Returns the ambisonics normalization based on its name
 *                  Returns 'sofa::AmbisonicsNormalization::kNumAmbisonicsNormalizationTypes'
 *                  in case the string does not correspond to a valid normalization
 *  @param[in]      name : the string to query
 *
 */
/************************************************************************************/
sofa::AmbisonicsNormalization::Type sofa::AmbisonicsNormalization::GetType(const std::string &name)
{
    const std::map< std::string, sofa::AmbisonicsNormalization::Type > & typeMap = AmbisonicsNormalizationHelper::getTypeMap();
    
    if( typeMap.count( name ) == 0 )
    {
        /* remove the assert, so program does not crash before showing the error message */
        //        SOFA_ASSERT( false );
        
        return sofa::AmbisonicsNormalization::kNumAmbisonicsNormalizationTypes;
    }
    else
    {
        return typeMap.at( name );
    }
}

/************************************************************************************/
/*!
 *  @brief          Returns true if the string corresponds to a valid ambisonics normalization
 *  @param[in]      name : the string to query
 *
 */
/************************************************************************************/
bool sofa::AmbisonicsNormalization::IsValid(const std::string &name)
{
    const std::map< std::string, sofa::AmbisonicsNormalization::Type > & typeMap = AmbisonicsNormalizationHelper::getTypeMap();
    
    return ( typeMap.count( name ) != 0 );
}

/************************************************************************************/
/*!
 *  @brief          Returns true if a NcAtt properly represents a SOFA AmbisonicsNormalization
 *  @param[in]      attr : the Nc attribute to query
 *
 */
/************************************************************************************/
bool sofa::AmbisonicsNormalization::IsValid(const netCDF::NcAtt & attr)
{
    if( sofa::NcUtils::IsValid( attr ) == false )
    {
        return false;
    }
    
    if( sofa::NcUtils::IsChar( attr ) == false )
    {
        return false;
    }
    
    const std::string positionType = sofa::NcUtils::GetAttributeValueAsString( attr );
    
    if( sofa::AmbisonicsNormalization::IsValid( positionType ) == false )
    {
        return false;
    }
    
    return true;
}



