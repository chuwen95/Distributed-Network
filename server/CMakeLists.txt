add_executable(CopyStateMachine main.cpp
        ../csm-framework/consensus/raft/Common.h)
target_link_libraries(CopyStateMachine PUBLIC initializer)