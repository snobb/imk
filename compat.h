#ifndef __COMPAT_H__
#define __COMPAT_H__

int fd_register(struct config *cfg, const char *path);
int fd_dispatch(const struct config *cfg);
void fd_close(struct config *cfg);

#endif /* __COMPAT_H__ */
