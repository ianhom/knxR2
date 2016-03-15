set( lib_SRCS eib rs232 debug mylib)
add_library( eibknxlib SHARED ${lib_SRCS})

install( TARGETS eibknxlib DESTINATION /usr/lib)
