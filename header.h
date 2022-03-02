/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   header.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/01 10:06:59 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/02 13:12:00 by fcadet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADER_H
#define HEADER_H

#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <limits.h>
#include <sys/time.h>
#include <stdarg.h>

#include "icmp.h"

#define	ICMP_FILTER			1

#define STDOUT				1

/*
#define HDR_SZ				8
*/
#define IP_HDR_SZ			20
#define BODY_SZ				56 
#define BUFF_SZ				64
#define UNR_ERR				"NHPPFS**U**TTXXX"

#define MAX_HOP				64
#define PROB_NB				3
#define SEL_TIMO_MS			500
#define RESP_TIMO_MS		3000

#define FLGS				"hV"
#define OPT_SZ				2
#define OPTS_NB				0
#define OPTS				""

#define HELP_TXT			"Usage: traceroute [OPTION...] HOST\n" \
							"Print the route packets trace to network host.\n\n" \
							"  -h           give this help list\n\n" \
							"Report bugs to <fcadet@student.42.fr>.\n"
#define VERS_TXT			"traceroute 1.0.0\n" \
							"This is free software: you are free to change and redistribute it.\n" \
							"There is NO WARRANTY, to the extent permitted by law.\n\n" \
							"Written by Florian Cadet.\n"

typedef enum				e_bool {
	FALSE,
	TRUE,
}							t_bool;

typedef enum				e_flag {
	F_H,
	F_UPV,
}							t_flag;

typedef enum				e_err {
	E_NO,
	E_SEL,
//	E_ARG_NB,
	E_PERM,
	E_TARG,
	E_SCK_CRE,
	E_SCK_OPT,
	E_SND,
	E_REC,
//	E_ALLOC,
	E_ARG,
//	E_DUP,
//	E_NO_MATCH,
	E_BCK_TIME,
	E_RESP,
}							t_err;

typedef struct					s_targ {
	struct sockaddr_in			in;
	char						*addr;
	char						*name;
}								t_targ;

typedef struct					s_icmp_pkt {
	uint8_t						type;
	uint8_t						code;
	uint16_t					sum;
	uint16_t					id;
	uint16_t					seq;
	uint8_t						body[BODY_SZ];
} __attribute__((packed))		t_icmp_pkt;

typedef struct					s_ip_pkt {
	uint8_t						ip_hdr[IP_HDR_SZ - 8];
	uint32_t					ip_src;
	uint32_t					ip_dst;
	t_icmp_pkt					icmp_pkt;
} __attribute__((packed))		t_ip_pkt;

typedef struct					s_args {
	size_t						flags;
	size_t						opts_flags;
	unsigned int				opts[OPTS_NB];		
}								t_args;

typedef struct					s_glob {
	t_targ						targ;
	int							sock;
	t_icmp_pkt					pkt;
	t_args						args;
}								t_glob;

#endif //HEADER_H
