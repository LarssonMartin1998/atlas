if(NOT DEFINED ATLAS_MODULES AND NOT BUILDING_ONLY_ATLAS)
  message(STATUS "No modules specified, skipping module setup")
  return()
endif()

set(AVAILABLE_ATLAS_MODULES Hephaestus)

set(EMODULES_ENTRIES "")
set(INCLUDE_EMODULES "")
set(MODULE_TRAITS "")
set(INCLUDE_MODULES "")
set(MODULES_CREATION "")

if(BUILDING_ONLY_ATLAS)
  set(ATLAS_MODULES ${AVAILABLE_ATLAS_MODULES})
endif()

function(to_camel_case input output_var)
  string(TOLOWER "${input}" lower_input)

  # Extract the first character and capitalize it
  string(SUBSTRING "${lower_input}" 0 1 first_char)
  string(TOUPPER "${first_char}" first_char_upper)

  # Combine the capitalized first character with the rest of the string
  string(SUBSTRING "${lower_input}" 1 -1 rest)
  set("${output_var}"
      "${first_char_upper}${rest}"
      PARENT_SCOPE)
endfunction()

function(add_module_to_project module)
  string(TOUPPER "${module}" module_upper)
  string(TOLOWER "${module}" module_lower)

  list(FIND ATLAS_MODULES_UPPER "${module_upper}" module_index)
  if(module_index GREATER -1)
    message(STATUS "Adding module: ${module}")
    target_compile_definitions(atlas PUBLIC "${module_upper}")
    add_subdirectory("modules/${module_lower}")
  else()
    message(STATUS "Skipping module: ${module}")
  endif()
endfunction()

function(setup_module_code_generation module)
  string(TOLOWER "${module}" module_lower)
  to_camel_case("${module}" module_camel_case)

  set(EMODULES_ENTRIES
      "${EMODULES_ENTRIES} ${module},\n"
      PARENT_SCOPE)
  set(INCLUDE_EMODULES
      "#include \"EModules.hpp\""
      PARENT_SCOPE)

  set(MODULE_TRAITS
      "${MODULE_TRAITS}
namespace ${module_lower} {
class ${module_camel_case};
}

namespace core {
template <> struct ModuleTraits<${module_lower}::${module_camel_case}> {
    static constexpr EModules module_enum = EModules::${module_camel_case};
};
}"
      PARENT_SCOPE)

  set(INCLUDE_MODULES
      "${INCLUDE_MODULES}
#include \"${module_camel_case}.hpp\"\n"
      PARENT_SCOPE)

  set(MODULES_CREATION
      "${MODULES_CREATION}
    create_module<${module_lower}::${module_camel_case}>(engine, modules, ticking_modules);\n"
      PARENT_SCOPE)

endfunction()

set(ATLAS_MODULES_UPPER)
foreach(module IN LISTS ATLAS_MODULES)
  string(TOUPPER "${module}" module_upper)
  list(APPEND ATLAS_MODULES_UPPER "${module_upper}")
endforeach()

foreach(module IN LISTS AVAILABLE_ATLAS_MODULES)
  add_module_to_project("${module}")
  setup_module_code_generation("${module}")
endforeach()

# The base paths will always be: generated/template/ and
# ${CMAKE_CURRENT_SOURCE_DIR}/generated/
set(CONFIGURE_PATHS
    "EModules.hpp.in:include/atlas/core/EModules.hpp"
    "ModuleTraits.hpp.in:include/atlas/core/ModuleTraits.hpp"
    "ModulesFactory.hpp.in:include/atlas/core/ModulesFactory.hpp")

file(MAKE_DIRECTORY generated/atlas/core)
foreach(paths IN LISTS CONFIGURE_PATHS)
  string(REPLACE ":" ";" path_pair ${paths})
  list(GET path_pair 0 template_path)
  list(GET path_pair 1 output_path)

  configure_file("generated/template/${template_path}"
                 "${CMAKE_CURRENT_SOURCE_DIR}/generated/${output_path}" @ONLY)
endforeach()

add_subdirectory("generated")
