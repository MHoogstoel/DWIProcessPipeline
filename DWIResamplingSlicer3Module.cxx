#include <iostream>
#include <stdlib.h>
#include <bmScriptParser.h>

#include <algorithm>
#include "DWIResamplingSlicer3ModuleCLP.h"
#include <fstream>
#include <itkTransformFileReader.h>
#include <itksys/SystemTools.hxx>
#include <itksys/Process.h>
#include "SlicerBatchMakeConfig.h"
#include <sys/param.h>

std::string SetOptionalString( std::string var ,
                     std::string BATCHNAME
                   )
{
  std::string script ;
  if( var.compare("") )
  {
    script += "Set( " + BATCHNAME + " " + var + " )\n" ;
  }
  else
  {
    script += "Set( " + BATCHNAME + " \'\' )\n" ;
  }
  return script ;
}

std::string SetBOOL( bool boolVar ,
                     std::string BATCHNAME
                   )
{
  std::string script ;
  if( boolVar )
  {
    script += "Set( " + BATCHNAME + " TRUE )\n" ;
  }
  else
  {
    script += "Set( " + BATCHNAME + " FALSE )\n" ;
  }
  return script ;
}

std::string RemoveExtension( std::string data , std::string &ext )
{
  int found = data.rfind( "." ) ;
  ext = data.substr( found + 1 , data.size() - found - 1 ) ;
  data.resize( found ) ;
  if( !ext.compare( "gz" ) )
  {
     found = data.rfind( "." ) ;
     ext = data.substr( found + 1 , data.size() - found - 1 ) ;
     ext = ext + ".gz" ;
     data.resize( found ) ;
  }
  return data ;
}

std::string GetDirectory( std::string &dwi )
{
  size_t found = dwi.rfind('/') ;
  if( found != std::string::npos )
  {
    itksys_stl::string dir1 = dwi.substr( 0 , found ) ;
    dwi.erase( dwi.begin() , dwi.begin() + found + 1 ) ;
    return dir1 ;
  }
  return "." ;
}

int SetPath( std::string &pathString , const char* name )
{
  if( !pathString.substr(pathString.size() - 10 , 9 ).compare( "-NOTFOUND" ) )
  {
    pathString= itksys::SystemTools::FindProgram( name ) ;
    if( !pathString.compare( "" ) )
    {
      std::cerr << name << " is missing or its PATH is not set" << std::endl ;
      return -1 ;
    }
  }
  return 0 ;
}

std::vector< std::string > SplitPath( std::string path )
{
   size_t found = 0 ;
   bool isDir = itksys::SystemTools::FileIsDirectory( path.c_str() ) ;
   std::vector< std::string > split ;
   do
   {
     std::cout<<path<<std::endl;
     found = path.find('/') ;
     if( found != std::string::npos )
     {
       std::string dir = path.substr( 0 , found ) ;
       if( dir.compare( "." ) )
       {
          std::cout<<dir<<std::endl;
          split.push_back( dir ) ;
       }
       path.erase( path.begin() , path.begin() + found + 1 ) ;
     }
   }
   while( found != std::string::npos ) ;
   if( isDir )
   {
      split.push_back( path ) ;
   }
   return split ;
}

std::string RelativePath( std::string ReferencePath , std::string path )
{
   std::string rPath ;
   std::vector< string > rsplitPath = SplitPath( ReferencePath ) ;
   std::vector< string > splitPath = SplitPath( path ) ;
   unsigned int count = 0 ;
   for( unsigned int i = 0 ; i < std::min( rsplitPath.size() , splitPath.size() ) ; i++ )
   {
      if( !rsplitPath[ i ].compare( splitPath[ i ] ) )
      {
         count++ ;
      }
   }
   for( unsigned int i = count ; i < rsplitPath.size() ; i++ )
   {
      rPath += "../" ;
   }
   for( unsigned int i = count ; i < splitPath.size() ; i++ )
   {
      rPath += splitPath[ i ] + "/" ;
   }
   if( !rPath.compare( "" ) )
   {
      rPath = "." ;
   }
   else
   {
      rPath.resize( rPath.size() - 1 ) ;
   }
   return rPath ;
}

