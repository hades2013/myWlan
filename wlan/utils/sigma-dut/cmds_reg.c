/*
 * Sigma Control API DUT (station/AP)
 * Copyright (c) 2010, Atheros Communications, Inc.
 * Copyright (c) 2011-2014, Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Licensed under the Clear BSD license. See README for more details.
 */

#include "sigma_dut.h"

void sigma_dut_register_cmds(void)
{
	void basic_register_cmds(void);
	basic_register_cmds();

	void sta_register_cmds(void);
	sta_register_cmds();

	void traffic_register_cmds(void);
	traffic_register_cmds();

#ifdef CONFIG_TRAFFIC_AGENT
	void traffic_agent_register_cmds(void);
	traffic_agent_register_cmds();
#endif /* CONFIG_TRAFFIC_AGENT */

	void p2p_register_cmds(void);
	p2p_register_cmds();

	void ap_register_cmds(void);
	ap_register_cmds();

	void powerswitch_register_cmds(void);
	powerswitch_register_cmds();

	void atheros_register_cmds(void);
	atheros_register_cmds();

#ifdef CONFIG_WLANTEST
	void wlantest_register_cmds(void);
	wlantest_register_cmds();
#endif /* CONFIG_WLANTEST */

	void dev_register_cmds(void);
	dev_register_cmds();

#ifdef CONFIG_SNIFFER
	void sniffer_register_cmds(void);
	sniffer_register_cmds();
#endif /* CONFIG_SNIFFER */

#ifdef CONFIG_SERVER
	void server_register_cmds(void);
	server_register_cmds();
#endif /* CONFIG_SERVER */
}
