FILE(REMOVE_RECURSE
  "CMakeFiles/example.dir/ex1simple.cc.o"
  "example2.pdb"
  "example2"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang CXX)
  INCLUDE(CMakeFiles/example.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
