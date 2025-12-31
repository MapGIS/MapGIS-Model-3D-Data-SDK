cmake_minimum_required(VERSION 3.1)

set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE} CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_STANDARD 11)

set(PROJECT_NAME m3d)
project(${PROJECT_NAME} )


set(MYLIB_SOURCES

        include/g3dtile.h
        include/g3dtilecontent.h
        include/g3dtiledefine.h
        include/g3dtilegeometry.h
        include/g3dtilelayer.h
        include/g3dtilem3dimpl.h
        include/g3dtilerecord.h
        include/g3dtilespatial.h
        include/g3dtilestorage.h
        include/g3dtilestructuretree.h
	include/g3dtilegaussian.h

        src/ci_3dmodel.h
        src/ci_3dmodel_3dtile.cpp
        src/ci_3dmodel_3dtile.h
        src/ci_3dmodel_attribute_att.cpp
        src/ci_3dmodel_attribute_att.h
        src/ci_3dmodel_attribute_json.cpp
        src/ci_3dmodel_attribute_json.h
        src/ci_3dmodel_attribute_sqlite.cpp
        src/ci_3dmodel_attribute_sqlite.h
        src/ci_3dmodel_attribute_sqlite_tool.cpp
        src/ci_3dmodel_attribute_sqlite_tool.h
        src/ci_3dmodel_b3dm.cpp
        src/ci_3dmodel_cmpt.cpp
        src/ci_3dmodel_gltf.cpp
        src/ci_3dmodel_i3dm.cpp
        src/ci_3dmodel_pnts.cpp
        src/ci_3dmodel_tid.cpp
        src/ci_3dmodel_tid.h
        src/ci_assist.cpp
        src/ci_assist.h
        src/ci_cache_storage.cpp
        src/ci_cache_storage.h
        src/ci_cache_tile_m3d_impl.cpp
        src/ci_cache_tile_m3d_impl.h
        src/ci_dracotool.cpp
        src/ci_dracotool.h
        src/ci_gltfextensions.cpp
        src/ci_gltfextensions.h
        src/ci_gltfsdktool.cpp
        src/ci_gltfsdktool.h
        src/ci_layer.cpp
        src/ci_m3d_tile.cpp
        src/ci_m3d_tile.h
        src/ci_meshopttool.cpp
        src/ci_meshopttool.h
        src/ci_record.cpp
        src/ci_spatial.cpp
        src/ci_structure_tree.cpp
        src/ci_structure_tree.h
        src/ci_tile.cpp
        src/ci_ziptool.cpp
        src/ci_ziptool.h
        src/ci_g3dtilecontent.cpp
        src/mini_zip.cpp
        src/mini_zip.h
        src/miniz.h
        src/stdafx.cpp
        src/stdafx.h
	src/load-spz.cpp
	src/load-spz.h
	src/splat-types.cpp
	src/splat-types.h
	src/tinyply.cpp
	src/tinyply.h
	src/ci_gaussian_ksplat_read_write.cpp
	src/ci_gaussian_ksplat_read_write.h
	src/ci_gaussian_ply_read_write.cpp
	src/ci_gaussian_ply_read_write.h
	src/ci_gaussian_read_write.cpp
	src/ci_gaussian_sog_read_write.cpp
	src/ci_gaussian_sog_read_write.h
	src/ci_gaussian_splat_read_write.cpp
	src/ci_gaussian_splat_read_write.h
	src/ci_gaussian_spz_read_write.cpp
	src/ci_gaussian_spz_read_write.h
	src/ci_gaussian_trans.cpp
)

if(EXISTS "${CMAKE_SOURCE_DIR}/src/ci_3dtiles_tile.cpp")
    list(APPEND MYLIB_SOURCES
    src/ci_3dtiles_tile.cpp
    src/ci_3dtiles_tile.h
    src/ci_cache_tile_3dtiles_impl.cpp
    src/ci_cache_tile_3dtiles_impl.h)
endif()

add_library(${PROJECT_NAME}  SHARED
       ${MYLIB_SOURCES}
)

source_group("3rd/draco" FILES
    src/ci_dracotool.h
    src/ci_dracotool.cpp
)
source_group("3rd/gltfsdk" FILES
    src/ci_gltfextensions.h
    src/ci_gltfextensions.cpp
    src/ci_gltfsdktool.h
    src/ci_gltfsdktool.cpp
)

source_group("3rd/meshopt" FILES
    src/ci_meshopttool.h
    src/ci_meshopttool.cpp
)
source_group("3rd/miniz" FILES
    src/ci_ziptool.h
    src/ci_ziptool.cpp
    src/mini_zip.h
    src/mini_zip.cpp
    src/miniz.h
)

source_group("assist" FILES
    src/ci_assist.h
    src/ci_assist.cpp
    src/stdafx.h
    src/stdafx.cpp
)

source_group("export" FILES
    include/g3dtile.h
    include/g3dtilecontent.h
    include/g3dtiledefine.h
    include/g3dtilegeometry.h
    include/g3dtilelayer.h
    include/g3dtilem3dimpl.h
    include/g3dtilerecord.h
    include/g3dtilespatial.h
    include/g3dtilestorage.h
    include/g3dtilestructuretree.h
    include/g3dtilegaussian.h
)


