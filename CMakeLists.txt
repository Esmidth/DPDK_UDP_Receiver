cmake_minimum_required(VERSION 3.10)
project(dpdk-recv)

# SET(CMAKE_CXX_FLAGS "-Wall -Wextra -g -O3 -std=gnu11")

#include_directories($ENV{FF_DPDK}/include)
#link_directories($ENV{FF_DPDK}/lib)
set(DPDK_LIBS "-Wl,--whole-archive -Wl,--no-whole-archive -L/usr/local/lib/x86_64-linux-gnu -Wl,--whole-archive -l:librte_common_cpt.a -l:librte_common_dpaax.a -l:librte_common_iavf.a -l:librte_common_octeontx.a -l:librte_common_octeontx2.a -l:librte_common_sfc_efx.a -l:librte_bus_dpaa.a -l:librte_bus_fslmc.a -l:librte_bus_ifpga.a -l:librte_bus_pci.a -l:librte_bus_vdev.a -l:librte_bus_vmbus.a -l:librte_common_qat.a -l:librte_mempool_bucket.a -l:librte_mempool_dpaa.a -l:librte_mempool_dpaa2.a -l:librte_mempool_octeontx.a -l:librte_mempool_octeontx2.a -l:librte_mempool_ring.a -l:librte_mempool_stack.a -l:librte_net_af_packet.a -l:librte_net_ark.a -l:librte_net_atlantic.a -l:librte_net_avp.a -l:librte_net_axgbe.a -l:librte_net_bond.a -l:librte_net_bnxt.a -l:librte_net_cxgbe.a -l:librte_net_dpaa.a -l:librte_net_dpaa2.a -l:librte_net_e1000.a -l:librte_net_ena.a -l:librte_net_enetc.a -l:librte_net_enic.a -l:librte_net_failsafe.a -l:librte_net_fm10k.a -l:librte_net_i40e.a -l:librte_net_hinic.a -l:librte_net_hns3.a -l:librte_net_iavf.a -l:librte_net_ice.a -l:librte_net_igc.a -l:librte_net_ixgbe.a -l:librte_net_kni.a -l:librte_net_liquidio.a -l:librte_net_memif.a -l:librte_net_netvsc.a -l:librte_net_nfp.a -l:librte_net_null.a -l:librte_net_octeontx.a -l:librte_net_octeontx2.a -l:librte_net_pfe.a -l:librte_net_qede.a -l:librte_net_ring.a -l:librte_net_sfc.a -l:librte_net_softnic.a -l:librte_net_tap.a -l:librte_net_thunderx.a -l:librte_net_txgbe.a -l:librte_net_vdev_netvsc.a -l:librte_net_vhost.a -l:librte_net_virtio.a -l:librte_net_vmxnet3.a -l:librte_raw_dpaa2_cmdif.a -l:librte_raw_dpaa2_qdma.a -l:librte_raw_ioat.a -l:librte_raw_ntb.a -l:librte_raw_octeontx2_dma.a -l:librte_raw_octeontx2_ep.a -l:librte_raw_skeleton.a -l:librte_crypto_bcmfs.a -l:librte_crypto_caam_jr.a -l:librte_crypto_dpaa_sec.a -l:librte_crypto_dpaa2_sec.a -l:librte_crypto_nitrox.a -l:librte_crypto_null.a -l:librte_crypto_octeontx.a -l:librte_crypto_octeontx2.a -l:librte_crypto_scheduler.a -l:librte_crypto_virtio.a -l:librte_compress_octeontx.a -l:librte_regex_octeontx2.a -l:librte_vdpa_ifc.a -l:librte_event_dlb.a -l:librte_event_dlb2.a -l:librte_event_dpaa.a -l:librte_event_dpaa2.a -l:librte_event_octeontx2.a -l:librte_event_opdl.a -l:librte_event_skeleton.a -l:librte_event_sw.a -l:librte_event_dsw.a -l:librte_event_octeontx.a -l:librte_baseband_null.a -l:librte_baseband_turbo_sw.a -l:librte_baseband_fpga_lte_fec.a -l:librte_baseband_fpga_5gnr_fec.a -l:librte_baseband_acc100.a -l:librte_node.a -l:librte_graph.a -l:librte_bpf.a -l:librte_flow_classify.a -l:librte_pipeline.a -l:librte_table.a -l:librte_port.a -l:librte_fib.a -l:librte_ipsec.a -l:librte_vhost.a -l:librte_stack.a -l:librte_security.a -l:librte_sched.a -l:librte_reorder.a -l:librte_rib.a -l:librte_regexdev.a -l:librte_rawdev.a -l:librte_pdump.a -l:librte_power.a -l:librte_member.a -l:librte_lpm.a -l:librte_latencystats.a -l:librte_kni.a -l:librte_jobstats.a -l:librte_ip_frag.a -l:librte_gso.a -l:librte_gro.a -l:librte_eventdev.a -l:librte_efd.a -l:librte_distributor.a -l:librte_cryptodev.a -l:librte_compressdev.a -l:librte_cfgfile.a -l:librte_bitratestats.a -l:librte_bbdev.a -l:librte_acl.a -l:librte_timer.a -l:librte_hash.a -l:librte_metrics.a -l:librte_cmdline.a -l:librte_pci.a -l:librte_ethdev.a -l:librte_meter.a -l:librte_net.a -l:librte_mbuf.a -l:librte_mempool.a -l:librte_rcu.a -l:librte_ring.a -l:librte_eal.a -l:librte_telemetry.a -l:librte_kvargs.a -Wl,--no-whole-archive -Wl,--as-needed -lrte_node -lrte_graph -lrte_bpf -lrte_flow_classify -lrte_pipeline -lrte_table -lrte_port -lrte_fib -lrte_ipsec -lrte_vhost -lrte_stack -lrte_security -lrte_sched -lrte_reorder -lrte_rib -lrte_regexdev -lrte_rawdev -lrte_pdump -lrte_power -lrte_member -lrte_lpm -lrte_latencystats -lrte_kni -lrte_jobstats -lrte_ip_frag -lrte_gso -lrte_gro -lrte_eventdev -lrte_efd -lrte_distributor -lrte_cryptodev -lrte_compressdev -lrte_cfgfile -lrte_bitratestats -lrte_bbdev -lrte_acl -lrte_timer -lrte_hash -lrte_metrics -lrte_cmdline -lrte_pci -lrte_ethdev -lrte_meter -lrte_net -lrte_mbuf -lrte_mempool -lrte_rcu -lrte_ring -lrte_eal -lrte_telemetry -lrte_kvargs -pthread -lm -ldl -lnuma -lz")

