//
//  convert_S3A_to_AmbisonicsDRIR.cpp
//  libsofa
//
//  Created by Andres Pérez López on 03/04/18.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "SOFA.h"
#include "SOFAString.h"
#include "ncDim.h"
#include "ncVar.h"
#include "ncFile.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"


/************************************************************************************/
/*!
 *  @brief          Helper function to  access element [i][j][k] of a "3D array" of dimensions [dim1][dim2][dim3]
 *                  stored in a 1D data array
 *
 */
/************************************************************************************/
static inline const std::size_t array3DIndex(const unsigned long i,
                                             const unsigned long j,
                                             const unsigned long k,
                                             const unsigned long dim1,
                                             const unsigned long dim2,
                                             const unsigned long dim3)
{
    return dim2 * dim3 * i + dim3 * j + k;
}


/************************************************************************************/
/*
 *                  UTILS
 */
/************************************************************************************/

std::string extractFolderName(std::string path)
{
    std::string delimiter = "/";
    std::string token;
    size_t pos = 0;
    std::string s = path;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        s.erase(0, pos + delimiter.length());
    }
    return s;
}

unsigned long long find_maximum_sample_length(std::string audio_folder, unsigned int num_files)
{
    std::string audio_file_name_prefix = audio_folder + "/Soundfield/" + "ls";
    std::string audio_file_name_suffix = ".wav";
    std::string audio_file_name;
    unsigned long long max_num_samples = 0;
    
    for (size_t ls_index=0; ls_index < num_files; ls_index++)
    {
        // achtung! file name counting starts at 1
        audio_file_name = audio_file_name_prefix + std::to_string(ls_index+1) + audio_file_name_suffix;
        drwav* pWav = drwav_open_file(audio_file_name.c_str());
        if (pWav == NULL) {
             // Error opening WAV file.
        }
        if (pWav->totalSampleCount > max_num_samples)
        {
            max_num_samples = pWav->totalSampleCount;
        }
        drwav_close(pWav);
    }
    return max_num_samples;
}

/* get sample rate from first recording */
double get_sample_rate(std::string audio_folder)
{
    std::string audio_file_name_prefix = audio_folder + "/Soundfield/" + "ls";
    std::string audio_file_name_suffix = ".wav";
    std::string audio_file_name;
    double sample_rate = 0;
    
    audio_file_name = audio_file_name_prefix + std::to_string(1) + audio_file_name_suffix;
    drwav* pWav = drwav_open_file(audio_file_name.c_str());
    sample_rate = pWav->sampleRate;
    drwav_close(pWav);
    
    return sample_rate;
}
static void DisplayHelp(std::ostream & output = std::cout)
{
    output << "converts a S3A database to AmbisonicsDRIR format"                                        << std::endl;
    output << ""                                                                                        << std::endl;
    output << "    syntax : ./convert_S3A_to_AmbisonicsDRIR [input_path] [output_path]"                 << std::endl;
    output << ""                                                                                        << std::endl;
    output << "    [input_path] must contain (at least) a folder called `Soundfield`,"                  << std::endl;
    output << "    with `E` wav files (the IRs) and a `LsPos.txt` file with the loudspeaker positions"  << std::endl;
    output << ""                                                                                        << std::endl;
}



/************************************************************************************/
/*
 *                  MAIN
 */
/************************************************************************************/

