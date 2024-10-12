vkb__register_component(
    NAME physics
    SRC
        include/physics/physics.hpp
        src/physics.cpp
    LINK_LIBS
        vkb__core
)

vkb__register_tests(
    NAME test_physics
    SRC
        tests/physics.test.cpp
    LINK_LIBS
        vkb__physics
)
