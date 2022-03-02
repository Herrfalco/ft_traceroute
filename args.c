/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/02 18:19:37 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/02 18:22:18 by fcadet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

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
