//
//  SOFAAmbisonicsDRIR.cpp
//  libsofa
//
//  Created by Andres Pérez López on 03/04/18.
//
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


/************************************************************************************/
/*!
 *   @file       SOFAAmbisonicsDRIR.cpp
 *   @brief      Class for SOFA files with AmbisonicsDRIR convention
 *   @author     Andrés Pérez López, Pompeu Fabra University - Eurecat
 *               based on the work of Thibaut Carpentier, UMR STMS 9912 - Ircam-Centre Pompidou / CNRS / UPMC
 *
 *   @date       03/04/2018
 *
 */
/************************************************************************************/
#include "../src/SOFAAmbisonicsDRIR.h"
#include "../src/SOFAExceptions.h"
#include "../src/SOFAUtils.h"
#include "../src/SOFANcUtils.h"
#include "../src/SOFAString.h"
#include "../src/SOFAListener.h"
#include "../src/SOFASource.h"
#include "../src/SOFAEmitter.h"

using namespace sofa;

const unsigned int AmbisonicsDRIR::ConventionVersionMajor  =   0;
const unsigned int AmbisonicsDRIR::ConventionVersionMinor  =   1;

std::string AmbisonicsDRIR::GetConventionVersion()
{
    return sofa::String::Int2String( AmbisonicsDRIR::ConventionVersionMajor ) + std::string(".") + sofa::String::Int2String( AmbisonicsDRIR::ConventionVersionMinor );
}

/************************************************************************************/
/*!
 *  @brief          Class constructor
 *  @param[in]      path : the file path
 *  @param[in]      mode : opening mode
 *
 */
/************************************************************************************/
AmbisonicsDRIR::AmbisonicsDRIR(const std::string &path,
                               const netCDF::NcFile::FileMode &mode)
: sofa::File( path, mode )
{
}

bool AmbisonicsDRIR::checkGlobalAttributes() const
{
    sofa::Attributes attributes;
    GetGlobalAttributes( attributes );
    
    sofa::File::ensureSOFAConvention( "AmbisonicsDRIR" );
    sofa::File::ensureDataType( "FIRE" );
    
    /* Convention-specific compulsory Global Attributes */
    sofa::File::ensureGlobalAttribute( "AmbisonicsOrder" );
    sofa::File::ensureGlobalAttribute( "AmbisonicsChannelOrdering" );
    sofa::File::ensureGlobalAttribute( "AmbisonicsNormalization" );
    
    return true;
}

