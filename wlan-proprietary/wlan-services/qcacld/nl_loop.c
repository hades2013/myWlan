/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "debug.h"
#include "nl_loop.h"

#define NL_LOOP_MAX_IND     5

struct nl_loop_ind_table {
    unsigned short ind;
    nl_loop_ind_handler ind_handler;
    void *user_data;
};

struct nl_loop {
    int init_done;
    struct nl_loop_ind_table ind_table[NL_LOOP_MAX_IND];
    int nl_fd;
    int terminate;
};

static struct nl_loop nl_loop;

/* Compiler will remove if function is not used */
static inline char *nl_loop_ind_to_str(int ind)
{
    switch(ind) {
    case WLAN_SVC_FW_CRASHED_IND: return "WLAN_SVC_FW_CRASHED_IND";
    case WLAN_SVC_LTE_COEX_IND: return "WLAN_SVC_LTE_COEX_IND";
    case WLAN_SVC_WLAN_AUTO_SHUTDOWN_IND:
                                return "WLAN_SVC_WLAN_AUTO_SHUTDOWN_IND";
    case WLAN_SVC_DFS_CAC_START_IND: return "WLAN_SVC_DFS_CAC_START_IND";
    case WLAN_SVC_DFS_CAC_END_IND: return "WLAN_SVC_DFS_CAC_END_IND";
    case WLAN_SVC_DFS_RADAR_DETECT_IND: return "WLAN_SVC_DFS_RADAR_DETECT_IND";
    }
    return "UNKNOWN";
}

static inline char *nl_loop_type_to_str(int ind)
{
    switch(ind) {
    case ANI_NL_MSG_PUMAC: return "ANI_NL_MSG_PUMAC";
    case ANI_NL_MSG_PTT: return "ANI_NL_MSG_PTT";
    case WLAN_NL_MSG_BTC: return "WLAN_NL_MSG_BTC";
    case WLAN_NL_MSG_OEM: return "WLAN_NL_MSG_OEM";
    case WLAN_NL_MSG_SVC: return "WLAN_NL_MSG_SVC";
    }
    return "UNKNOWN";
}

static struct nl_loop_ind_table *nl_loop_find_ind_table(int ind)
{
    int i;
    struct nl_loop_ind_table *ind_table = NULL;

    for (i = 0; i < NL_LOOP_MAX_IND; i++) {
        ind_table = &nl_loop.ind_table[i];
        if (ind_table->ind_handler != NULL && ind_table->ind == ind) {
            break;
        }
    }

    return i < NL_LOOP_MAX_IND ? ind_table : NULL;
}


