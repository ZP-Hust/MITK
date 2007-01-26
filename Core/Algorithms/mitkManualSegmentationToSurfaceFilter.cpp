#include <mitkManualSegmentationToSurfaceFilter.h>

mitk::ManualSegmentationToSurfaceFilter::ManualSegmentationToSurfaceFilter() 
{
  m_MedianFilter3D = false;
  m_MedianKernelSizeX = 3;
  m_MedianKernelSizeY = 3;
  m_MedianKernelSizeZ = 3;
  m_UseGaussianImageSmooth = false;
  m_GaussianStandardDeviation = 1.5;
  m_Interpolation = false;
  m_InterpolationX = 1.0f;
  m_InterpolationY = 1.0f;
  m_InterpolationZ = 1.0f;
};

mitk::ManualSegmentationToSurfaceFilter::~ManualSegmentationToSurfaceFilter(){};
void mitk::ManualSegmentationToSurfaceFilter::GenerateData() 
{
  mitk::Surface *surface = this->GetOutput();
  mitk::Image * image    =  (mitk::Image*)GetInput();
  mitk::Image::RegionType outputRegion = image->GetRequestedRegion();
  int tstart=outputRegion.GetIndex(3);
  int tmax=tstart+outputRegion.GetSize(3); //GetSize()==1 

  ScalarType thresholdExpanded = this->m_Threshold;
  for( int t=tstart; t<tmax; t++ )
  {
    vtkImageData *vtkimage = image->GetVtkImageData(t);
    //Inkrement Referenzcounter Counter (hier: RC)
    vtkimage->Register(NULL);
    // Median -->smooth 3D 
    if(m_MedianFilter3D)
    {
      vtkImageMedian3D *median = vtkImageMedian3D::New();
      median->SetInput(vtkimage); //RC++ (VTK < 5.0)
      median->SetKernelSize(m_MedianKernelSizeX,m_MedianKernelSizeY,m_MedianKernelSizeZ);//Std: 3x3x3
      median->ReleaseDataFlagOn();
      median->UpdateInformation();
      median->Update();
      vtkimage->Delete(); //->Input
      vtkimage = median->GetOutput(); //->Out
      vtkimage->Register(NULL);
      median->Delete();
    }
    //Interpolate image spacing 
    if(m_Interpolation)
    {
      vtkImageResample * imageresample = vtkImageResample::New();
      imageresample->SetInput(vtkimage);
      //Set Spacing Manual to 1mm in each direction (Original spacing is lost during image processing)      
      imageresample->SetAxisOutputSpacing(0, m_MedianKernelSizeX);
      imageresample->SetAxisOutputSpacing(1, m_MedianKernelSizeY);
      imageresample->SetAxisOutputSpacing(2, m_MedianKernelSizeZ);
      imageresample->UpdateInformation();
      imageresample->Update();
      vtkimage->Delete(); //->Input
      vtkimage=imageresample->GetOutput();//->Output
      vtkimage->Register(NULL);
      imageresample->Delete();
    }
    if(m_UseGaussianImageSmooth)//gauss
    {
      vtkImageThreshold* vtkimagethreshold = vtkImageThreshold::New();
      vtkimagethreshold->SetInput(vtkimage);
      vtkimagethreshold->SetInValue( 100 );
      vtkimagethreshold->SetOutValue( 0 );
      vtkimagethreshold->ThresholdByUpper( this->m_Threshold ); 
      thresholdExpanded = 49;
      vtkimagethreshold->SetOutputScalarTypeToUnsignedChar();
      vtkimagethreshold->ReleaseDataFlagOn();
      vtkImageGaussianSmooth *gaussian = vtkImageGaussianSmooth::New();
      gaussian->SetInput(vtkimagethreshold->GetOutput()); 
      gaussian->SetDimensionality(3);
      gaussian->SetRadiusFactor(0.49);
      gaussian->SetStandardDeviation( m_GaussianStandardDeviation );
      gaussian->ReleaseDataFlagOn();
      gaussian->UpdateInformation();
      gaussian->Update();
      vtkimage->Delete();//->Input
      vtkimage = gaussian->GetOutput(); //->Out
      gaussian->Register(NULL);
      gaussian->Delete();
    }
    // Create sureface for t-Slice
    CreateSurface(t, vtkimage, surface, thresholdExpanded);
  }
};

void mitk::ManualSegmentationToSurfaceFilter::SetMedianKernelSize(int x, int y, int z)
{
  m_MedianKernelSizeX = x;
  m_MedianKernelSizeY = y;
  m_MedianKernelSizeZ = z;
 }
void mitk::ManualSegmentationToSurfaceFilter::SetInterpolation(vtkDouble x, vtkDouble y, vtkDouble z)
{
  m_InterpolationX = x;
  m_InterpolationY = y;
  m_InterpolationZ = z;
} 