bool AmbisonicsDRIR::checkListenerVariables() const
{
    const long I = GetDimension( "I" );
    if( I != 1 )
    {
        SOFA_THROW( "invalid SOFA dimension : I" );
        return false;
    }
    
    const long C = GetDimension( "C" );
    if( C != 3 )
    {
        SOFA_THROW( "invalid SOFA dimension : C" );
        return false;
    }
    
    const long M = GetNumMeasurements();
    if( M <= 0 )
    {
        SOFA_THROW( "invalid SOFA dimension : M" );
        return false;
    }
    
    const long R = GetNumReceivers();
    
    if( R <= 0 )
    {
        SOFA_THROW( "invalid SOFA dimension : R" );
        return false;
    }
 
    /************************ TODO ************************
     *
     *  ENSURE THAT THESE KIND OF OPERATIONS ARE VALID
     *
     *  Not really sure about the concept...
     *    1) Only global attribute with numerical value (but still string)
     *    2) What about mixed orders? (Octomic...)
     *    3) Human error in AmbisonicsOrder attr might fuck up the whole file
     ******************************************************/

//    std::string ambisonicsOrderString   = GetAttributeValueAsString( "AmbisonicsOrder" );
//    uint16_t    ambisonicsOrderUInt16   = ( uint16_t ) std::stoi( ambisonicsOrderString );
//    uint16_t    computedNumReceivers    = ( ambisonicsOrderUInt16 + 1 ) * ( ambisonicsOrderUInt16 + 1 );
//    
//    if( R != computedNumReceivers )
//    {
//        SOFA_THROW( "invalid SOFA dimension : R does not match Ambisonics Order" );
//        return false;
//    }
    
    const netCDF::NcVar varListenerPosition        = NetCDFFile::getVariable( "ListenerPosition" );
    const netCDF::NcVar varListenerUp              = NetCDFFile::getVariable( "ListenerUp" );
    const netCDF::NcVar varListenerView            = NetCDFFile::getVariable( "ListenerView" );
    
    const sofa::Listener listener( varListenerPosition, varListenerUp, varListenerView );
    
    if( listener.IsValid() == false )
    {
        SOFA_THROW( "invalid 'Listener' variables" );
        return false;
    }
    
    if( listener.ListenerPositionHasDimensions(  I,  C ) == false
       && listener.ListenerPositionHasDimensions(  M,  C ) == false )
    {
        SOFA_THROW( "invalid 'ListenerPosition' dimensions" );
        return false;
    }
    
    if( listener.HasListenerUp() == true )
    {
        /// ListenerUp is not required in the Specifications
        /// but if it is present, is should be [ I C ] or [ M C ]
        
        if( listener.ListenerUpHasDimensions(  I,  C ) == false
           && listener.ListenerUpHasDimensions(  M,  C ) == false )
        {
            SOFA_THROW( "invalid 'ListenerUp' dimensions" );
            return false;
        }
    }
    else
    {
        SOFA_THROW( "missing 'ListenerUp' variable" );
        return false;
    }
    
    if( listener.HasListenerView() == true )
    {
        /// ListenerView is not required in the Specifications
        /// but if it is present, is should be [ I C ] or [ M C ]
        
        if( listener.ListenerViewHasDimensions(  I,  C ) == false
           && listener.ListenerViewHasDimensions(  M,  C ) == false )
        {
            SOFA_THROW( "invalid 'ListenerView' dimensions" );
            return false;
        }
    }
    else
    {
        SOFA_THROW( "missing 'ListenerView' variable" );
        return false;
    }
    
    /// everything is OK !
    return true;
}

bool AmbisonicsDRIR::checkEmitterVariables() const
{
    /* Ensure Constants */
    
    const long I = GetDimension( "I" );
    if( I != 1 )
    {
        SOFA_THROW( "invalid SOFA dimension : I" );
        return false;
    }
    
    const long C = GetDimension( "C" );
    if( C != 3 )
    {
        SOFA_THROW( "invalid SOFA dimension : C" );
        return false;
    }
    
    const long M = GetNumMeasurements();
    if( M <= 0 )
    {
        SOFA_THROW( "invalid SOFA dimension : M" );
        return false;
    }

    const long E = GetNumEmitters();
    if( E <= 0 )
    {
        SOFA_THROW( "invalid SOFA dimension : E" );
        return false;
    }
    
    const long N = GetNumDataSamples();
    if( N <= 0 )
    {
        SOFA_THROW( "invalid SOFA dimension : N" );
        return false;
    }
    
    /* Ensure Dimensions */
    
    /* Source*/
    const netCDF::NcVar varSourcePosition           = NetCDFFile::getVariable( "SourcePosition" );
    const netCDF::NcVar varSourceUp                 = NetCDFFile::getVariable( "SourceUp" );
    const netCDF::NcVar varSourceView               = NetCDFFile::getVariable( "SourceView" );
    
    const sofa::Source source( varSourcePosition, varSourceUp, varSourceView);
    
    if( source.IsValid() == false )
    {
        SOFA_THROW( "invalid 'Source' variables" );
        return false;
    }
    
    if( source.SourcePositionHasDimensions(  I,  C ) == false )
    {
        SOFA_THROW( "invalid 'SourcePosition' dimensions" );
        return false;
    }
    
    /* Emitter */
    const netCDF::NcVar varEmitterPosition           = NetCDFFile::getVariable( "EmitterPosition" );
    const netCDF::NcVar varEmitterUp                 = NetCDFFile::getVariable( "EmitterUp" );
    const netCDF::NcVar varEmitterView               = NetCDFFile::getVariable( "EmitterView" );
    
    const sofa::Emitter emitter( varEmitterPosition, varEmitterUp, varEmitterView);
    
    if( emitter.IsValid() == false )
    {
        SOFA_THROW( "invalid 'Emitter' variables" );
        return false;
    }
    
    if( emitter.EmitterPositionHasDimensions( E, C, M ) == false )
    {
        SOFA_THROW( "invalid 'EmitterPsotion' dimensions" );
        return false;
    }
    
    
    
    return true;
}

