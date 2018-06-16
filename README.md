ethtool-snmpd: implementation of a MIB for ethtool
==================================================

`ethtool-snmpd` is a subagent for NetSNMP (or any other agent
supporting the AgentX protocol). It currently allows the export of
ethtool statistics (both NIC and PHY).

Here is an example of walk of this MIB:

    ETHTOOL-MIB::ethtoolStat.2.'align_errors' = Counter64: 0
    ETHTOOL-MIB::ethtoolStat.2.'broadcast' = Counter64: 760
    ETHTOOL-MIB::ethtoolStat.2.'multicast' = Counter64: 1322
    ETHTOOL-MIB::ethtoolStat.2.'rx_errors' = Counter64: 0
    ETHTOOL-MIB::ethtoolStat.2.'rx_missed' = Counter64: 0
    ETHTOOL-MIB::ethtoolStat.2.'rx_packets' = Counter64: 2358495
    ETHTOOL-MIB::ethtoolStat.2.'tx_aborted' = Counter64: 0
    ETHTOOL-MIB::ethtoolStat.2.'tx_errors' = Counter64: 0
    ETHTOOL-MIB::ethtoolStat.2.'tx_multi_collisions' = Counter64: 0
    ETHTOOL-MIB::ethtoolStat.2.'tx_packets' = Counter64: 3349654
    ETHTOOL-MIB::ethtoolStat.2.'tx_single_collisions' = Counter64: 0
    ETHTOOL-MIB::ethtoolStat.2.'tx_underrun' = Counter64: 0
    ETHTOOL-MIB::ethtoolStat.2.'unicast' = Counter64: 2356413
    ETHTOOL-MIB::ethtoolStat.36.'peer_ifindex' = Counter64: 37
    ETHTOOL-MIB::ethtoolStat.37.'peer_ifindex' = Counter64: 36

This work has been sponsored by [The IMS Company][1].

To compile this agent:

    ./configure
    make
    sudo make install

[1]: http://imsco-us.com/
