target_sources(passman PRIVATE
    blake.cc
    chacha.cc
    cli.cc
    db.cc
    main.cc
    random.cc)

#if(PASSMAN_BUILD_GUI)
#    qt5_wrap_cpp(MOC_SOURCES
#        window.hh)
#    target_sources(passman PRIVATE
#        ${MOC_SOURCES}
#        window.cc)
#endif()
