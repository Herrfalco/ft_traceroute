/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/01 09:57:43 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/01 16:40:17 by fcadet           ###   ########.fr       */
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
	}
	if (sendto(glob->sock, &glob->pkt, sizeof(t_icmp_pkt), 0, (struct sockaddr *)&glob->targ.in, sizeof(struct sockaddr)) < 0)
		error(E_SND, "Ping", "Can't send packet", NULL);
}

int			main(int argc, char **argv) {
	t_glob			glob = { 0 };
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
	glob.pkt.type = ICMP_ECHO;
	glob.pkt.id = htons(getpid());
	glob.pkt.sum = checksum(&glob.pkt, sizeof(t_icmp_pkt));
	for (unsigned int i = 1; i <= MAX_HOP; ++i) {
		glob.r_count = 0;
		for (unsigned int j = 0; j < PROB_NB; ++j)
			new_probe(&glob, i);
		for (unsigned int j = 0; j < PROB_NB; ++j) {
			t_icmp_pkt			r_pkt = { 0 };
			struct sockaddr		r_addr = { 0 };
			socklen_t			r_addr_sz = sizeof(struct sockaddr);
			int					ret;

			if ((ret = recvfrom(glob.sock, &r_pkt, sizeof(t_icmp_pkt), 0, &r_addr, &r_addr_sz)) < 0)
				error(E_REC, "Ping", "Can't receive packet", NULL);
			printf("rec ");
		}
		printf("\n");
	}
}