find_package(cppzmq)
find_package(fmt)
find_package(PkgConfig)
pkg_search_module(xxx REQUIRED libdpdk)

MESSAGE(STATUS ${xxx_libraries})

include_directories(./tests)

include_directories($ENV{FF_PATH}/lib/)
link_directories($ENV{FF_PATH}/lib)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -include rte_config.h -march=native -I/usr/local/include -I/usr/include/libnl3 -Wmissing-field-initializers")
# set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wmissing-field-initializers")

include_directories(${xxx_include_dirs})
add_executable(server_cmake basicfwd.cpp Data_Struct.h Functions.h common.h concurrentqueue.h)
target_link_libraries(server_cmake cppzmq)
target_link_libraries(server_cmake fmt::fmt)
target_link_libraries(server_cmake ${xxx_libraries})
target_link_libraries(server_cmake ${DPDK_LIBS})
target_link_libraries(server_cmake pthread dl rt numa crypto pcap)
target_link_libraries(server_cmake spdlog::spdlog)
target_link_libraries(server_cmake /usr/lib/libpulsar.so /usr/lib/libpulsarnossl.so)


include_directories(${xxx_include_dirs})
add_executable(repeater repeater.cpp Data_Struct.h Functions.h common.h concurrentqueue.h)
target_link_libraries(repeater cppzmq)
target_link_libraries(repeater fmt::fmt)
target_link_libraries(repeater ${xxx_libraries})
target_link_libraries(repeater ${DPDK_LIBS})
target_link_libraries(repeater pthread dl rt numa crypto pcap)
target_link_libraries(repeater spdlog::spdlog)
target_link_libraries(repeater /usr/lib/libpulsar.so /usr/lib/libpulsarnossl.so)


