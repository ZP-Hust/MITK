MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkAlgorithmsExt MitkSceneSerializationBase MitkDICOMReader
  PACKAGE_DEPENDS PRIVATE ITK|ITKQuadEdgeMesh+ITKAntiAlias+ITKIONRRD
)

add_subdirectory(autoload/IO)
if(BUILD_TESTING)
 add_subdirectory(Testing)
endif()
