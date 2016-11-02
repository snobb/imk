#ifndef __COMPAT_H__
#define __COMPAT_H__

int fd_register(const struct config *cfg, const char *path);
int fd_dispatch(const struct config *cfg);
void fd_close(void);

#endif /* __COMPAT_H__ */
