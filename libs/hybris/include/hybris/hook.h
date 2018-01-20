/*
 * Copyright (c) 2013 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _HYBRIS_HOOK_H_
#define _HYBRIS_HOOK_H_

#ifdef __cplusplus
extern "C" {
#endif

struct _hook {
    const char *name;
    void *func;
};

void hybris_hook(const char *name, void* func);
void hybris_register_hooks(struct _hook *hooks);

#define REGISTER_HOOKS(name) \
    __attribute__((constructor)) static void _register_hooks_##name() { \
        hybris_register_hooks(name); \
    }


#ifdef __cplusplus
}
#endif

#endif // _HYBRIS_HOOK_H_

// vim: noai:ts=4:sw=4:ss=4:expandtab
