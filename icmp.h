/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   icmp.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/02/25 19:41:58 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/01 11:48:41 by fcadet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ICMP_H
#define ICMP_H

typedef enum					e_icmp_type {
	ICMP_ECHOREPLY				= 0,
	ICMP_DEST_UNREACH			= 3,
	ICMP_SOURCE_QUENCH			= 4,
	ICMP_REDIRECT				= 5,
	ICMP_ECHO					= 8,
	ICMP_TIME_EXCEEDED			= 11,
	ICMP_PARAMETERPROB			= 12,
	ICMP_TIMESTAMP				= 13,
	ICMP_TIMESTAMPREPLY			= 14,
	ICMP_INFO_REQUEST			= 15,
	ICMP_INFO_REPLY				= 16,
	ICMP_ADDRESS				= 17,
	ICMP_ADDRESSREPLY			= 18,
}								t_icmp_type;

/*
typedef enum					e_ur_code {
	ICMP_NET_UNREACH			= 0,
	ICMP_HOST_UNREACH			= 1,
	ICMP_PROT_UNREACH			= 2,
	ICMP_PORT_UNREACH			= 3,
	ICMP_FRAG_NEEDED			= 4,
	ICMP_SR_FAILED				= 5,
	ICMP_NET_UNKNOWN			= 6,
	ICMP_HOST_UNKNOWN			= 7,
	ICMP_HOST_ISOLATED			= 8,
	ICMP_NET_ANO				= 9,
	ICMP_HOST_ANO				= 10,
	ICMP_NET_UNR_TOS			= 11,
	ICMP_HOST_UNR_TOS			= 12,
	ICMP_PKT_FILTERED			= 13,
	ICMP_PREC_VIOLATION			= 14,
	ICMP_PREC_CUTOFF			= 15,
}								t_ur_code;

typedef enum					e_re_code {
	ICMP_REDIR_NET				= 0,
	ICMP_REDIR_HOST				= 1,
	ICMP_REDIR_NETTOS			= 2,
	ICMP_REDIR_HOSTTOS			= 3,
}								t_re_code;

typedef enum					e_te_code {
	ICMP_EXC_TTL				= 0,
	ICMP_EXC_FRAGTIME			= 1,
}								t_te_code;
*/

#endif //ICMP_H
