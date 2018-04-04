//
//  main.cpp
//  create_test_ambisonicsdrir
//
//  Created by Andres Pérez López on 03/04/18.
//
//

#include "SOFA.h"
#include "SOFAString.h"
#include "ncDim.h"
#include "ncVar.h"
#include "ncFile.h"


/************************************************************************************/
static void CreateAmbisonicsDRIRFile()
{
    //==============================================================================
    /// create the file
    
    /// for creating a new file
    const netCDF::NcFile::FileMode mode = netCDF::NcFile::newFile;
    
    /// the file format that is used (netCDF4 / HDF5)
    const netCDF::NcFile::FileFormat format = netCDF::NcFile::nc4;
    
    /// the file shall not exist beforehand
    const std::string filePath = "/Volumes/Dinge/AmbisonicsDRIR/testAmbisonicsDRIR.sofa";
    
    const netCDF::NcFile theFile( filePath, mode, format );
    
    //==============================================================================
    /// create the attributes
    sofa::Attributes attributes;
    attributes.ResetToDefault();
    
    /// fill the attributes as you want
    {
        attributes.Set( sofa::Attributes::kRoomLocation,   "Eurecat Studio, Barcelona" );
        attributes.Set( sofa::Attributes::kRoomShortName,  "Eurecat25" );
        /// etc.
    }
    
    /// put all the attributes into the file
    for( unsigned int k = 0; k < sofa::Attributes::kNumAttributes; k++ )
    {
        const sofa::Attributes::Type attType = static_cast< sofa::Attributes::Type >(k);
        
        const std::string attName  = sofa::Attributes::GetName( attType );
        const std::string attValue = attributes.Get( attType );
        
        theFile.putAtt( attName, attValue );
    }
    
    /// add attribute specific to your convention (e.g. 'DatabaseName' for the 'SimpleFreeFieldHRIR' convention)
    {
        const std::string attName  = "DatabaseName";
        const std::string attValue = "TestDatabase";
        
        theFile.putAtt( attName, attValue );
    }
    
    //==============================================================================
    /// create the dimensions
    const unsigned int numMeasurements  = 1680;
    const unsigned int numReceivers     = 2;
    const unsigned int numEmitters      = 1;
    const unsigned int numDataSamples   = 941;
    
    theFile.addDim( "C", 3 );   ///< this is required by the standard
    theFile.addDim( "I", 1 );   ///< this is required by the standard
    theFile.addDim( "M", numMeasurements );
    theFile.addDim( "R", numReceivers );
    theFile.addDim( "E", numEmitters );
    theFile.addDim( "N", numDataSamples );
    
    //==============================================================================
    /// create the variables
    
    /// Data.SamplingRate
    {
        const std::string varName  = "Data.SamplingRate";
        const std::string typeName = "double";
        const std::string dimName  = "I";
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimName );
        
        const double samplingRate = 48000;
        
        var.putVar( &samplingRate );
        var.putAtt( "Units", "hertz" );
    }
    
    /// Data.Delay
    {
        const std::string varName  = "Data.Delay";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("I");
        dimNames.push_back("R");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        ///@todo : fill the variable
    }
    
    /// ListenerPosition
    {
        const std::string varName  = "ListenerPosition";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("I");
        dimNames.push_back("C");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "cartesian" );
        var.putAtt( "Units", "meter" );
        
        ///@todo : fill the variable
    }
    
    /// ListenerUp
    {
        const std::string varName  = "ListenerUp";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("I");
        dimNames.push_back("C");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        ///@todo : fill the variable
    }
    
    /// ListenerView
    {
        const std::string varName  = "ListenerView";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("I");
        dimNames.push_back("C");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "cartesian" );
        var.putAtt( "Units", "meter" );
        
        ///@todo : fill the variable
    }
    
    /// ReceiverPosition
    {
        const std::string varName  = "ReceiverPosition";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("R");
        dimNames.push_back("C");
        dimNames.push_back("I");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "cartesian" );
        var.putAtt( "Units", "meter" );
        
        ///@todo : fill the variable
    }
    
    /// SourcePosition
    {
        const std::string varName  = "SourcePosition";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("M");
        dimNames.push_back("C");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "spherical" );
        var.putAtt( "Units", "degree, degree, meter" );
        
        ///@todo : fill the variable
    }
    
    /// EmitterPosition
    {
        const std::string varName  = "EmitterPosition";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("E");
        dimNames.push_back("C");
        dimNames.push_back("I");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "cartesian" );
        var.putAtt( "Units", "meter" );
        
        ///@todo : fill the variable
        const double fillValue = 0.0;
        var.setFill( true, fillValue );
    }
    
    /// Data.IR
    {
        const std::string varName  = "Data.IR";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("M");
        dimNames.push_back("R");
        dimNames.push_back("N");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        ///@todo : fill the variable
    }
    
    /// RoomVolume
    {
        const std::string varName  = "RoomVolume";
        const std::string typeName = "double";
        
        const std::string dimName  = "I";
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimName );
        
        var.putAtt( "Units", "cubic meter" );
        
        const double roomVolume = 103;
        var.putVar( &roomVolume );
    }
    
    ///@todo add any other variables, as you need
}


/************************************************************************************/
/*!
 *  @brief          Main entry point
 *
 */
/************************************************************************************/
int main(int argc, char *argv[])
{

    
    const std::string filename = "/Volumes/Dinge/HRTF/Data_Andres/HRIR_measured_pp73_pos_0_0.sofa";

    /// example for creating a SimpleFreeFieldHRIR file
    CreateAmbisonicsDRIRFile();
    
    return 0;
}