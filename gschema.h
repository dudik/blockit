#ifndef __RESOURCE_gschema_H__
#define __RESOURCE_gschema_H__

#include <gio/gio.h>

extern GResource *gschema_get_resource (void);

extern void gschema_register_resource (void);
extern void gschema_unregister_resource (void);

#endif
