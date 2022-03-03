/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/02 18:19:37 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/03 08:08:02 by fcadet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/header.h"

static t_bool		flag_set(t_flag flg, t_glob *glob) {
	return (!!(glob->args.flags & (0x1 << flg)));
}

static t_bool		opt_set(t_flag flg, t_glob *glob, unsigned int *val) {
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

static t_bool		parse_arg(char **arg, t_glob *glob) {
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

void				get_args(t_glob *glob, t_opts *opts, int argc, char ***argv) {
	t_bool			no_addr;

	opts->ttl = MIN_HOP;
	opts->max_hop = MAX_HOP;
	opts->prob_nb = PROB_NB;
	opts->resp_timo = RESP_TIMO;
	if (argc < MIN_ARG)
		error(E_ARG, "Command line", "Need argument (-h for help)", NULL);
	no_addr = parse_arg(++(*argv), glob);
	if (flag_set(F_H, glob)) {
		printf("%s", HELP_TXT);
		exit(0);
	} else if (flag_set(F_UPV, glob)) {
		printf("%s", VERS_TXT);
		exit(0);
	} else if (no_addr)
		error(E_ARG, "Command line", "No domain or address specified", NULL);
	opt_set(O_F, glob, &opts->ttl);
	opt_set(O_M, glob, &opts->max_hop);
	opt_set(O_Q, glob, &opts->prob_nb);
	opt_set(O_W, glob, &opts->resp_timo);
	if (opts->max_hop < MIN_HOP || opts->ttl < MIN_HOP || opts->ttl > MAX_TTL
			|| opts->max_hop > MAX_TTL)
		error(E_ARG, "Command line", "Bad TTL value", NULL);
	if (opts->prob_nb < MIN_PROB_NB || opts->prob_nb > MAX_PROB_NB)
		error(E_ARG, "Command line", "Bad prob number", NULL);
	if (opts->resp_timo > MAX_RSP_TIMO)
		error(E_ARG, "Command line", "Bad waiting time", NULL);
}
