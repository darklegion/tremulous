include ( FindCURL )
add_library( RESTCLIENT 
             connection.cc
             connection.h
             helpers.cc
             helpers.h
             restclient.cc
             restclient.h
             version.h )
target_link_libraries( RESTCLIENT ${CURL_LIBRARIES} )