source_group("tile" FILES
    src/ci_tile.cpp
)

source_group("tile/3dmodel" FILES
    src/ci_3dmodel.h
    src/ci_3dmodel_3dtile.cpp
    src/ci_3dmodel_3dtile.h
    src/ci_3dmodel_b3dm.cpp
    src/ci_3dmodel_cmpt.cpp
    src/ci_3dmodel_gltf.cpp
    src/ci_3dmodel_i3dm.cpp
    src/ci_3dmodel_pnts.cpp
)

if(EXISTS "${CMAKE_SOURCE_DIR}/src/ci_3dtiles_tile.cpp")
    source_group("tile/3dtiles" FILES
        src/ci_3dtiles_tile.cpp
        src/ci_3dtiles_tile.h
        src/ci_cache_tile_3dtiles_impl.cpp
        src/ci_cache_tile_3dtiles_impl.h
    )
endif()

source_group("tile/storage" FILES
    src/ci_cache_storage.cpp
    src/ci_cache_storage.h
)
source_group("tile/structure_tree" FILES
    src/ci_structure_tree.cpp
    src/ci_structure_tree.h
)

source_group("tile/record" FILES
    src/ci_record.cpp
)

source_group("tile/layer" FILES
    src/ci_layer.cpp
)

source_group("tile/spatial" FILES
    src/ci_spatial.cpp
)
source_group("tile/content" FILES
    src/ci_g3dtilecontent.cpp
)

source_group("tile/m3d" FILES
    src/ci_cache_tile_m3d_impl.h
    src/ci_cache_tile_m3d_impl.cpp
    src/ci_m3d_tile.cpp
    src/ci_m3d_tile.h
)
source_group("tile/m3d/attribute" FILES
    src/ci_3dmodel_attribute_att.h
    src/ci_3dmodel_attribute_att.cpp
    src/ci_3dmodel_attribute_json.h
    src/ci_3dmodel_attribute_json.cpp
    src/ci_3dmodel_attribute_sqlite.cpp
    src/ci_3dmodel_attribute_sqlite.h
    src/ci_3dmodel_attribute_sqlite_tool.cpp
    src/ci_3dmodel_attribute_sqlite_tool.h
)
source_group("tile/m3d/tid" FILES
    src/ci_3dmodel_tid.h
    src/ci_3dmodel_tid.cpp
)
source_group("gaussian/ply" FILES
    src/tinyply.h
    src/tinyply.cpp
)
source_group("gaussian/spz" FILES
    src/load-spz.h
    src/load-spz.cpp
	src/splat-types.h
    src/splat-types.cpp
)
source_group("gaussian/trans" FILES
    src/ci_gaussian_trans.cpp
)

source_group("gaussian/readwrite" FILES
    src/ci_gaussian_trans.cpp
    src/ci_gaussian_ksplat_read_write.cpp
    src/ci_gaussian_ksplat_read_write.h
    src/ci_gaussian_ply_read_write.cpp
    src/ci_gaussian_ply_read_write.h
    src/ci_gaussian_read_write.cpp
    src/ci_gaussian_sog_read_write.cpp
    src/ci_gaussian_sog_read_write.h
    src/ci_gaussian_splat_read_write.cpp
    src/ci_gaussian_splat_read_write.h
    src/ci_gaussian_spz_read_write.cpp
    src/ci_gaussian_spz_read_write.h
)
# 添加 -fpermissive 编译选项
add_compile_options(-fpermissive)

target_compile_definitions(${PROJECT_NAME}  PRIVATE _MAPGIS_G3D_TILE_EXPORT_)
target_compile_definitions(${PROJECT_NAME}  PRIVATE NOMINMAX)
if(EXISTS "${CMAKE_SOURCE_DIR}/src/ci_3dtiles_tile.cpp")
  target_compile_definitions(${PROJECT_NAME}  PRIVATE _HAS_3D_TILES_)
endif()

if(NOT WIN32)
    target_compile_definitions(${PROJECT_NAME}  PRIVATE MAPGIS_LINUX)
endif()



if(NOT SDK_ROOT)
    set(SDK_ROOT "${CMAKE_SOURCE_DIR}/3rdparty" CACHE PATH "SDK ROOT")
endif()

# 设置第三方库路径（可通过 -DTHIRD_PARTY_ROOT=/path/to/custom 覆盖）
#set(LIBICONV_ROOT "${SDK_ROOT}/3rd/libiconv" )
set(ZLIB_ROOT "${SDK_ROOT}/3rd/zlib")

set(BOOST_ROOT "${SDK_ROOT}/boost" )
set(GLTFSDK_ROOT "${SDK_ROOT}/GLTFSDK-1.9.6.0" )
set(DRACO_ROOT "${SDK_ROOT}/draco-1.5.6" )
set(MESHOPT_ROOT "${SDK_ROOT}/meshoptimizer-0.22" )
set(SQLITE3_ROOT "${SDK_ROOT}/sqlite3" )
set(TINYGLTF_ROOT "${SDK_ROOT}/tinygltf-2.4.0" )
set(EIGEN_ROOT "${SDK_ROOT}/eigen" )
set(MAPGIS_COMMON "${SDK_ROOT}/mapgis_common")

