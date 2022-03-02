/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   const.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/02 18:29:23 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/02 19:45:38 by fcadet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONST_H
#define CONST_H

#define	ICMP_FILTER			1

#define STDOUT				1

#define IP_HDR_SZ			20
#define BODY_SZ				56 
#define BUFF_SZ				64
#define UNR_ERR				"NHPPFS**U**TTXXX"

#define MAX_HOP				64
#define MIN_HOP				1
#define MAX_TTL				255
#define PROB_NB				3
#define MIN_PROB_NB			1
#define MAX_PROB_NB			10
#define SEL_TIMO_MS			500
#define RESP_TIMO			3
#define MAX_RSP_TIMO		60
#define MIN_ARG				2

#define HELP_TXT			"Usage: traceroute [OPTION...] HOST\n" \
							"Print the route packets trace to network host.\n\n" \
							"  -h           give this help list\n" \
							"  -f <NUM>     set initial hop distance, i.e., time-to-live\n" \
							"  -m <NUM>     set maximal hop count (default: 64)\n" \
							"  -q <NUM>     send NUM probe packets per hop (default: 3)\n" \
							"  -w <NUM>     wait NUM seconds for response (default: 3)\n" \
							"  -V           print program version\n\n" \
							"Report bugs to <fcadet@student.42.fr>.\n"

#define VERS_TXT			"traceroute 1.0.0\n" \
							"This is free software: you are free to change and redistribute it.\n" \
							"There is NO WARRANTY, to the extent permitted by law.\n\n" \
							"Written by Florian Cadet.\n"

#define FLGS				"hV"
#define OPTS				"fmqw"
#define OPTS_NB				4
#define OPT_SZ				2

typedef enum				e_flag {
	F_H,
	F_UPV,
}							t_flag;

typedef enum				e_opt {
	O_F,
	O_M,
	O_Q,
	O_W,
}							t_opt;

typedef enum				e_err {
	E_NO,
	E_SEL,
	E_PERM,
	E_TARG,
	E_SCK_CRE,
	E_SCK_OPT,
	E_SND,
	E_REC,
	E_ARG,
	E_BCK_TIME,
	E_RESP,
}							t_err;

typedef enum				e_bool {
	FALSE,
	TRUE,
}							t_bool;

#endif //CONST_H
