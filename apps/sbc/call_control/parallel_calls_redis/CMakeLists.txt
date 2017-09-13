set (cc_parallel_calls_SRCS
CC_ParallelCallsRedis.cpp
)

SET(sems_sbc_call_control_name cc_parallel_calls_redis)
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/sbc.call_control.rules.txt)