target_include_directories(${PROJECT_NAME}  PRIVATE ${SDK_ROOT})
target_include_directories(${PROJECT_NAME}  PRIVATE ${ZLIB_ROOT}/include)
target_include_directories(${PROJECT_NAME}  PRIVATE ${BOOST_ROOT}/include)
target_include_directories(${PROJECT_NAME}  PRIVATE ${GLTFSDK_ROOT}/Include)
target_include_directories(${PROJECT_NAME}  PRIVATE ${DRACO_ROOT}/Include)
target_include_directories(${PROJECT_NAME}  PRIVATE ${MESHOPT_ROOT}/include)
target_include_directories(${PROJECT_NAME}  PRIVATE ${SQLITE3_ROOT}/include)
target_include_directories(${PROJECT_NAME}  PRIVATE ${TINYGLTF_ROOT})
target_include_directories(${PROJECT_NAME}  PRIVATE ${EIGEN_ROOT})
target_include_directories(${PROJECT_NAME}  PRIVATE ${MAPGIS_COMMON}/include)

if(WIN32)
    # 包含头文件路径
    #target_include_directories(${PROJECT_NAME}  PRIVATE ${LIBICONV_ROOT}/include)
    # 链接静态/动态库
    #target_link_libraries(${PROJECT_NAME}  PRIVATE "${LIBICONV_ROOT}/lib(x64)/libiconv.lib")
   # target_link_libraries(${PROJECT_NAME}  PRIVATE "${LIBICONV_ROOT}/lib(x64)/libiconv_dynamic.lib")
    target_link_libraries(${PROJECT_NAME}  PRIVATE "${ZLIB_ROOT}/lib(x64)/zlibstat.lib")
    target_link_libraries(${PROJECT_NAME}  PRIVATE "${SQLITE3_ROOT}/lib(x64)/sqlite3_dynamic.lib")
    target_link_libraries(${PROJECT_NAME}  PRIVATE "${DRACO_ROOT}/lib(x64)/draco.lib")
    target_link_libraries(${PROJECT_NAME}  PRIVATE "${GLTFSDK_ROOT}/lib(x64)/GLTFSDK.lib")
    target_link_libraries(${PROJECT_NAME}  PRIVATE "${MESHOPT_ROOT}/lib(x64)/gltfpack.lib")
    target_link_libraries(${PROJECT_NAME}  PRIVATE "${MESHOPT_ROOT}/lib(x64)/meshoptimizer.lib")
    target_link_libraries(${PROJECT_NAME}  PRIVATE "${BOOST_ROOT}/lib(x64)/libboost_filesystem-vc140-mt-1_64.lib")
    target_link_libraries(${PROJECT_NAME}  PRIVATE "${BOOST_ROOT}/lib(x64)/libboost_system-vc140-mt-1_64.lib")
    target_link_libraries(${PROJECT_NAME}  PRIVATE "${MAPGIS_COMMON}/lib(x64)/mapgiscommon.lib")
elseif(UNIX AND NOT APPLE)
    target_include_directories(${PROJECT_NAME}  PRIVATE ${LIBICONV_ROOT}/include_linux)

     # 检查操作系统是否为Linux
     if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
         # 判断系统架构
         if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")

             target_link_libraries(${PROJECT_NAME}  PRIVATE "${DRACO_ROOT}/lib_linux_aarch64/libdraco.a")
             target_link_libraries(${PROJECT_NAME}  PRIVATE "${GLTFSDK_ROOT}/lib_linux_aarch64/libGLTFSDK.a")
			 target_link_libraries(${PROJECT_NAME}  PRIVATE "${MAPGIS_COMMON}/lib_linux_aarch64/libmapgiscommon.so")
			 target_link_libraries(${PROJECT_NAME}  PRIVATE "${SQLITE3_ROOT}/lib_linux_aarch64/libsqlite.so")
			  # 关于boot的现在没有正确链接
			  target_link_libraries(${PROJECT_NAME}  PRIVATE "${BOOST_ROOT}/lib_linux_aarch64/libboost_filesystem.so")
         elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")

             target_link_libraries(${PROJECT_NAME}  PRIVATE "${DRACO_ROOT}/lib_linux_x86_64/libdraco.a")
             target_link_libraries(${PROJECT_NAME}  PRIVATE "${GLTFSDK_ROOT}/lib_linux_x86_64/libGLTFSDK.a")
			 target_link_libraries(${PROJECT_NAME}  PRIVATE "${MAPGIS_COMMON}/lib_linux_x86_64/libmapgiscommon.so")
			 target_link_libraries(${PROJECT_NAME}  PRIVATE "${SQLITE3_ROOT}/lib_linux_x86_64/libsqlite.so")
			 
			 # 关于boot的现在没有正确链接
			  target_link_libraries(${PROJECT_NAME}  PRIVATE "${BOOST_ROOT}/lib_linux_x86_64/libboost_filesystem.so")
         endif()
     endif()

endif ()