int main(int argc, char* argv[])
{
  PARSE_ARGS;
  std::string ext ;
  std::string dir ;
  data = itksys::SystemTools::ConvertToUnixOutputPath( data.c_str() ) ;
  outputDir = itksys::SystemTools::ConvertToUnixOutputPath( outputDir.c_str() ) ;
  templateFile = itksys::SystemTools::ConvertToUnixOutputPath( templateFile.c_str() ) ;
  im1 = itksys::SystemTools::ConvertToUnixOutputPath( im1.c_str() ) ;
  im2 = itksys::SystemTools::ConvertToUnixOutputPath( im2.c_str() ) ;
  im3 = itksys::SystemTools::ConvertToUnixOutputPath( im3.c_str() ) ;
  imnn1 = itksys::SystemTools::ConvertToUnixOutputPath( imnn1.c_str() ) ;
  imnn2 = itksys::SystemTools::ConvertToUnixOutputPath( imnn2.c_str() ) ;
  transformationFile = itksys::SystemTools::ConvertToUnixOutputPath( transformationFile.c_str() ) ;
  std::string outputDirForRelative ;
  if( !itksys::SystemTools::FileIsFullPath( outputDir.c_str() ) )
  {
     char path[MAXPATHLEN] ;
     getcwd(path, MAXPATHLEN) ;
     outputDirForRelative = path ;
     if( outputDirForRelative[ outputDirForRelative.size() - 1 ] != '/' )
     {
        outputDirForRelative += "/" ;
     }
     outputDirForRelative += outputDir ;
  }
  else
  {
     outputDirForRelative = outputDir ;
     if( outputDirForRelative[ outputDirForRelative.size() - 1 ] != '/' )
     {
        outputDirForRelative += "/" ;
     }
     //outputDirForRelative += "addition" ;
  }
  outputDirForRelative = itksys::SystemTools::GetRealPath( outputDirForRelative.c_str() ) ;
  std::cout<< outputDirForRelative << std::endl ;
    std::string transformRelativePath ;
  if( transformationFile.compare( "" ) )
  {
    std::string transformationFileRelative = itksys::SystemTools::GetRealPath( transformationFile.c_str() ) ;
    transformRelativePath = itksys::SystemTools::RelativePath( outputDirForRelative.c_str() , transformationFileRelative.c_str() ) ;
   std::cout<< "plop " << transformRelativePath << std::endl;
  }
  else
  {
     transformRelativePath = outputDirForRelative ;
  }
  std::cout<< "relative path1: "<<transformRelativePath << std::endl;
  std::string dataRelative = itksys::SystemTools::GetRealPath( data.c_str() ) ;
  //std::string inputRelativePath = itksys::SystemTools::RelativePath( outputDirForRelative.c_str() , dataRelative.c_str() ) ;
  //std::string templateFileRelative = itksys::SystemTools::GetRealPath( templateFile.c_str() ) ;
//  std::string templateDirForRelative = itksys::SystemTools::RelativePath( outputDirForRelative.c_str() , templateFileRelative.c_str() ) ;
//  std::cout<<templateDirForRelative<<std::endl;
  std::string templateDir = GetDirectory( templateFile ) ;
  if( outputDir[ outputDir.size() - 1 ] == '/' )
  {
     outputDir.resize( outputDir.size() - 1 ) ;
  }
  dir = GetDirectory( data ) ;
  data = RemoveExtension( data , ext ) ;
  std::string extim1 ;
  if( im1.compare("") )
  {
    im1 = RemoveExtension( im1 , extim1 ) ;
  }
  std::string extim2 ;
  if( im2.compare("") )
  {
    im2 = RemoveExtension( im2 , extim2 ) ;
  }
  std::string extim3 ;
  if( im3.compare("") )
  {
    im3 = RemoveExtension( im3 , extim3 ) ;
  }
  std::string extimnn1 ;
  if( imnn1.compare("") )
  {
    imnn1 = RemoveExtension( imnn1 , extimnn1 ) ;
  }
  std::string extimnn2 ;
  if( imnn2.compare("") )
  {
    imnn2 = RemoveExtension( imnn2 , extimnn2 ) ;
  }
  //Find tools
  //ResampleVolume2
  std::string pathResampleVolume2String= itksys::SystemTools::FindProgram("ResampleVolume2");
  if( SetPath( pathResampleVolume2String , "ResampleVolume2" ) )
  {
    return EXIT_FAILURE ;
  }
  //ResampleDTI (log euclidean and original)  
  std::string pathResampleDTIString = ResampleDTIlogEuclidean_PATH ;
  if( nologEuclidean || SetPath( pathResampleDTIString , "ResampleDTIlogEuclidean" ) )
  {
    pathResampleDTIString = ResampleDTI_PATH ;
    if( SetPath( pathResampleDTIString , "ResampleDTI" ) )
    {
      return EXIT_FAILURE ;
    }
  }
  //ComputeOrientation & ITKTransformTools
  std::string pathITKTransformToolsString = ITKTransformTools_PATH ;
  std::string pathManualImageOrientString = ComputeOrientation_PATH ;
  if( computeOrientation )
  {
    if( SetPath( pathManualImageOrientString , "ManualImageOrient" ) )
    {
      return EXIT_FAILURE ;
    }
  }
  if( computeOrientation || scale )
  {
    if( SetPath( pathITKTransformToolsString , "ITKTransformTools" ) )
    {
      return EXIT_FAILURE ;
    }
  }
  //Compute mask
  std::string pathMaskComputationString = MaskComputation_PATH ;
  if( skullStrip )
  {
    if( SetPath( pathMaskComputationString , "MaskComputationWithThresholding" ) )
    {
      return EXIT_FAILURE ;
    }
  }
  //Register Images
  std::string pathRegisterImagesString = RegisterImages_PATH ;
  if( SetPath( pathRegisterImagesString , "RegisterImages" ) )
  {
    return EXIT_FAILURE ;
  }
  //dtiestim
  std::string pathdtiestimString = dtiestim_PATH ;
  if( SetPath( pathdtiestimString , "dtiestim" ) )
  {
    return EXIT_FAILURE ;
  }
  //dtiprocess
  std::string pathdtiprocessString = dtiprocess_PATH ;
  if( SetPath( pathdtiprocessString , "dtiprocess" ) )
  {
    return EXIT_FAILURE ;
  }
  //DiffusionTensorEstimation
  std::string pathDiffusionTensorEstimationString = DiffusionTensorEstimation_PATH ;
  if( SetPath( pathDiffusionTensorEstimationString , "DiffusionTensorEstimation" ) )
  {
    return EXIT_FAILURE ;
  }
  //ImageMath
  std::string pathImageMathString = ImageMath_PATH ;
  if( SetPath( pathImageMathString , "ImageMath" ) )
  {
    return EXIT_FAILURE ;
  }
  //CreateMRML
  std::string pathCreateMRMLString = CreateMRML_PATH ;
  if( SetPath( pathCreateMRMLString , "CreateMRML" ) )
  {
     return EXIT_FAILURE ;
  }
  //HistogramMatching
  std::string pathHistogramMatchingString = HistogramMatching_PATH ;
  if( SetPath( pathHistogramMatchingString , "HistogramMatching" ) )
  {
     return EXIT_FAILURE ;
  }
  //get batchmake script directory
  std::string script = "echo('Starting BatchMake Script')\n" ;
////Set options/////////////////////
  script += SetOptionalString( BatchMake_WRAPPED_APPLICATION_DIR , "SCRIPTDIR" ) ;
  script += SetOptionalString( pathITKTransformToolsString , "ITKTransformToolsPATH" ) ;
  script += SetOptionalString( pathImageMathString , "ImageMathPATH" ) ;
  script += SetOptionalString( pathdtiestimString , "dtiestimPATH" ) ;
  script += SetOptionalString( pathManualImageOrientString , "ManualImageOrientPATH" ) ;
  script += SetOptionalString( pathdtiprocessString , "dtiprocessPATH" ) ;
  script += SetOptionalString( pathRegisterImagesString , "RegisterImagesPATH" ) ;
  script += SetOptionalString( pathResampleDTIString , "ResampleDTIPATH" ) ;
  script += SetOptionalString( pathResampleVolume2String , "ResampleVolume2PATH" ) ;
  script += SetOptionalString( pathDiffusionTensorEstimationString , "DiffusionTensorEstimationPATH" ) ;
  script += SetOptionalString( pathHistogramMatchingString , "HistogramMatchingPATH" ) ;
  script += SetOptionalString( pathCreateMRMLString , "CreateMRMLPATH" ) ;
  script += SetOptionalString( pathMaskComputationString , "MaskComputationPATH" ) ;
  std::cout<< "relative path: "<<transformRelativePath << std::endl;
  script += SetOptionalString( transformRelativePath , "TransformRelativePATH" ) ;
  script += SetOptionalString( initialTransform , "InitialTransform" ) ;
//  script += SetOptionalString( inputRelativePath , "InputRelativePATH" ) ;
//  script += SetOptionalString( templateDirForRelative , "atlasRelativePATH" ) ;
  script += "Set( INPUTDIR " + dir + " )\n" ;
  script += "Set( INPUT " + data + " )\n" ;
  script += "Set( EXT " + ext + " )\n" ;
  script += "Set( TEMPLATE " + templateFile + " )\n" ;
  script += "Set( TEMPLATEDIR " + templateDir + " )\n" ;
  script += "Set( INPUTTYPE " + inputType + " )\n" ;
  script += "Set( OUTPUTDIR " + outputDir + " )\n" ;
  script += SetOptionalString( im1 , "IM1" ) ;
  script += SetOptionalString( extim1 , "EXT_IM1" ) ;
  script += SetOptionalString( im2 , "IM2" ) ;
  script += SetOptionalString( extim2 , "EXT_IM2" ) ;
  script += SetOptionalString( im3 , "IM3" ) ;
  script += SetOptionalString( extim3 , "EXT_IM3" ) ;
  script += SetOptionalString( imnn1 , "IMNN1" ) ;
  script += SetOptionalString( extimnn1 , "EXT_IMNN1" ) ;
  script += SetOptionalString( imnn2 , "IMNN2" ) ;
  script += SetOptionalString( extimnn2 , "EXT_IMNN2" ) ;
  script += SetBOOL( createB0 , "CREATEB0" ) ;
  script += SetBOOL( createIDWI , "CREATEIDWI" ) ;
  script += SetBOOL( computeOrientation , "COMPUTEORIENTATION" ) ;
  script += "Set( MANUALORIENTATION \'" + manualOrientation + "\' )\n" ;
  script += SetOptionalString( transformationFile , "TRANSFORMATIONFILE" ) ;
  script += SetBOOL( computeFA , "COMPUTEFA" ) ;
  script += SetBOOL( computeMD , "COMPUTEMD" ) ;
  script += SetBOOL( computeColorFA , "COMPUTECOLORFA" ) ;
  script += SetBOOL( computeRadial , "COMPUTERD" ) ;
  script += SetBOOL( computeAxial , "COMPUTEAD" ) ;
  script += SetBOOL( skullStrip , "SKULLSTRIP" ) ;
  script += SetBOOL( copyInputs , "COPY_INPUTS" ) ;
  script += SetBOOL( scale , "SCALE" ) ;
  script += SetOptionalString( rootName , "ROOTNAME" ) ;
  script += "Set( REGTYPE \'" + registrationType + "\' )\n" ;
  script += "Set( INTERPOLATION \'" + interpolationType + "\' )\n" ;
  script += "include( " ;
  script += BatchMake_WRAPPED_APPLICATION_DIR ;
  script += "/BatchMakeScript.bms )\n" ;
/////////////////////////////////////////////  
  
    // Write script to disk to execute the batch
  // -------------------------
  itksys::SystemTools::MakeDirectory( outputDir.c_str() ) ;
  std::string scriptFile = outputDir
                           + "/DTIPipeline.bms";
  std::cout<<scriptFile<<std::endl;
  std::ofstream file( scriptFile.c_str() );
  file << script; 
  file.close();
  

  
  // Create a progress manager gui
  // -------------------------
  bm::ScriptParser batchMakeParser;

  // If we want to run the script locally

  if( !runChoice.compare("Run") )
  {
    batchMakeParser.ParseBuffer( script ) ;
  }
  else if( !runChoice.compare("Condor") )
  {
    batchMakeParser.RunCondor( script ) ;
  }

  return EXIT_SUCCESS ;
}
