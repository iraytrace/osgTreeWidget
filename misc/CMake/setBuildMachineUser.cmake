#
# These provide compile time macro definitions that allow 
# information to be provided in "About" dialog boxes
#
if (WIN32)
set(BUILT_BY_USER "$ENV{USERNAME}")
else()
set(BUILT_BY_USER "$ENV{USER}" )
endif()
add_definitions( -DBUILT_BY_USER="${BUILT_BY_USER}" )

SITE_NAME(BUILT_ON_MACHINE)
add_definitions( -DBUILT_ON_MACHINE="${BUILT_ON_MACHINE}" )

