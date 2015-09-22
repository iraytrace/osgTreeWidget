include(BundleUtilities)
set(BUNDLE_DIRS
  "${CMAKE_INSTALL_PREFIX}"
  "${CMAKE_INSTALL_PREFIX}/bin"
  "${CMAKE_INSTALL_PREFIX}/lib"
  "${BIN_DIR}"
  "${BIN_DIR}/bin"
  "${BIN_DIR}/bin/${CMAKE_BUILD_TYPE}"
  "${BIN_DIR}/lib"
  "${BIN_DIR}/lib/${CMAKE_BUILD_TYPE}"
"${GDAL_LIBDIR}/../bin"
"C:/Program Files/OpenSceneGraph/bin"
"C:/Program Files/OpenSceneGraph/lib"
  "${Qt5_DIR}/../../"
  "${Qt5_DIR}/../../bin"
  "${Qt5_DIR}/../../../bin"
  "${Qt5_DIR}/../../lib"
  "${Qt5_DIR}/../../../lib"
)

fixup_bundle("${APP}" "" "${BUNDLE_DIRS}")

