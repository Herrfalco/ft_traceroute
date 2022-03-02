/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/01 09:57:43 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/02 13:05:23 by fcadet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//Return value ?

#include "header.h"

static void			error(t_err ret, char *fnc, char *msg, char *quote) {
	fprintf(stderr, "Error: ");	
	if (fnc)
		fprintf(stderr, "%s: ", fnc);	
	fprintf(stderr, "%s", msg);
	if (quote)
		fprintf(stderr, " \"%s\"", quote);
	fprintf(stderr, "\n");
	if (ret)
		exit(2);
}

size_t			str_len(char *str) {
	size_t		len;

	for (len = 0; str[len]; ++len);
	return (len);
}

t_bool		flag_set(t_flag flg, t_glob *glob) {
	return (!!(glob->args.flags & (0x1 << flg)));
}

t_bool		opt_set(t_flag flg, t_glob *glob, unsigned int *val) {
	if (val && glob->args.opts_flags & (0x1 << flg)) {
		*val = glob->args.opts[flg];
		return (TRUE);
	}
	return (FALSE);
}

static t_bool		add_flag(char *arg, t_glob *glob) {
	t_bool		found;
	size_t		flgs = 0;

	while (*++arg) {
		found = FALSE;
		for (size_t i = 0; i < str_len(FLGS); ++i) {
			if (*arg == FLGS[i]) {
				if (flag_set(i, glob))
					error(E_ARG, "Command line", "Duplicated argument", arg);
				flgs |= 0x1 << i;
				found = TRUE;
				break;
			}
		}
		if (!found)
			return (FALSE);
	}
	glob->args.flags |= flgs;
	return (TRUE);
}

t_bool			str_2_uint(char *str, unsigned int *result) {
	long		res = 0;

	for (; *str; ++str) {
		if (*str < '0' || *str > '9')
			return (TRUE);
		res *= 10;
		res += *str - '0';
		if (res > UINT_MAX)
			return (TRUE);
	}
	*result = res;
	return (FALSE);
}

static t_bool		add_opt(char ***arg, t_glob *glob) {
	if (str_len(**arg) != OPT_SZ)
		return (FALSE);
	for (size_t i = 0; i < str_len(OPTS); ++i) {
		if ((**arg)[1] == OPTS[i]) {
			if (opt_set(i, glob, NULL))
				error(E_ARG, "Command line", "Duplicated argument", **arg);
			if (!*(*arg + 1))
				error(E_ARG, "Command line", "Need value for option", **arg);
			if (str_2_uint(*(*arg + 1), &glob->args.opts[i]))
				error(E_ARG, "Command line", "Bad value for option", **arg);
			glob->args.opts_flags |= 0x1 << i;
			++(*arg);
			return (TRUE);
		}
	}
	return (FALSE);
}

t_bool		parse_arg(char **arg, t_glob *glob) {
	for (; *arg; ++arg) {
		if (**arg != '-') {
			if (*(arg + 1))
				error(E_ARG, "Command line", "Unrecognized argument", *arg);
			return (FALSE);
		}
		if (str_len(*arg) < OPT_SZ || (!add_flag(*arg, glob) && !add_opt(&arg, glob)))
			error(E_ARG, "Command line", "Unrecognized argument", *arg);
	}
	return (TRUE);
}

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
/*
	if (ttl > 255)
		error(E_SCK_OPT, "Socket", "TTL value is too high", NULL);
	*/
	if (setsockopt(glob->sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(uint8_t)))
		error(E_SCK_OPT, "Socket", "Can't configure TTL", NULL);
}

uint16_t	checksum(void *body, int size) {
	uint16_t	*data = body;
	uint32_t	result = 0;

	for (; size > 1; size -= 2)
		result += *(data++);		
	if (size)
		result += *((uint8_t *)data);
	while (result >> 16)
		result = (result & 0xffff) + (result >> 16);
	return (~result);
}

void		new_probe(t_glob *glob, unsigned int ttl) {
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

struct timeval	until_now(struct timeval start) {
	struct timeval		result = { 0 };	
	struct timeval		end;
	long long			tmp;

	gettimeofday(&end, NULL);
	if (start.tv_sec > end.tv_sec)
		error(E_BCK_TIME, "Clock", "Time goes backward !!!", NULL);
	result.tv_sec = end.tv_sec - start.tv_sec;
	if ((tmp = end.tv_usec - start.tv_usec) < 0) {
		if (result.tv_sec < 1)
			error(E_BCK_TIME, "Clock", "Time goes backward !!!", NULL);
		--result.tv_sec;
		result.tv_usec = 1000000 + tmp;
	} else
		result.tv_usec = tmp;
	return (result);
}

int			ft_printf(char *form, ...) {
	static char			buff[BUFF_SZ];
	va_list				args;

	va_start(args, form);
	vsprintf(buff, form, args);
	return (write(STDOUT, buff, str_len(buff)));
}

int			main(int argc, char **argv) {
	t_glob			glob = { 0 };
	struct timeval		sel_timo = {
		.tv_sec = SEL_TIMO_MS / 1000,
		.tv_usec = SEL_TIMO_MS % 1000 * 1000,
	};
	t_bool			no_addr;

	if (argc < 2)
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
	printf("traceroute to %s (%s), %d hops max\n",
		glob.targ.name ? glob.targ.name : glob.targ.addr, glob.targ.addr, MAX_HOP);
	glob.pkt.type = ICMP_ECHO;
	glob.pkt.id = htons(getpid());
	for (unsigned int i = 1; i <= MAX_HOP; ++i) {
		t_bool				first = TRUE;
		struct timeval		prob_start = { 0 };
		struct timeval		duration = { 0 };
		fd_set				set;
		t_err				err = E_NO;

		for (unsigned int j = 0; j < PROB_NB; ++j) {
			FD_ZERO(&set);
			FD_SET(glob.sock, &set);
			select(glob.sock + 1, NULL, &set, NULL, &sel_timo);
			if (!FD_ISSET(glob.sock, &set))
				error(E_SEL, "Socket", "Target unable to receive data", NULL);
			new_probe(&glob, i);
		}
		ft_printf(" %2d ", i);
		gettimeofday(&prob_start, NULL);
		for (size_t r_count = 0; r_count < PROB_NB;) {
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
				++r_count;
				if (r_count == PROB_NB) {
					if ((r_pkt.icmp_pkt.type == ICMP_ECHOREPLY && r_pkt.icmp_pkt.id == glob.pkt.id)
							|| err) {
						printf("\n");
						exit(err);
					}
				}
			} else if ((duration.tv_sec * 1000000 + duration.tv_usec) / (RESP_TIMO_MS * 1000)
				> (long int)r_count) {
				ft_printf("  *");	
				++r_count;
			}
		}
		printf("\n");
	}
}
