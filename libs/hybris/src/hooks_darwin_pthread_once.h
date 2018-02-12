#ifndef HOOKS_DARWIN_PTHREAD_ONCE_H_
#define HOOKS_DARWIN_PTHREAD_ONCE_H_

#ifdef __cplusplus
extern "C" {
#endif
int darwin_my_pthread_once(void *once_control, void (*init_routine)(void));
#ifdef __cplusplus
}
#endif

#endif /* HOOKS_DARWIN_PTHREAD_ONCE_H_ */
