/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcadet <fcadet@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/03/02 18:22:00 by fcadet            #+#    #+#             */
/*   Updated: 2022/03/02 18:33:06 by fcadet           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

void			error(t_err ret, char *fnc, char *msg, char *quote) {
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

int			ft_printf(char *form, ...) {
	static char			buff[BUFF_SZ];
	va_list				args;

	va_start(args, form);
	vsprintf(buff, form, args);
	return (write(STDOUT, buff, str_len(buff)));
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
