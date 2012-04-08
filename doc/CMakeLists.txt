
# openPablo doc CMakeLists.txt

# --- cmake settings
#
  cmake_minimum_required (VERSION 2.8)


# ---
  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

  message("CMAKE ROOT is ${CMAKE_ROOT}")
  message("CMAKE MODULE PATH is ${CMAKE_MODULE_PATH}")
  include(${CMAKE_MODULE_PATH}/UseLATEX.cmake)

  set(PDFLATEX_COMPILER xelatex)

  set(INPUTS_TEX_FILES
    fonts/Nunito-Light.ttf
    fonts/Nunito-Regular.ttf
  )

#  set(BIB_FILE
#      data/main.bib
#  )




#
# ----------------------------------------------------------------------------
# add a target to generate manual with xelatex (if requested)
# ----------------------------------------------------------------------------#
# 

    if(compile_docs)
      ADD_LATEX_DOCUMENT(manual.ltx
	INPUTS ${INPUTS_TEX_FILES}
	IMAGE_DIRS images
	BIBFILES ${BIB_FILE}
	DEFAULT_PDF
      )
    endif(compile_docs)

