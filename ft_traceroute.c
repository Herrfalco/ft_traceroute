/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/01 09:57:43 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/02 19:49:32 by fcadet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

static struct addrinfo		create_hints(void) {
	struct addrinfo		hints = { 0 };

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	return (hints);
}

static void		find_targ(char *arg, t_glob *glob) {
	struct in_addr		ip = { 0 };
	struct addrinfo		hints = create_hints();
	struct addrinfo		*inf = NULL;

	if (inet_pton(AF_INET, arg, &ip) == 1) {
		glob->targ.in.sin_family = AF_INET;
		glob->targ.in.sin_addr = ip;
		glob->targ.addr = inet_ntoa(ip);
	} else if ((getaddrinfo(arg, NULL, &hints, &inf) == 0)) {
		glob->targ.in = *((struct sockaddr_in *)inf->ai_addr);
		glob->targ.addr = inet_ntoa(glob->targ.in.sin_addr);
		glob->targ.name = arg;
		free(inf);
	} else 
		error(E_TARG, "Target search", "Can't find domain or address", arg);
}

static void		create_sock(t_glob *glob) {
	uint32_t		filt = 1 << ICMP_ECHO;

	if ((glob->sock = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
		error(E_SCK_CRE, "Socket", "Can't be created", NULL);
	if (setsockopt(glob->sock, SOL_RAW, ICMP_FILTER, &filt, sizeof(uint32_t)))
		error(E_SCK_OPT, "Socket", "Can't be configured", NULL);
}

static void		conf_ttl(t_glob *glob, unsigned int ttl) {
	if (ttl > 255)
		error(E_SCK_OPT, "Socket", "TTL value is too high", NULL);
	if (setsockopt(glob->sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(uint8_t)))
		error(E_SCK_OPT, "Socket", "Can't configure TTL", NULL);
}

void			new_probe(t_glob *glob, unsigned int ttl) {
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

int			main(int argc, char **argv) {
	t_glob			glob = { 0 };
	struct timeval		sel_timo = {
		.tv_sec = SEL_TIMO_MS / 1000,
		.tv_usec = SEL_TIMO_MS % 1000 * 1000,
	};
	t_bool			no_addr;
	unsigned int	ttl = MIN_HOP;
	unsigned int	max_hop = MAX_HOP;
	unsigned int	prob_nb = PROB_NB;
	unsigned int	resp_timo = RESP_TIMO;

	if (argc < MIN_ARG)
		error(E_ARG, "Command line", "Need argument (-h for help)", NULL);
	no_addr = parse_arg(++argv, &glob);
	if (flag_set(F_H, &glob)) {
		printf("%s", HELP_TXT);
		exit(0);
	} else if (flag_set(F_UPV, &glob)) {
		printf("%s", VERS_TXT);
		exit(0);
	} else if (no_addr)
		error(E_ARG, "Command line", "No domain or address specified", NULL);
	if (getuid())
		error(E_PERM, "Permissions", "Need to be run with sudo", NULL);
	find_targ(argv[argc - 2], &glob);
	create_sock(&glob);
	opt_set(O_F, &glob, &ttl);
	opt_set(O_M, &glob, &max_hop);
	if (max_hop < MIN_HOP || ttl < MIN_HOP || ttl > MAX_TTL || max_hop > MAX_TTL)
		error(E_ARG, "Command line", "Bad TTL value", NULL);
	opt_set(O_Q, &glob, &prob_nb);
	if (prob_nb < MIN_PROB_NB || prob_nb > MAX_PROB_NB)
		error(E_ARG, "Command line", "Bad prob number", NULL);
	opt_set(O_W, &glob, &resp_timo);
	if (resp_timo > MAX_RSP_TIMO)
		error(E_ARG, "Command line", "Bad waiting time", NULL);
	printf("traceroute to %s (%s), %d hops max\n",
		glob.targ.name ? glob.targ.name : glob.targ.addr, glob.targ.addr, max_hop);
	glob.pkt.type = ICMP_ECHO;
	glob.pkt.id = htons(getpid());
	for (unsigned int i = 0; i < max_hop; ++i) {
		t_bool				first = TRUE;
		struct timeval		prob_start = { 0 };
		struct timeval		duration = { 0 };
		fd_set				set;
		t_err				err = E_NO;
		size_t				miss_count = 0;
		t_bool				targ_found = FALSE;

		for (unsigned int j = 0; j < prob_nb; ++j) {
			FD_ZERO(&set);
			FD_SET(glob.sock, &set);
			select(glob.sock + 1, NULL, &set, NULL, &sel_timo);
			if (!FD_ISSET(glob.sock, &set))
				error(E_SEL, "Socket", "Target unable to receive data", NULL);
			new_probe(&glob, i + ttl);
		}
		ft_printf(" %2d ", i + 1);
		gettimeofday(&prob_start, NULL);
		for (size_t r_count = 0; r_count + miss_count < prob_nb;) {
			FD_ZERO(&set);
			FD_SET(glob.sock, &set);
			duration = until_now(prob_start);
			select(glob.sock + 1, &set, NULL, NULL, &sel_timo);
			if (FD_ISSET(glob.sock, &set)) {
				t_ip_pkt			r_pkt = { 0 };
				struct sockaddr		r_addr = { 0 };
				socklen_t			r_addr_sz = sizeof(struct sockaddr);
				t_icmp_pkt			*s_pkt = (t_icmp_pkt *)(r_pkt.icmp_pkt.body + IP_HDR_SZ);

				if (recvfrom(glob.sock, &r_pkt, sizeof(t_ip_pkt), 0, &r_addr, &r_addr_sz) < 0)
					error(E_REC, "Ping", "Can't receive packet", NULL);
				duration = until_now(prob_start);
				if (r_pkt.icmp_pkt.type != ICMP_ECHOREPLY
						&& (s_pkt->id != glob.pkt.id || s_pkt->seq != glob.pkt.seq))
					continue;
				if (first) {
					ft_printf("  %s", inet_ntoa(*(struct in_addr *)&r_pkt.ip_src));
					first = FALSE;
				}
				ft_printf("  %.3fms", duration.tv_sec * 1000. + duration.tv_usec / 1000.,
					r_pkt.icmp_pkt.type);
				if (r_pkt.icmp_pkt.type == ICMP_DEST_UNREACH) {
					ft_printf(" !%c", UNR_ERR[r_pkt.icmp_pkt.code]);
					err = E_RESP;
				}
				if (r_pkt.icmp_pkt.type == ICMP_ECHOREPLY && r_pkt.icmp_pkt.id == glob.pkt.id)
					targ_found = TRUE;
				++r_count;
			} else if (!resp_timo
					|| (duration.tv_sec * 1000000 + duration.tv_usec) / (resp_timo * 1000000)
					> (long int)miss_count) {
				ft_printf("  *");	
				++miss_count;
			}
		}
		printf("\n");
		if (targ_found || err)
			exit(err);
	}
}