add_executable(test_dpdk_ring test_dpdk_ring.cpp)
target_link_libraries(test_dpdk_ring cppzmq)
target_link_libraries(test_dpdk_ring fmt::fmt)
target_link_libraries(test_dpdk_ring ${DPDK_LIBS})
target_link_libraries(test_dpdk_ring pthread dl rt numa crypto pcap)
target_link_libraries(test_dpdk_ring spdlog::spdlog)


add_executable(test_dpdk_ipc_client test_dpdk_ipc_client.cpp)
target_link_libraries(test_dpdk_ipc_client cppzmq)
target_link_libraries(test_dpdk_ipc_client fmt::fmt)
target_link_libraries(test_dpdk_ipc_client ${DPDK_LIBS})
target_link_libraries(test_dpdk_ipc_client pthread dl rt numa crypto pcap)
target_link_libraries(test_dpdk_ipc_client spdlog::spdlog)

add_executable(test_dpdk_ipc_server test_dpdk_ipc_server.cpp)
target_link_libraries(test_dpdk_ipc_server cppzmq)
target_link_libraries(test_dpdk_ipc_server fmt::fmt)
target_link_libraries(test_dpdk_ipc_server ${DPDK_LIBS})
target_link_libraries(test_dpdk_ipc_server pthread dl rt numa crypto pcap)
target_link_libraries(test_dpdk_ipc_server spdlog::spdlog)

add_executable(test_thread_affinity_c thread_affinity_c.cpp)
target_link_libraries(test_thread_affinity_c pthread)

add_executable(test_thread_affinity thread_affinity.cpp)
target_link_libraries(test_thread_affinity pthread)

add_executable(test_handler test_handler.cpp)


add_executable(test_unordered_map test_unorder_map.cpp)


#add_executable(test_dpdk_timer test_dpdk_timer.cpp)
#target_link_libraries(test_dpdk_timer cppzmq)
#target_link_libraries(test_dpdk_timer fmt::fmt)
# target_link_libraries(test_dpdk_ring libdpdk)
#target_link_libraries(test_dpdk_timer ${DPDK_LIBS})
#target_link_libraries(test_dpdk_timer pthread dl rt numa fstack crypto)
#target_link_libraries(test_dpdk_timer spdlog::spdlog)

#add_executable(test_zmq_trigger test_zmq_trigger.cpp Data_Struct.h common.h)
#target_link_libraries(test_zmq_trigger cppzmq)

#add_executable(test_read_binary test_read_binary.cpp)
#target_link_libraries(test_read_binary pthread)

#add_executable(distrub distrub.cpp)
#target_link_libraries(distrub pthread)
#add_executable(mem_alloc_test mem_alloc_test.cpp Data_Struct.h Functions.h common.h threadsafe_queue.h concurrentqueue.h)
#target_link_libraries(mem_alloc_test ${DPDK_LIBS})
#target_link_libraries(mem_alloc_test pthread dl rt numa fstack crypto)
#target_link_libraries(mem_alloc_test spdlog::spdlog)
#target_link_libraries(mem_alloc_test /usr/lib/libpulsar.so /usr/lib/libpulsarnossl.so)
#
#add_executable(log_test log_test.cpp)
#target_link_libraries(log_test PRIVATE spdlog::spdlog)
#
#add_executable(queue_test ./tests/queue_test.cpp)
#target_link_libraries(queue_test -lpthread)
#
#add_executable(queue_test1 ./tests/queue_test1.cpp)
#target_link_libraries(queue_test1 -lpthread)
#
#add_executable(memcpy_perf ./tests/memcpy_test.cpp)
#target_link_libraries(memcpy_perf -lpthread)
#
#add_executable(trans_perf ./tests/thread_cache_perf.cpp)
#target_link_libraries(trans_perf spdlog::spdlog)
#target_link_libraries(trans_perf -lpthread)
#
#add_executable(test_fake_data ./tests/test_fake_data.cpp)
#target_link_libraries(test_fake_data /usr/lib/libdhsm.so)
#target_link_libraries(test_fake_data /usr/lib/libpulsar.so /usr/lib/libpulsarnossl.so)
#target_link_libraries(server_cmake $ENV{FF_DPDK}/lib)
#target_link_libraries(server_cmake $ENV{FF_PATH}/lib)

#target_link_libraries(server_cmake -ldpdk -lfstack -lrt -lm -ldl -lcrypto -pthread -lnuma)
