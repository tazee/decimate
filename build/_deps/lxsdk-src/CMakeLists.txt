#
# Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.
#


# Set the minimum CMake version.
cmake_minimum_required(VERSION 3.20.2 FATAL_ERROR)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)


# Define the project.
project(ModoSamples VERSION 16.1.5.000025)


#
# Download dependencies.
#
find_package(glew 1.13.0 QUIET)
if(NOT GLEW_FOUND)
    include(FetchContent)
    FetchContent_Declare(glew URL https://sourceforge.net/projects/glew/files/glew/2.2.0/glew-2.2.0.zip)
    FetchContent_GetProperties(glew)
    if(NOT glew_POPULATED)
        set(BUILD_UTILS OFF)
        FetchContent_Populate(glew)
        add_subdirectory("${glew_SOURCE_DIR}/build/cmake" ${glew_BINARY_DIR})
        set(GLEW_INCLUDE_DIRS "${glew_SOURCE_DIR}/include")
        set(GLEW_LIBRARY_DIRS "${glew_BINARY_DIR}/libs")
        set(GLEW_LIBRARIES glew_s)
    endif(NOT glew_POPULATED)
endif(NOT GLEW_FOUND)


#
# Define Modo SDK library.
#
function(define_modo_sdk_library)
    add_library(lxsdk STATIC
        "common/clean.cpp"
        "common/cwrap.cpp"
        "common/initialize.cpp"
        "common/lxu_attrdesc.cpp"
        "common/lxu_attributes.cpp"
        "common/lxu_chanmod.cpp"
        "common/lxu_command.cpp"
        "common/lxu_deform.cpp"
        "common/lxu_dialog.cpp"
        "common/lxu_drawover.cpp"
        "common/lxu_drawover.cpp"
        "common/lxu_drop.cpp"
        "common/lxu_format.cpp"
        "common/lxu_io.cpp"
        "common/lxu_log.cpp"
        "common/lxu_math.cpp"
        "common/lxu_matrix.cpp"
        "common/lxu_message.cpp"
        "common/lxu_meta.cpp"
        "common/lxu_modifier.cpp"
        "common/lxu_package.cpp"
        "common/lxu_parser.cpp"
        "common/lxu_prefvalue.cpp"
        "common/lxu_quaternion.cpp"
        "common/lxu_queries.cpp"
        "common/lxu_scene.cpp"
        "common/lxu_schematic.cpp"
        "common/lxu_select.cpp"
        "common/lxu_shade.cpp"
        "common/lxu_simd.cpp"
        "common/lxu_surface.cpp"
        "common/lxu_tableau.cpp"
        "common/lxu_value.cpp"
        "common/lxu_vector.cpp"
        "common/lxu_vectorarray.cpp"
        "common/util.cpp"
    )

    set_target_properties(lxsdk
        PROPERTIES
            POSITION_INDEPENDENT_CODE ON
    )

    target_compile_features(lxsdk
        PRIVATE
            cxx_std_14
    )
    
    target_include_directories(lxsdk
        PUBLIC
            "include"
    )
endfunction(define_modo_sdk_library)


#
# Helper function to define a Modo SDK sample.
#
function(define_sample name)
    add_library(${name} SHARED ${ARGN})

    set_target_properties(${name}
        PROPERTIES
            POSITION_INDEPENDENT_CODE ON
    )

    target_compile_definitions(${name}
        PRIVATE
            GL_SILENCE_DEPRECATION
    )

    target_compile_features(${name}
        PRIVATE
            cxx_std_14
    )

    target_include_directories(${name}
        PRIVATE
            "${GLEW_INCLUDE_DIRS}"
    )

    target_link_libraries(${name}
        PUBLIC
            glew
            lxsdk
    )
endfunction(define_sample)


#
# Define the Modo SDK library.
#
define_modo_sdk_library()


#
# Channel Modifiers.
#
define_sample(samples.channel_modifier.double
    "samples/channel_modifier/double/cc_chanmod_double.cpp"
)

define_sample(samples.channel_modifier.linear_blend
    "samples/channel_modifier/linear_blend/cmLinearBlend.cpp"
)

define_sample(samples.channel_modifier.simple_kinematics
    "samples/channel_modifier/simple_kinematics/cmSimpleKinematics.cpp"
)

define_sample(samples.channel_modifier.simwave
    "samples/channel_modifier/simwave/cc_simmod_wave.cpp"
)

define_sample(samples.channel_modifier.sum
    "samples/channel_modifier/sum/cc_chanmod_sum.cpp"
)


#
# Color Models.
#
define_sample(samples.color_model.hsv
    "samples/color_model/hsv/hsvcolormodel.cpp"
)


#
# Commands.
#
define_sample(samples.command.alignschema
    "samples/command/alignschema/cc_command_alignschema.cpp"
)

define_sample(samples.command.arg
    "samples/command/arg/cc_command_arg.cpp"
)

define_sample(samples.command.filename
    "samples/command/filename/cc_command_filename.cpp"
)

define_sample(samples.command.mesh
    "samples/command/mesh/cc_command_mesh.cpp"
)

define_sample(samples.command.modallayout
    "samples/command/modallayout/cc_command_modallayout.cpp"
)

define_sample(samples.command.popup
    "samples/command/popup/cc_command_popup.cpp"
)

define_sample(samples.command.rendercache
    "samples/command/rendercache/cmdrendercache.cpp"
    "samples/command/rendercache/mrcwriter.cpp"
)

define_sample(samples.command.vertvalue
    "samples/command/vertvalue/instsrc.cpp"
    "samples/command/vertvalue/vertvalue.cpp"
)

#
# Custom Materials.
#
define_sample(samples.custom_material.cel
    "samples/custom_material/cel/celShader.cpp"
)

define_sample(samples.custom_material.cel_edges
    "samples/custom_material/cel_edges/shdCelEdges.cpp"
)

define_sample(samples.custom_material.hair
    "samples/custom_material/hair/hairShader.cpp"
)

define_sample(samples.custom_material.halftone
    "samples/custom_material/halftone/shdHalftone.cpp"
)

define_sample(samples.custom_material.skin
    "samples/custom_material/skin/skinShader.cpp"
)


#
# Drop.
#
define_sample(samples.drop.files
    "samples/drop/files/cc_drop_files.cpp"
)

define_sample(samples.drop.items
    "samples/drop/items/cc_drop_items.cpp"
)


#
# File I/O.
#
define_sample(samples.file_io.image_basicload
    "samples/file_io/image_basicload/cc_loader_basic.cpp"
)

define_sample(samples.file_io.image_basicsave
    "samples/file_io/image_basicsave/cc_saver_basic.cpp"
)

define_sample(samples.file_io.scene_geo
    "samples/file_io/scene_geo/fmtgeo.cpp"
)

define_sample(samples.file_io.scene_hpgl
    "samples/file_io/scene_hpgl/fmthpgl.cpp"
)

define_sample(samples.file_io.scene_itemsaver
    "samples/file_io/scene_itemsaver/cc_saver_item.cpp"
    "samples/file_io/scene_itemsaver/cc_saver_subset.cpp"
)

define_sample(samples.file_io.scene_loadoptions
    "samples/file_io/scene_loadoptions/cc_loader_options.cpp"
)

define_sample(samples.file_io.scene_objio
    "samples/file_io/scene_objio/objio.cpp"
    "samples/file_io/scene_objio/objpb.cpp"
    "samples/file_io/scene_objio/options.cpp"
)


#
# Image Monitors.
#
define_sample(samples.image_monitor.histogram
    "samples/image_monitor/histogram/histogrammonitor.cpp"
)


#
# Item Types.
#
define_sample(samples.item_type.falloff_box
    "samples/item_type/falloff_box/cc_falloff_box.cpp"
)

define_sample(samples.item_type.falloff_linear
    "samples/item_type/falloff_linear/linear.cpp"
)

define_sample(samples.item_type.falloff_radial
    "samples/item_type/falloff_radial/radial.cpp"
)

define_sample(samples.item_type.influence_morph
    "samples/item_type/influence_morph/morph.cpp"
    "samples/item_type/influence_morph/morphcmd.cpp"
)

define_sample(samples.item_type.particle_gasket
    "samples/item_type/particle_gasket/gasket.cpp"
)

define_sample(samples.item_type.psim_shield
    "samples/item_type/psim_shield/shieldsup.cpp"
)

define_sample(samples.item_type.surface_sphere
    "samples/item_type/surface_sphere/orbitem.cpp"
)

define_sample(samples.item_type.volume_ground
    "samples/item_type/volume_ground/gplane.cpp"
)


#
# Mesh Operations.
#
define_sample(samples.mesh_operations.create_vertex
    "samples/mesh_operations/create_vertex/initialize.cpp"
    "samples/mesh_operations/create_vertex/pmodel_createVertex.cpp"
)


#
# Modifiers.
#
define_sample(samples.modifier.time
    "samples/modifier/time/cc_modifier_timeinput.cpp"
)


#
# Notifiers.
#
define_sample(samples.notifier.time
    "samples/notifier/global/cc_notify_global.cpp"
)


#
# Packages.
#
define_sample(samples.package.drawover_pass
    "samples/package/drawover_pass/cc_drawover_pass.cpp"
)

define_sample(samples.package.drawover_wireframe
    "samples/package/drawover_wireframe/cc_drawover_wireframe.cpp"
)

define_sample(samples.package.filepath_asset
    "samples/package/filepath_asset/cc_filepath_asset.cpp"
)

define_sample(samples.package.view3d_overlay
    "samples/package/view3d_overlay/CameraInfo.cpp"
    "samples/package/view3d_overlay/initializer.cpp"
    "samples/package/view3d_overlay/olaySafeArea.cpp"
    "samples/package/view3d_overlay/StereoViz.cpp"
)


#
# Presets.
#
define_sample(samples.preset.colorSynthPath
    "samples/preset/colorSynthPath/colorSynthPath.cpp"
)


#
# Scene Rendering.
#
define_sample(samples.scene_render.trace
    "samples/scene_render/trace/lxu_rendercontext.cpp"
)


#
# Schematic Connections.
#
define_sample(samples.schematic_connection.graph
    "samples/schematic_connection/graph/cc_schema_graph.cpp"
)

define_sample(samples.schematic_connection.modifier
    "samples/schematic_connection/modifier/cc_schema_modifier.cpp"
)


#
# Selection Types.
#
define_sample(samples.selection_type.int
    "samples/selection_type/int/cc_seltype_int.cpp"
)


#
# Tools.
#
define_sample(samples.tool.arc
    "samples/tool/arc/arc.cpp"
)

define_sample(samples.tool.seashell
    "samples/tool/seashell/seashell.cpp"
)

define_sample(samples.tool.spikey
    "samples/tool/spikey/spikey.cpp"
)


#
# Values.
#
define_sample(samples.value.full
    "samples/value/full/cc_value_full.cpp"
)

#
# Value Textures.
#
define_sample(samples.value_texture.solid
    "samples/value_texture/solid/cc_valtex_solid.cpp"
)
