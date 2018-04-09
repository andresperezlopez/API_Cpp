//
//  convert_openAIR_to_AmbisonicsDRIR.cpp
//  libsofa
//
//  Created by Andres Pérez López on 09/04/18.
//
//

/*
 *  This file is intended for conversion into the AmbisonicsDRIR convention
 *  a number of IRs coming from the great openAIRlib resource.
 *
 *  We will show as an example the conversion procedure for the "York Guildhall Council Chamber"
 *  (http://www.openairlib.net/auralizationdb/content/york-guildhall-council-chamber)
 */


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



//==============================================================================
// Data
//==============================================================================

/*
 *  In this specific case, the audio material is stored in different files,
 *  with the following naming convention:
 *  "councilchamber_s[X]_r[Y]_ir_1_96000.wav", where:
 *      - X denotes source position, and
 *      - Y denotes receiver position.
 *  Furthermore, each file consists of 4 channels (FOA with FuMa convention)
 */

std::string filename_base = "councilchamber";
std::string filename_source_id = "_s";
std::string filename_receiver_id = "_r";
std::string filename_base_end = "_ir_1_96000.wav";


/*
 *  Regarding the positions:
 *  In the specs, the exact positions are not given.
 *  By using some geometrical approximations, we will use the following positions.
 *      - Reference: center of the room
 *      - s1 = [6,0]
 *      - s2 = [1.5, 2.59]
 *      - s3 = [1.5, -2.59]
 *      - r1 = [-6,0]
 *      - r2 = [-2.5, 2.95]
 *      - r3 = [-2.5, -2.95]
 *      - r4 = [-1,0]
 *  All sources' and receivers' height is 1.60 m.
 */


/*
 * Dimensions:
 *  M = 4
 *  E = 3
 */

const unsigned int M = 4;   // number of different measurement (mic) (r) positions
const unsigned int E = 3;   // number of different emitter (speakers) (s) positions



//==============================================================================
// Utils
//==============================================================================

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

unsigned long long find_maximum_sample_length(std::string audio_folder,
                                              unsigned int num_sources,
                                              unsigned int num_receivers)
{
    unsigned long long max_num_samples = 0;
    std::string audio_file_name;
    drwav* pWav;
    
    for (size_t s=0; s<E; s++)
    {
        for (size_t r=0; r<M; r++)
        {
            /* build file name */
            audio_file_name = audio_folder + "/" + filename_base;
            audio_file_name += filename_source_id + std::to_string(s+1);
            audio_file_name += filename_receiver_id + std::to_string(r+1);
            audio_file_name += filename_base_end;
            
            /* open file */
            drwav* pWav = drwav_open_file(audio_file_name.c_str());
            if (pWav == NULL)
            {
                // Error opening WAV file.
                printf("Error opening file %s\n", audio_file_name.c_str() );
                return -1;
            }
                       
           /* check num samples */
           if (pWav->totalSampleCount > max_num_samples)
           {
               max_num_samples = pWav->totalSampleCount;
           }
           drwav_close(pWav);
        }
    }

    return max_num_samples;
}

/* get sample rate from first recording */
double get_sample_rate(std::string audio_folder)
{
    double sample_rate = 0;
    std::string audio_file_name;
    
    /* build file name of first file */
    audio_file_name = audio_folder + "/" + filename_base;
    audio_file_name += filename_source_id + std::to_string(1);
    audio_file_name += filename_receiver_id + std::to_string(1);
    audio_file_name += filename_base_end;
    
    /* open file */
    drwav* pWav = drwav_open_file(audio_file_name.c_str());
    if (pWav == NULL)
    {
        // Error opening WAV file.
        printf("Error opening file %s", audio_file_name.c_str() );
               return -1;
    }
    sample_rate = pWav->sampleRate;
    drwav_close(pWav);
    
    return sample_rate;
}

static void DisplayHelp(std::ostream & output = std::cout)
{
    output << "converts an openAIRlib database to AmbisonicsDRIR format"                                << std::endl;
    output << ""                                                                                        << std::endl;
    output << "    syntax : ./convert_openAIR_to_AmbisonicsDRIR [input_path] [output_path]"             << std::endl;
}



               
//==============================================================================
// Main
//==============================================================================

