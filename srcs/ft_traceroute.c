/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/01 09:57:43 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/03 08:08:22 by fcadet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/header.h"

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

int			main(int argc, char **argv) {
	t_glob				glob = { 0 };
	t_opts				opts = { 0 };
	struct timeval		sel_timo = {
		.tv_sec = SEL_TIMO_MS / 1000,
		.tv_usec = SEL_TIMO_MS % 1000 * 1000,
	};
	t_rec_ret			r_ret;

	get_args(&glob, &opts, argc, &argv);
	if (getuid())
		error(E_PERM, "Permissions", "Need to be run with sudo", NULL);
	find_targ(argv[argc - 2], &glob);
	create_sock(&glob);
	printf("traceroute to %s (%s), %d hops max\n",
		glob.targ.name ? glob.targ.name : glob.targ.addr, glob.targ.addr, opts.max_hop);
	glob.pkt.type = ICMP_ECHO;
	glob.pkt.id = htons(getpid());
	for (unsigned int i = 0; i < opts.max_hop; ++i) {
		send_probes(&glob, &opts, &sel_timo, i);
		ft_printf(" %2d ", i + 1);
		r_ret = recv_probes(&glob, &opts, &sel_timo);
		printf("\n");
		if (r_ret.found || r_ret.err)
			exit(r_ret.err);
	}
}
