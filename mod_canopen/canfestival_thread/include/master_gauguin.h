#ifndef _MASTER_GAUGUIN_H_
#define _MASTER_GAUGUIN_H_

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct gauguin_s
{
	int			sock_fd;
	int			last_errno;
} GAUGUIN_S;

void master_gauguin_deinit(void);
int master_gauguin_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _MASTER_GAUGUIN_H_ */