int main(int argc, char *argv[])
{
    
    std::ostream & output = std::cout;
    std::string in;
    
    //==============================================================================
    // Parsing arguments
    //==============================================================================
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
    // Create AmbisonicsDRIR File
    //==============================================================================
    
    //==============================================================================
    
    std::string input_folder_path = argv[1];
    std::string output_folder_path = argv[2];
    
    /// create the file
    
    /// for creating a new file
    const netCDF::NcFile::FileMode mode = netCDF::NcFile::newFile;
    
    /// the file format that is used (netCDF4 / HDF5)
    const netCDF::NcFile::FileFormat format = netCDF::NcFile::nc4;
    
    /// the file shall not exist beforehand
    std::string file_name   = extractFolderName(input_folder_path);
    std::string slash       = "/";
    
    const std::string output_file_path = output_folder_path + slash + file_name + ".sofa";
    
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
        attributes.Set( sofa::Attributes::kApplicationName,         "convert_openAIR_to_AmbisonicsDRIR" );
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
        
        /* Microphone Model */
        theFile.putAtt( "AmbisonicsMicrophoneModel", "Soundfield ST450 MkII" );
        
        /* Ambisonics Conversion Method */
        theFile.putAtt( "AmbisonicsConversionMethod", "Hardware" );
        
        /* Room Description */
        theFile.putAtt( "RoomDesctiption", "York Guildhall Council Chamber" );
    }
    
    //==============================================================================
    
    /// create the dimensions
    const unsigned int numMeasurements  = M;    // same as number of microphone positions
    const unsigned int numReceivers     = 4;    // Ambisonics First Order by default
    const unsigned int numEmitters      = E;    // same as number of speakers
    /*
     *  All IRs must have the same length.
     *  So let's open all of them and find the maximum,
     *  zero-padding the rest
     */
    unsigned long long num_max_samples = find_maximum_sample_length(input_folder_path, E, M);
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
        
        /* values: zeros by default */
        double* delay = (double*)calloc(numMeasurements*numReceivers*numEmitters,
                                        sizeof(double));
        var.putVar((const double*)delay);
        free(delay);
    }
    
    /// -------------------------------------------------
    /// ListenerPosition
    
    /*
     *      - r1 = [-6, 0, 1.6]
     *      - r2 = [-2.5, 2.95, 1.6]
     *      - r3 = [-2.5, -2.95, 1.6]
     *      - r4 = [-1, 0, 1.6]
     */
    {
        const std::string varName  = "ListenerPosition";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("M");
        dimNames.push_back("C");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "cartesian" );
        var.putAtt( "Units", "meter" );
        
        const double listenerPosition[numMeasurements*3] = {
            -6,     0,      1.6,    \
            -2.5,   2.95,   1.6,    \
            -2.5,   -2.95,  1.6,    \
            -1,     0,      1.6
        };

        var.putVar((const double*)listenerPosition);

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
        
        /* values: [0 0 1] for each measurement position */
        double* listenerUp = (double*)calloc(numMeasurements*3,
                                             sizeof(double));
        for (size_t i=0; i<numMeasurements; i++)
        {
            listenerUp[(i*3)+2] = 1;
        }
        var.putVar((const double*)listenerUp);
        free(listenerUp);
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
        
        /* values: [1 0 0] for each measurement position */
        double* listenerView = (double*)calloc(numMeasurements*3,
                                               sizeof(double));
        for (size_t i=0; i<numMeasurements; i++)
        {
            listenerView[i*3] = 1;
        }
        var.putVar((const double*)listenerView);
        free(listenerView);
    }
    
    /// -------------------------------------------------
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
        
        /* values: [0 0 0] by default */
        double* receiverPosition = (double*)calloc(numReceivers*3*1,
                                                   sizeof(double));
        var.putVar((const double*)receiverPosition);
        free(receiverPosition);
    }
    
    /// -------------------------------------------------
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
        
        /* let's locate it just at the reference center */
        double sourcePosition[3] = {0, 0, 0};
        var.putVar((const double*)sourcePosition);
    }
    
    /// -------------------------------------------------
    /// EmitterPosition
    /*
     *      - s1 = [6, 0, 1.6]
     *      - s2 = [1.5, 2.59, 1.6]
     *      - s3 = [1.5, -2.59, 1.6]
     */
    {
        const std::string varName  = "EmitterPosition";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("E");
        dimNames.push_back("C");
        dimNames.push_back("I");    // same emitter position for each receiver position, no need for M
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "cartesian" );
        var.putAtt( "Units", "meter" );
        
        const double emitterPosition[numEmitters*3] = {
            6,      0,      1.6,    \
            1.5,    2.59,   1.6,    \
            1.5,   -2.59,   1.6
        };
        
        var.putVar((const double*)emitterPosition);
    }
    
    /// EmitterUp
    {
        const std::string varName  = "EmitterUp";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("E");
        dimNames.push_back("C");
        dimNames.push_back("M");
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "cartesian" );
        var.putAtt( "Units", "meter" );
        
        /* values: [0 0 1] for each speaker */
        double* emitterUp = (double*)calloc(numEmitters*3*numMeasurements,
                                            sizeof(double));
        for (size_t i=0; i<(numEmitters*numMeasurements); i++)
        {
            emitterUp[(i*3)+2] = 1;
        }
        var.putVar((const double*)emitterUp);
        free(emitterUp);
    }
    
    /// EmitterView
    {
        const std::string varName  = "EmitterView";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("E");
        dimNames.push_back("C");
        dimNames.push_back("M");
        
        
        const netCDF::NcVar var = theFile.addVar( varName, typeName, dimNames );
        
        var.putAtt( "Type", "cartesian" );
        var.putAtt( "Units", "meter" );
        
        /* values: [1 0 0] for each measurement position */
        double* emitterView = (double*)calloc(numEmitters*3*numMeasurements,
                                              sizeof(double));
        for (size_t i=0; i<(numEmitters*numMeasurements); i++)
        {
            emitterView[i*3] = 1;
        }
        var.putVar((const double*)emitterView);
        free(emitterView);
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
        
        // M * R * E * N
        uint64_t total_num_samples = numDataSamplesPerChannel * numMeasurements * numEmitters * numReceivers;
        
        float* audiodata = (float*)calloc(total_num_samples,sizeof(float));
        uint64_t total_num_samples_read = 0; // accumulated though all files
        std::string audio_file_name;
        
        for (size_t s=0; s<E; s++)
        {
            for (size_t r=0; r<M; r++)
            {
                /* build file name */
                audio_file_name = input_folder_path + "/" + filename_base;
                audio_file_name += filename_source_id + std::to_string(s+1);
                audio_file_name += filename_receiver_id + std::to_string(r+1);
                audio_file_name += filename_base_end;
                
                printf("Processing %s...\n",audio_file_name.c_str());
                
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
        
        /* attributes */
        var.putAtt( "ChannelOrdering", "fuma" );
        var.putAtt( "Normalization", "fuma" );
        
        /* actual data */
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

    
    
    
    
