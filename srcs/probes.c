/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   probes.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/03 08:01:25 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/03 08:43:24 by fcadet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/header.h"

static void		conf_ttl(t_glob *glob, unsigned int ttl) {
	if (ttl > 255)
		error(E_SCK_OPT, "Socket", "TTL value is too high", NULL);
	if (setsockopt(glob->sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(uint8_t)))
		error(E_SCK_OPT, "Socket", "Can't configure TTL", NULL);
}

static void		new_probe(t_glob *glob, unsigned int ttl) {
	static unsigned int		old_ttl = 0;

	if (ttl != old_ttl) {
		conf_ttl(glob, ttl);
		old_ttl = ttl;
		glob->pkt.seq = htons(ttl);
		glob->pkt.sum = 0;
		glob->pkt.sum = checksum(&glob->pkt, sizeof(t_icmp_pkt));
	}
	if (sendto(glob->sock, &glob->pkt, sizeof(t_icmp_pkt), 0,
				(struct sockaddr *)&glob->targ.in, sizeof(struct sockaddr)) < 0)
		error(E_SND, "Ping", "Can't send packet", NULL);
}

void			send_probes(t_glob *glob, t_opts *opts, struct timeval *sel_timo, unsigned int hop) {
	fd_set		set;

	for (unsigned int j = 0; j < opts->prob_nb; ++j) {
		FD_ZERO(&set);
		FD_SET(glob->sock, &set);
		select(glob->sock + 1, NULL, &set, NULL, sel_timo);
		if (!FD_ISSET(glob->sock, &set))
			error(E_SEL, "Socket", "Target unable to receive data", NULL);
		new_probe(glob, hop + opts->ttl);
	}
}

static t_bool	recv_data(t_glob *glob, struct timeval *prob_start, t_rec_ret *ret, t_bool first) {
	t_rec_data			r_dat = { 0 };
	struct timeval		duration = { 0 };

	r_dat.from_sz = sizeof(struct sockaddr);
	r_dat.base = (t_icmp_pkt *)(r_dat.pkt.icmp_pkt.body + IP_HDR_SZ);
	if (recvfrom(glob->sock, &r_dat, sizeof(t_ip_pkt), 0, &r_dat.from, &r_dat.from_sz) < 0)
		error(E_REC, "Ping", "Can't receive packet", NULL);
	duration = until_now(*prob_start);
	if ((r_dat.pkt.icmp_pkt.type != ICMP_ECHOREPLY
				&& (r_dat.base->id != glob->pkt.id
					|| r_dat.base->seq != glob->pkt.seq))
			|| (r_dat.pkt.icmp_pkt.type == ICMP_ECHOREPLY
				&& r_dat.pkt.icmp_pkt.id != glob->pkt.id))
		return (FALSE);
	if (first)
		ft_printf("  %s", inet_ntoa(*(struct in_addr *)&r_dat.pkt.ip_src));
	ft_printf("  %.3fms", duration.tv_sec * 1000. + duration.tv_usec / 1000.,
			r_dat.pkt.icmp_pkt.type);
	if (r_dat.pkt.icmp_pkt.type == ICMP_DEST_UNREACH) {
		ft_printf(" !%c", UNR_ERR[r_dat.pkt.icmp_pkt.code]);
		ret->err = E_RESP;
	}
	if (r_dat.pkt.icmp_pkt.type == ICMP_ECHOREPLY && r_dat.pkt.icmp_pkt.id == glob->pkt.id)
		ret->found = TRUE;
	return (TRUE);
}

t_rec_ret		recv_probes(t_glob *glob, t_opts *opts, struct timeval *sel_timo) {
	fd_set				set;
	size_t				miss_count = 0;
	struct timeval		prob_start = { 0 };
	struct timeval		duration = { 0 };
	t_rec_ret			ret = { 0 };
	t_bool				first = TRUE;

	gettimeofday(&prob_start, NULL);
	for (size_t r_count = 0; r_count + miss_count < opts->prob_nb;) {
		FD_ZERO(&set);
		FD_SET(glob->sock, &set);
		duration = until_now(prob_start);
		select(glob->sock + 1, &set, NULL, NULL, sel_timo);
		if (FD_ISSET(glob->sock, &set)) {
			if (recv_data(glob, &prob_start, &ret, first)) {
				first = FALSE;
				++r_count;
			}
		} else if (!opts->resp_timo
				|| (duration.tv_sec * 1000000 + duration.tv_usec) / (opts->resp_timo * 1000000)
				> (long int)miss_count) {
			ft_printf("  *");	
			++miss_count;
		}
	}
	return (ret);
}
