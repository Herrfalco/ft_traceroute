/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   header.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/01 10:06:59 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/01 12:40:45 by fcadet           ###   ########.fr       */
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

#include "icmp.h"

#define	ICMP_FILTER			1

#define BODY_SZ				56 
#define TTL					64

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
//	E_ARG_NB,
	E_PERM,
	E_TARG,
	E_SCK_CRE,
	E_SCK_OPT,
//	E_SND,
//	E_REC,
//	E_ALLOC,
	E_ARG,
//	E_DUP,
//	E_NO_MATCH,
//	E_BCK_TIME,
}							t_err;

typedef struct					s_targ {
	struct sockaddr_in			in;
	char						*addr;
	char						*name;
}								t_targ;

typedef struct					s_args {
	size_t						flags;
	size_t						opts_flags;
	unsigned int				opts[OPTS_NB];		
}								t_args;

typedef struct					s_glob {
	t_targ						targ;
	int							sock;
//	struct timeval				start;
//	t_icmp_pkt					pkt;
//	t_pngs						pngs;
//	t_acc						acc;
//	t_errors					errors;
	t_args						args;
}								t_glob;

#endif //HEADER_H
