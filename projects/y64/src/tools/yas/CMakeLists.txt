add_executable(yas yas.cpp)

if (WIN32)
  target_link_libraries(yas
    y64
  )
else (WIN32)
  target_link_libraries(yas
    y64
    stdc++fs
  )
endif (WIN32)