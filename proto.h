/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   proto.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/02 18:18:52 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/02 18:33:39 by fcadet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROTO_H
#define PROTO_H

void			error(t_err ret, char *fnc, char *msg, char *quote);
size_t			str_len(char *str);
t_bool			str_2_uint(char *str, unsigned int *result);
uint16_t		checksum(void *body, int size);
int				ft_printf(char *form, ...);
struct timeval	until_now(struct timeval start);

t_bool			flag_set(t_flag flg, t_glob *glob);
t_bool			opt_set(t_flag flg, t_glob *glob, unsigned int *val);
t_bool			parse_arg(char **arg, t_glob *glob);

#endif //PROTO_H
