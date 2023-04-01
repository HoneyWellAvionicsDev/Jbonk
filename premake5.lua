include "Dependencies.lua"

workspace "Jbonk"
    architecture "x64"
    startproject "Jbonker"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    flags
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
    include "vendor/premake"
    include "Jbonk/vendors/Box2D"
    include "Jbonk/vendors/Glad"
    include "Jbonk/vendors/glfw"
    include "Jbonk/vendors/imGui"
    include "Jbonk/vendors/yaml-cpp"
group ""


include "Jbonk"
include "DevGround"
include "Jbonker"