include_directories( AFTER /usr/local/Cellar/openssl/1.0.2e/include ../libs ${MYSQL_INCLUDE_DIR})
link_directories( /usr/local/Cellar/openssl/1.0.2e/lib)

add_executable ( knxbackbone knxbackbone.c eiblog.c knxlog.c)
add_executable ( knxbbi knxbbi.c eiblog.c knxlog.c)
add_executable ( knxtpbridge knxtpbridge.c eiblog.c knxlog.c)
add_executable ( knxipbridge knxipbridge.c eiblog.c knxlog.c)
add_executable ( knxwebbridge knxwebbridge.c eiblog.c knxlog.c)

add_executable ( knxmon knxmon.c eiblog.c knxlog.c myxml.c)
add_executable ( knxtrace knxtrace.c eiblog.c knxlog.c)
add_executable ( knxprogadr knxprogadr.c eiblog.c knxlog.c)
add_executable ( knxsim knxsim.c eiblog.c knxlog.c)

add_executable ( hdlsolar hdlsolar.c eiblog.c knxlog.c)
add_executable ( hdlpellet hdlpellet.c eiblog.c knxlog.c)
add_executable ( hdlheatpump hdlheatpump.c eiblog.c knxlog.c)
add_executable ( hdlheating hdlheating.c eiblog.c knxlog.c)
add_executable ( hdltimer hdltimer.c eiblog.c knxlog.c)

add_executable ( sendb1 sendb1.c eiblog.c knxlog.c)
add_executable ( sendu16 sendu16.c eiblog.c knxlog.c)
add_executable ( sendf16 sendf16.c eiblog.c knxlog.c)
add_executable ( sendtime sendtime.c eiblog.c knxlog.c)

target_link_libraries ( knxbackbone LINK_PUBLIC m eibknxlib ${MYSQL_LIBRARIES})
target_link_libraries ( knxbbi LINK_PUBLIC m eibknxlib ${MYSQL_LIBRARIES})
target_link_libraries ( knxtpbridge LINK_PUBLIC m eibknxlib ${MYSQL_LIBRARIES})
target_link_libraries ( knxipbridge LINK_PUBLIC m eibknxlib)
target_link_libraries ( knxwebbridge LINK_PUBLIC m crypto eibknxlib)

target_link_libraries ( knxmon LINK_PUBLIC m xml2 eibknxlib ${MYSQL_LIBRARIES})
target_link_libraries ( knxtrace LINK_PUBLIC m eibknxlib ${MYSQL_LIBRARIES})
target_link_libraries ( knxprogadr LINK_PUBLIC m eibknxlib)
target_link_libraries ( knxsim LINK_PUBLIC m eibknxlib)

target_link_libraries ( hdlsolar LINK_PUBLIC m eibknxlib)
target_link_libraries ( hdlpellet LINK_PUBLIC m eibknxlib)
target_link_libraries ( hdlheatpump LINK_PUBLIC m eibknxlib)
target_link_libraries ( hdlheating LINK_PUBLIC m eibknxlib)
target_link_libraries ( hdltimer LINK_PUBLIC m eibknxlib)

target_link_libraries ( sendb1 LINK_PUBLIC m eibknxlib)
target_link_libraries ( sendu16 LINK_PUBLIC m eibknxlib)
target_link_libraries ( sendf16 LINK_PUBLIC m eibknxlib)
target_link_libraries ( sendtime LINK_PUBLIC m eibknxlib)

install( TARGETS knxbackbone RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
install( TARGETS knxbbi RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
install( TARGETS knxtpbridge RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
install( TARGETS knxipbridge RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
install( TARGETS knxwebbridge RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
install( TARGETS knxmon RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
install( TARGETS knxtrace RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
install( TARGETS hdlpellet RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
install( TARGETS hdlheatpump RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
install( TARGETS hdlheating RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
install( TARGETS hdlsolar RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
install( TARGETS hdltimer RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
install( TARGETS sendb1 RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
install( TARGETS sendf16 RUNTIME DESTINATION /usr/bin LIBRARY DESTINATION /usr/lib PERMISSIONS SETUID)