int nl_loop_init(void)
{
    struct sockaddr_nl sa;
    int fd = -1;
    int ret = 0;

    if (nl_loop.init_done) {
        wsvc_printf_err("%s: nl loop already initialized", __func__);
        return -1;
    }

    fd = socket(AF_NETLINK, SOCK_RAW, WLAN_NLINK_PROTO_FAMILY);
    if (fd < 0) {
        wsvc_perror("socket");
        ret = -1;
        goto out;
    }

    memset (&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = WLAN_NLINK_MCAST_GRP_ID;
    sa.nl_pid = getpid();

    ret = bind(fd, (struct sockaddr*)&sa, sizeof(sa));

    if (ret < 0) {
        wsvc_perror("bind");
        close(fd);
        goto out;
    }

    nl_loop.nl_fd = fd;
    nl_loop.init_done = 1;

out:
    if (fd >= 0 && ret < 0)
        close(fd);
    return ret;
}

int nl_loop_deinit(void)
{
    if (!nl_loop.init_done) {
        wsvc_printf_err("%s: nl loop not initialized", __func__);
        return -1;
    }

    close(nl_loop.nl_fd);

    memset(&nl_loop, 0, sizeof(nl_loop));

    return 0;
}

int nl_loop_register(int ind, nl_loop_ind_handler ind_handler, void *user_data)
{
    int i;
    struct nl_loop_ind_table *ind_table = NULL;

    if (!nl_loop.init_done) {
        wsvc_printf_err("%s: nl loop not initialized", __func__);
        return -1;
    }

    if (ind_handler == NULL) {
        wsvc_printf_err("%s: ind_handler is NULL!", __func__);
        return -1;
    }

    for (i = 0; i < NL_LOOP_MAX_IND; i++) {
        if (nl_loop.ind_table[i].ind_handler == NULL) {
            ind_table = &nl_loop.ind_table[i];
            break;
        }
    }

    if (ind_table == NULL) {
        wsvc_printf_err("%s: Ind table is full: %d, %x", __func__, i, ind);
        return -1;
    }

    wsvc_printf_dbg("%s: Registering ind: %x, %p", __func__, ind,
            ind_handler);

    ind_table->ind = ind;
    ind_table->ind_handler = ind_handler;
    ind_table->user_data = user_data;

    return 0;
}

int nl_loop_unregister(int ind)
{
    struct nl_loop_ind_table *ind_table = NULL;

    if (!nl_loop.init_done) {
        wsvc_printf_err("%s: nl loop not initialized", __func__);
        return -1;
    }

    ind_table = nl_loop_find_ind_table(ind);

    if (ind_table == NULL) {
        wsvc_printf_err("%s: Ind table entry not found: %x", __func__, ind);
        return -1;
    }

    wsvc_printf_dbg("%s: Unregistering ind: %x", __func__, ind);

    memset(ind_table, 0, sizeof(*ind_table));

    return 0;
}

static void nl_loop_handle_alarm(int sig)
{
    wsvc_printf_err("terminate: %d"
            "Could not terminate loop in 2 seconds! Some bug???",
            nl_loop.terminate);

    exit(EXIT_FAILURE);
}

int nl_loop_terminate(void)
{
    nl_loop.terminate = 1;

    wsvc_printf_info("%s called", __func__);

    signal(SIGALRM, nl_loop_handle_alarm);
    alarm(2);

    return 0;
}

static int nl_loop_process_msg_svc(struct nlmsghdr *nlh)
{
    struct sAniHdr *ani_hdr;
    struct nl_loop_ind_table *ind_table = NULL;

    ani_hdr = (struct sAniHdr *) NLMSG_DATA(nlh);

    ind_table = nl_loop_find_ind_table(ani_hdr->type);

    wsvc_printf_dbg("Ind type: %s, %x, ind_table: %p",
            nl_loop_ind_to_str(ani_hdr->type), ani_hdr->type, ind_table);

    if (ind_table)
        ind_table->ind_handler(ani_hdr->type, ind_table->user_data);

    return 0;
}

static int nl_loop_process_msg(struct nlmsghdr *nlh, int len)
{

    while(NLMSG_OK(nlh, len)) {

        wsvc_printf_dbg("nlmsghdr: len: %u, type: %s, %x, pid: %u",
                nlh->nlmsg_len,
                nl_loop_type_to_str(nlh->nlmsg_type),
                nlh->nlmsg_type,
                nlh->nlmsg_pid);

        switch(nlh->nlmsg_type) {
        case WLAN_NL_MSG_SVC:
            nl_loop_process_msg_svc(nlh);
            break;
        }

        nlh = NLMSG_NEXT(nlh, len);
    }

    return 0;
}

int nl_loop_run(void)
{
    struct sockaddr_nl dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;
    int ret = 0;
    int fd;

    if (!nl_loop.init_done) {
        wsvc_printf_err("%s: nl loop not initialized", __func__);
        ret = -1;
        goto end;
    }

    fd = nl_loop.nl_fd;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(WLAN_NL_MAX_PAYLOAD));
    if (nlh == NULL) {
        wsvc_printf_err("Cannot allocate memory!");
        ret = -1;
        goto end;
    }

    memset(nlh, 0, NLMSG_SPACE(WLAN_NL_MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(WLAN_NL_MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = WLAN_NLINK_MCAST_GRP_ID;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    while(!nl_loop.terminate) {
        ret = recvmsg(fd, &msg, 0);

        if (ret < 0 && errno == EINTR) {
            wsvc_printf_info("Loop terminating: EINTR");
            ret = 0;
            break;
        }

        if (ret < 0) {
            wsvc_perror("recvmsg");
            continue;
        }

        wsvc_hexdump("recvmsg", (uint8_t *)nlh, ret);

        nl_loop_process_msg(nlh, ret);
    }
end:
    if (nlh)
        free(nlh);

    return ret;
}