int main(int argc, char *argv[])
{
    
    std::ostream & output = std::cout;
    std::string in;
    
    //==============================================================================
    // Parsing arguments
    //==============================================================================e
    if( argc == 3 )
    {
        in = argv[1];
        
        if( in == "h" || in == "-h" || in == "--h" || in == "--help" || in == "-help" )
        {
            DisplayHelp( output );
            return 0;
        }
    }
    else
    {
        DisplayHelp( output );
        return 0;
    }
    
    
    //==============================================================================
    // Check audio files
    //==============================================================================
    
    const std::string input_folder_path = in;
    std::string audio_file_name_prefix = input_folder_path + "/Soundfield/" + "ls";
    std::string audio_file_name_suffix = ".wav";
    
    unsigned int ls_index = 0; // achtung! file name counting starts at 1
    bool res = true;
    
    while (res == true)
    {
        std::string audio_file_name = audio_file_name_prefix + std::to_string(ls_index+1) + audio_file_name_suffix;
        std::ifstream f(audio_file_name.c_str());
        res = f.good();
        if (res) ls_index++;
    }
    
    /* now we know how many loudspeakers there should be */
    unsigned int num_loudspeaker_files = ls_index;

    
    //==============================================================================
    // Parse metadata file
    //==============================================================================
    
    const std::string   speaker_positions_file_name = "/Soundfield/LsPos.txt";
    const std::string   speaker_positions_file_path = input_folder_path + speaker_positions_file_name;
    std::ifstream       speaker_positions_file(speaker_positions_file_path);

    double  loudspeaker_positions[num_loudspeaker_files*3]; // let's interleave
    float   x, y, z;
    ls_index = 0;

    while ( speaker_positions_file >> x >> y >> z )
    {
        loudspeaker_positions[ (ls_index*3) + 0 ] = x;
        loudspeaker_positions[ (ls_index*3) + 1 ] = y;
        loudspeaker_positions[ (ls_index*3) + 2 ] = z;
        //    std::cout << x << " " << y << " " << z << " " << std::endl;
        ls_index++;
    }
    /* number of files in the text file should correspond to number of audio files... */
    if (num_loudspeaker_files != ls_index)
    {
        std::cerr << "ERROR: discrepancy on number of loudspeakers!" << std::endl;
        return -1;
    }
    
    
    //==============================================================================
    // Create AmbisonicsDRIR File
    //==============================================================================
    
    //==============================================================================
    /// create the file
    
    /// for creating a new file
    const netCDF::NcFile::FileMode mode = netCDF::NcFile::newFile;
    
    /// the file format that is used (netCDF4 / HDF5)
    const netCDF::NcFile::FileFormat format = netCDF::NcFile::nc4;
    
    /// the file shall not exist beforehand
    std::string file_name   = extractFolderName(input_folder_path);
    std::string slash       = "/";
    
    const std::string output_file_path = argv[2] + slash + file_name + ".sofa";
    
    const netCDF::NcFile theFile( output_file_path, mode, format );
    
    //==============================================================================
    /// create the attributes
    sofa::Attributes attributes;
    attributes.ResetToDefault();
    
    /// fill the attributes as you want
    {
        
        attributes.Set( sofa::Attributes::kSOFAConventions,         "AmbisonicsDRIR" );
        attributes.Set( sofa::Attributes::kSOFAConventionsVersion,  "0.1" );
        attributes.Set( sofa::Attributes::kDataType,                "FIRE" );
        attributes.Set( sofa::Attributes::kTitle,                   file_name );
        attributes.Set( sofa::Attributes::kApplicationName,         "convert_S3A_to_AmbisonicsDRIR" );
        attributes.Set( sofa::Attributes::kApplicationVersion,      "0.1" );
        attributes.Set( sofa::Attributes::kReferences,              "Ambisonics Directional Room Impulse Response as a new Convention of the Spatially Oriented Format for Acoustics" );
        attributes.Set( sofa::Attributes::kRoomType,                "reverberant" );
        
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
        /* Ambisonics Order */
        theFile.putAtt( "AmbisonicsOrder", "1" );
        // TODO: MAYBE USE INT TYPE??
        // NO!! "in SOFA, the global attributes must always be strings" (SOFAFile.cpp, line 1689)
        
        /* Ambisonics Channel Ordering */
        theFile.putAtt( "AmbisonicsChannelOrdering", "FuMa" );
        
        /* Ambisonics Normalization */
        theFile.putAtt( "AmbisonicsNormalization", "FuMa" );
    }
    
    //==============================================================================
    /// create the dimensions
    const unsigned int numMeasurements  = 1;                        // only one listener position in all S3A files
    const unsigned int numReceivers     = 4;                        // Ambisonics First Order by default
    const unsigned int numEmitters      = num_loudspeaker_files;
    /* 
     *  All IRs must have the same length.
     *  So let's open all of them and find the maximum,
     *  zero-padding the rest
     */
    unsigned long long num_max_samples = find_maximum_sample_length(input_folder_path, num_loudspeaker_files);
    const unsigned int numDataSamplesPerChannel   = (const unsigned int)num_max_samples / numReceivers;
    
    theFile.addDim( "C", 3 );   ///< this is required by the standard
    theFile.addDim( "I", 1 );   ///< this is required by the standard
    theFile.addDim( "M", numMeasurements );
    theFile.addDim( "R", numReceivers );
    theFile.addDim( "E", numEmitters );
    theFile.addDim( "N", numDataSamplesPerChannel );  // per channel!!
    
    //==============================================================================
    /// create the variables
    
    /// Data.SamplingRate
    {
        const std::string varName  = "Data.SamplingRate";
        const std::string typeName = "double";
        const std::string dimName  = "I";
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimName );
        
        const double samplingRate = get_sample_rate(input_folder_path);
        
        var.putVar( &samplingRate );
        var.putAtt( "Units", "hertz" );
    }
    
    /// Data.Delay
    {
        const std::string varName  = "Data.Delay";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("M");
        dimNames.push_back("R");
        dimNames.push_back("E");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        ///@todo : fill the variable
    }
    
    /// ListenerPosition
    {
        const std::string varName  = "ListenerPosition";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("M");
        dimNames.push_back("C");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "cartesian" );
        var.putAtt( "Units", "meter" );
        
        /* zeros */
        const double fillValue = 0.0;
        var.setFill( true, fillValue );
    }
    
    /// ListenerUp
    {
        const std::string varName  = "ListenerUp";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("M");
        dimNames.push_back("C");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "cartesian" );
        var.putAtt( "Units", "meter" );
        
        /* zeros */
        const double fillValue = 0.0;
        var.setFill( true, fillValue );
    }
    
    /// ListenerView
    {
        const std::string varName  = "ListenerView";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("M");
        dimNames.push_back("C");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "cartesian" );
        var.putAtt( "Units", "meter" );
        
        /* zeros */
        const double fillValue = 0.0;
        var.setFill( true, fillValue );
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
        
        /* zeros */
        const double fillValue = 0.0;
        var.setFill( true, fillValue );
    }
    
    /// SourcePosition
    {
        const std::string varName  = "SourcePosition";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("I");
        dimNames.push_back("C");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "cartesian" );
        var.putAtt( "Units", "meter" );
        
        /* zeros */
        const double fillValue = 0.0;
        var.setFill( true, fillValue );
    }
    
    /// -------------------------------------------------
    /// EmitterPosition
    {
        const std::string varName  = "EmitterPosition";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("E");
        dimNames.push_back("C");
        dimNames.push_back("M");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "cartesian" );
        var.putAtt( "Units", "meter" );

        var.putVar((const double*)loudspeaker_positions);    // not really sure if the casting is needed
    }
    
    /// -------------------------------------------------
    /// Data.IR
    {
        const std::string varName  = "Data.IR";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("M");
        dimNames.push_back("R");
        dimNames.push_back("E");
        dimNames.push_back("N");

        
        /* let's open all audio files in a row, copy all the data in the same buffer, and put it into the var */
        
        std::string audio_file_name_prefix = input_folder_path + "/Soundfield/" + "ls";
        std::string audio_file_name_suffix = ".wav";
        std::string audio_file_name;
        
        // M * R * E * N
        uint64_t total_num_samples = numDataSamplesPerChannel * numEmitters * numReceivers;
        
        float* audiodata = (float*)calloc(total_num_samples,sizeof(float));
        uint64_t total_num_samples_read = 0; // accumulated though all files
        
        for (size_t file_idx=0; file_idx<num_loudspeaker_files; file_idx++)
        {
            /* ensemble file name */
            audio_file_name = audio_file_name_prefix + std::to_string(file_idx+1) + audio_file_name_suffix;
            
            /* open file */
            drwav* pWav = drwav_open_file(audio_file_name.c_str());
            if (pWav == NULL)
            {
                std::cerr << "ERROR: opening file :" + audio_file_name << std::endl;
                free(audiodata);
                return -1;
            }
            
            /* read as f32 anyway, and  */
            uint64_t num_samples_read = drwav_read_f32(pWav,
                                                       numDataSamplesPerChannel*numReceivers,
                                                       audiodata+total_num_samples_read);
            
            /* we should have read the whole file */
            if (num_samples_read != numDataSamplesPerChannel*numReceivers)
            {
                std::cerr << "ACTHUNG!!!!!!" << std::endl;
            }
            total_num_samples_read += num_samples_read;

            /* don't forget it */
            drwav_close(pWav);
        }
        
        /* estimated and real total num samples should match */
        if (total_num_samples != total_num_samples_read)
        {
            std::cerr << "ACTHUNG!!!!!! num total samples not matching" << std::endl;
        }
        
        /*
         *  At this point, we have all audio data in interleaved form, but with order (M)ENR,
         *  and the SOFA specs defines FIRE data as (M)REN.
         *  Therefore, we whould swap dimmensions...
         */
        float* audiodata_reordered = (float*)calloc(total_num_samples,sizeof(float));
        
        uint64_t index_ENR = 0;     //current
        uint64_t index_REN = 0;     //reordered
        
        uint64_t E = numEmitters;
        uint64_t N = numDataSamplesPerChannel;
        uint64_t R = numReceivers;
        
        for( uint64_t e = 0; e < E; e++ )   // num speakers
        {
            for( uint64_t n = 0; n < N; n++ )   // num samples
            {
                for( uint64_t r = 0; r < R; r++ )   // num channels
                {
                    index_ENR = array3DIndex(e,n,r,E,N,R);
                    index_REN = array3DIndex(r,e,n,R,E,N);
//                    std::cerr << index_ENR << "\t -> \t" << index_REN << std::endl;
                    audiodata_reordered[index_REN] = audiodata[index_ENR];
                }
            }
        }
        
        /* now put all data at once in the variable */
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        try
        {
            var.putVar(audiodata_reordered);
        } catch (netCDF::exceptions::NcException& e) {
            std::cerr << "ERROR: processing audio" << std::endl;
        }
        
        /* don't forget it */
        free(audiodata_reordered);
        free(audiodata);
    }
    
    
    return 0;
}
