add_executable(CopyStateMachine main.cpp
        ../csm-framework/consensus/raft/Common.h
        ../csm-framework/protocol/Protocol.h)
target_link_libraries(CopyStateMachine PUBLIC initializer)