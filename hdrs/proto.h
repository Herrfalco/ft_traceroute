/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   proto.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/02 18:18:52 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/03 08:41:42 by fcadet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROTO_H
#define PROTO_H

//////////////// utils.c ////////////////

void			error(t_err ret, char *fnc, char *msg, char *quote);
size_t			str_len(char *str);
t_bool			str_2_uint(char *str, unsigned int *result);
uint16_t		checksum(void *body, int size);
int				ft_printf(char *form, ...);
struct timeval	until_now(struct timeval start);

//////////////// args.c /////////////////

void			get_args(t_glob *glob, t_opts *opts, int argc, char ***argv);

//////////////// probes.c ///////////////

void			send_probes(t_glob *glob, t_opts *opts, struct timeval *sel_timo, unsigned int hop);
t_rec_ret		recv_probes(t_glob *glob, t_opts *opts, struct timeval *sel_timo);

#endif //PROTO_H
