#ifndef HACK_H_
#define HACK_H_

#ifdef __cplusplus
extern "C" {
#endif
int my_pthread_once(void *once_control, void (*init_routine)(void));
#ifdef __cplusplus
}
#endif

#endif /* HACK_H_ */
