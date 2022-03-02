/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   struct.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/02 18:26:58 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/02 18:57:55 by fcadet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STRUCT_H
#define STRUCT_H

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

#endif //STRUCT_H