/************************************************************************************/
/*!
 *  @brief          Returns true if this is a valid SOFA file with AmbisonicsDRIR convention
 *
 */
/************************************************************************************/
bool AmbisonicsDRIR::IsValid() const
{
    if( sofa::File::IsValid() == false )
    {
        return false;
    }
    
    if( IsFIREDataType() == false )
    {
        SOFA_THROW( "'DataType' shall be FIRE" );
        return false;
    }
    
    if( checkGlobalAttributes() == false )
    {
        return false;
    }
    
    if( checkListenerVariables() == false )
    {
        return false;
    }
    
    if( checkEmitterVariables() == false )
    {
        return false;
    }
    
    SOFA_ASSERT( GetDimension( "I" ) == 1 );
    SOFA_ASSERT( GetDimension( "C" ) == 3 );
    
    return true;
}

/************************************************************************************/
/*!
 *  @brief          In case Data.SamplingRate is of dimension [I], this function returns
 *                  its value. In case Data.SamplingRate is of dimension [M], an error is thrown
 *  @return         true on success
 *
 */
/************************************************************************************/
bool AmbisonicsDRIR::GetSamplingRate(double &value) const
{
    SOFA_ASSERT( AmbisonicsDRIR::IsValid() == true );
    
    return sofa::File::getSamplingRate( value );
}

/************************************************************************************/
/*!
 *  @brief          Retrieves the units of the Data.SamplingRate variable
 *  @return         true on success
 *
 */
/************************************************************************************/
bool AmbisonicsDRIR::GetSamplingRateUnits(sofa::Units::Type &units) const
{
    return sofa::File::getSamplingRateUnits( units );
}

/************************************************************************************/
/*!
 *  @brief          Retrieves the Data.IR values
 *  @param[in]      values : array containing the values.
 *                  The array must be allocated large enough
 *  @param[in]      dim1 : first dimension (M)
 *  @param[in]      dim2 : second dimension (R)
 *  @param[in]      dim3 : third dimension (N)
 *  @param[in]      dim4 : fourth dimension (E)
 *  @return         true on success
 *
 */
/************************************************************************************/
bool AmbisonicsDRIR::GetDataIR(double *values,
                               const unsigned long dim1,
                               const unsigned long dim2,
                               const unsigned long dim3,
                               const unsigned long dim4) const
{
    /// Data.IR is [ M R N E ]
    
    return NetCDFFile::GetValues( values, dim1, dim2, dim3, dim4, "Data.IR" );
}


/************************************************************************************/
/*!
 *  @brief          Retrieves the Data.IR values
 *  @param[in]      values : the array is resized if needed
 *  @return         true on success
 *
 */
/************************************************************************************/
bool AmbisonicsDRIR::GetDataIR(std::vector< double > &values) const
{
    /// Data.IR is [ M R N E ]
    
    return sofa::File::getDataIR( values );
}


bool AmbisonicsDRIR::GetDataDelay(double *values, const unsigned long dim1, const unsigned long dim2, const unsigned long dim3) const
{
    /// Data.Delay is [ I R E ] or [ M R E ]
    
    return sofa::File::getDataDelay( values, dim1, dim2, dim3 );
}


